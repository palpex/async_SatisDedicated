#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

namespace {
std::uint64_t work(std::uint32_t index, std::uint32_t iterations) {
    std::uint64_t x = 0x9e3779b97f4a7c15ULL ^ index;
    for (std::uint32_t i = 0; i < iterations; ++i) {
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        x *= 0x2545f4914f6cdd1dULL;
    }
    return x;
}

struct FrameStats {
    double mean_ms{0.0};
    double min_ms{0.0};
    double max_ms{0.0};
    double p95_ms{0.0};
    double p99_ms{0.0};
    double stddev_ms{0.0};
    double effective_tps{0.0};
};

FrameStats compute_stats(std::vector<double>& times_ms) {
    if (times_ms.empty()) return {};
    std::vector<double> sorted = times_ms;
    std::sort(sorted.begin(), sorted.end());

    double sum = std::accumulate(sorted.begin(), sorted.end(), 0.0);
    double mean = sum / sorted.size();

    double sq_sum = 0.0;
    for (double t : sorted) {
        sq_sum += (t - mean) * (t - mean);
    }
    double stddev = std::sqrt(sq_sum / sorted.size());

    std::size_t p95_idx = static_cast<std::size_t>(0.95 * sorted.size());
    std::size_t p99_idx = static_cast<std::size_t>(0.99 * sorted.size());
    if (p95_idx >= sorted.size()) p95_idx = sorted.size() - 1;
    if (p99_idx >= sorted.size()) p99_idx = sorted.size() - 1;

    FrameStats stats;
    stats.mean_ms = mean;
    stats.min_ms = sorted.front();
    stats.max_ms = sorted.back();
    stats.p95_ms = sorted[p95_idx];
    stats.p99_ms = sorted[p99_idx];
    stats.stddev_ms = stddev;
    stats.effective_tps = (mean > 0.0) ? (1000.0 / mean) : 0.0;
    return stats;
}
}

int main(int argc, char** argv) {
    const std::uint32_t num_tasks = (argc > 1) ? std::stoul(argv[1]) : 64;
    const std::uint32_t iterations = (argc > 2) ? std::stoul(argv[2]) : 200000;
    const std::uint32_t num_frames = (argc > 3) ? std::stoul(argv[3]) : 100;
    const unsigned int hw_threads = std::thread::hardware_concurrency();

    std::vector<double> sync_frame_times;
    std::vector<double> async_frame_times;
    std::vector<double> gamethread_stall_times;

    sync_frame_times.reserve(num_frames);
    async_frame_times.reserve(num_frames);
    gamethread_stall_times.reserve(num_frames);

    std::uint64_t sync_checksum = 0;
    std::uint64_t async_checksum = 0;

    // 1. Benchmark Synchronous Baseline (Game Thread sequential execution)
    for (std::uint32_t f = 0; f < num_frames; ++f) {
        auto t0 = std::chrono::high_resolution_clock::now();
        std::uint64_t frame_sum = 0;
        for (std::uint32_t i = 0; i < num_tasks; ++i) {
            frame_sum ^= work(i + (f * num_tasks), iterations);
        }
        sync_checksum ^= frame_sum;
        auto t1 = std::chrono::high_resolution_clock::now();
        double elapsed_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        sync_frame_times.push_back(elapsed_ms);
    }

    // 2. Benchmark Asynchronous Worker Pool (TaskGraph pattern)
    const unsigned int worker_count = (hw_threads > 1) ? (hw_threads - 1) : 1;
    for (std::uint32_t f = 0; f < num_frames; ++f) {
        auto t0 = std::chrono::high_resolution_clock::now();
        std::atomic<std::uint32_t> completed{0};
        std::atomic<std::uint64_t> frame_sum{0};
        std::vector<std::thread> workers;
        workers.reserve(worker_count);

        std::atomic<std::uint32_t> next_task{0};
        auto worker_fn = [&]() {
            while (true) {
                std::uint32_t task_idx = next_task.fetch_add(1, std::memory_order_relaxed);
                if (task_idx >= num_tasks) break;
                std::uint64_t val = work(task_idx + (f * num_tasks), iterations);
                frame_sum.fetch_xor(val, std::memory_order_relaxed);
                completed.fetch_add(1, std::memory_order_release);
            }
        };

        // Measure Game Thread dispatch duration
        auto t_dispatch_start = std::chrono::high_resolution_clock::now();
        for (unsigned int w = 0; w < worker_count; ++w) {
            workers.emplace_back(worker_fn);
        }
        auto t_dispatch_end = std::chrono::high_resolution_clock::now();
        double stall_ms = std::chrono::duration<double, std::milli>(t_dispatch_end - t_dispatch_start).count();
        gamethread_stall_times.push_back(stall_ms);

        for (auto& w : workers) {
            if (w.joinable()) w.join();
        }
        async_checksum ^= frame_sum.load();
        auto t1 = std::chrono::high_resolution_clock::now();
        double elapsed_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        async_frame_times.push_back(elapsed_ms);
    }

    FrameStats sync_stats = compute_stats(sync_frame_times);
    FrameStats async_stats = compute_stats(async_frame_times);
    FrameStats stall_stats = compute_stats(gamethread_stall_times);

    double speedup = (async_stats.mean_ms > 0.0) ? (sync_stats.mean_ms / async_stats.mean_ms) : 1.0;
    double stall_reduction = (sync_stats.mean_ms > 0.0) 
        ? ((sync_stats.mean_ms - stall_stats.mean_ms) / sync_stats.mean_ms * 100.0) : 0.0;
    double jitter_reduction = (sync_stats.stddev_ms > 0.0)
        ? ((sync_stats.stddev_ms - async_stats.stddev_ms) / sync_stats.stddev_ms * 100.0) : 0.0;

    std::cout << "{\n";
    std::cout << "  \"hardware\": {\n";
    std::cout << "    \"logical_cores\": " << hw_threads << ",\n";
    std::cout << "    \"worker_threads_used\": " << worker_count << "\n";
    std::cout << "  },\n";
    std::cout << "  \"configuration\": {\n";
    std::cout << "    \"tasks_per_frame\": " << num_tasks << ",\n";
    std::cout << "    \"iterations_per_task\": " << iterations << ",\n";
    std::cout << "    \"total_frames_measured\": " << num_frames << ",\n";
    std::cout << "    \"checksum_match\": " << (sync_checksum == async_checksum ? "true" : "false") << "\n";
    std::cout << "  },\n";
    std::cout << "  \"synchronous_baseline\": {\n";
    std::cout << "    \"mean_frame_time_ms\": " << sync_stats.mean_ms << ",\n";
    std::cout << "    \"min_frame_time_ms\": " << sync_stats.min_ms << ",\n";
    std::cout << "    \"max_frame_time_ms\": " << sync_stats.max_ms << ",\n";
    std::cout << "    \"p95_frame_time_ms\": " << sync_stats.p95_ms << ",\n";
    std::cout << "    \"p99_frame_time_ms\": " << sync_stats.p99_ms << ",\n";
    std::cout << "    \"stddev_jitter_ms\": " << sync_stats.stddev_ms << ",\n";
    std::cout << "    \"effective_tps\": " << sync_stats.effective_tps << "\n";
    std::cout << "  },\n";
    std::cout << "  \"asynchronous_bridge\": {\n";
    std::cout << "    \"mean_frame_time_ms\": " << async_stats.mean_ms << ",\n";
    std::cout << "    \"min_frame_time_ms\": " << async_stats.min_ms << ",\n";
    std::cout << "    \"max_frame_time_ms\": " << async_stats.max_ms << ",\n";
    std::cout << "    \"p95_frame_time_ms\": " << async_stats.p95_ms << ",\n";
    std::cout << "    \"p99_frame_time_ms\": " << async_stats.p99_ms << ",\n";
    std::cout << "    \"stddev_jitter_ms\": " << async_stats.stddev_ms << ",\n";
    std::cout << "    \"effective_tps\": " << async_stats.effective_tps << "\n";
    std::cout << "  },\n";
    std::cout << "  \"gamethread_impact\": {\n";
    std::cout << "    \"gamethread_stall_mean_ms\": " << stall_stats.mean_ms << ",\n";
    std::cout << "    \"gamethread_stall_p99_ms\": " << stall_stats.p99_ms << ",\n";
    std::cout << "    \"gamethread_stall_reduction_pct\": " << stall_reduction << "\n";
    std::cout << "  },\n";
    std::cout << "  \"summary\": {\n";
    std::cout << "    \"speedup_factor\": " << speedup << ",\n";
    std::cout << "    \"jitter_reduction_pct\": " << jitter_reduction << "\n";
    std::cout << "  }\n";
    std::cout << "}\n";
    return 0;
}

