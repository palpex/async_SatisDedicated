# Performance evidence and measurement limits

## What has actually been measured

| Evidence | Establishes | Does not establish |
| --- | --- | --- |
| Historical TaskGraph batch/completion probes | Worker dispatch, pure-data checksums, Game Thread continuation | Thread safety of every gameplay function |
| Standalone C++ compute benchmark | Serial versus std::thread integer computation | Vanilla-versus-modified Satisfactory TPS |
| Bounded live observations | Averaged API TPS, CPU intervals, completion/fallback counters, log events | Causal speedup without matched baseline; long-term stability |

## Current guarded build

[The anonymous observation](../evidence/runtime/guarded-observation.json) records a 120-second window on an AMD Ryzen 7 5825U (8 cores, 16 hardware threads), after removal of the thread-identity override. The library SHA-256 is `97c8fff582efe86dc6e5dd728cae1b9b15ddbbeaa18789942072d363ff644b11`.

- 13 API samples, zero connected players throughout this particular window.
- 3,565 additional intercepted and completed asynchronous factory ticks.
- API average tick rate approximately 29.70–29.81 TPS. Each value is already a server-side average, not a raw frame duration.
- Main-thread CPU approximately 22.6–23.9% of one logical CPU; total process CPU approximately 99.6–114.0% in measured intervals.
- Zero counted assertion, fatal-signal or critical-error entries; 11 inventory warnings remain recorded.

This is an idle-world observation. A repeat player-building test is still required for this exact library. Earlier player activity was observed on the preceding library, which included an unsafe thread-identity override and is not the release candidate.

![Observed tick rate](assets/gamethread_cpu_reduction.svg)

![Observed CPU intervals](assets/multithread_cpu_distribution.svg)

## Synthetic compute scaling

Values come directly from [scaling-sweep.json](../evidence/runtime/scaling-sweep.json), recorded on an Intel Core i5-4570 with three compute workers. They represent a standalone integer workload in [benchmark_simulation.cpp](../benchmarks/benchmark_simulation.cpp), not factory actors.

| Compute tasks | Serial mean | Threaded mean | Compute speedup |
| ---: | ---: | ---: | ---: |
| 16 | 6.26 ms | 2.55 ms | 2.46× |
| 32 | 12.68 ms | 4.79 ms | 2.65× |
| 64 | 25.32 ms | 11.62 ms | 2.18× |
| 128 | 50.10 ms | 23.68 ms | 2.12× |

![Synthetic compute scaling](assets/scaling_throughput_tps.svg)

Thread-creation/dispatch time excludes subsequent worker joins. Calling it Game Thread stall reduction hides that wait. Reciprocal compute duration is compute throughput, not live server TPS.

## Corrections to earlier claims

The previous 79.1% CPU-reduction claim compared whole-process lifetime CPU (103%) with a short main-thread sample (21.5%). These quantities cannot establish that reduction. A retained matched baseline for the previous 92.3% jitter claim is also missing. Both claims are withdrawn. Old synthetic tables differed from their committed data; the table above uses that JSON. Thermal throttling, fixed CPU affinity and stutter-free gameplay were not established.

Live observations near 29.7 TPS show no material TPS increase over the earlier reported baseline near 29.7 TPS. Worker distribution and short crash-free windows are useful diagnostics, not proof that the full server is rewritten or gameplay is faster.

## Reproduce

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j4
./build/satis_benchmark 64 250000 100
python3 scripts/run_scaling_sweep.py

# On the server host: names, save paths and tokens are omitted.
sudo python3 scripts/observe_server.py --duration 120 --interval 10 --output observation.json
```

For an A/B result, use the same copied save, hardware, player count and actions with `SATIS_ASYNC_TICK=0` and `1`; warm up equally and collect multiple alternating runs. Compare per-frame timings, gameplay correctness, saves/reloads and matched CPU intervals. Do not infer frame-time jitter from variation of averaged API TPS. Preserve failures and measure after each correctness fix.

Regenerate charts using `python3 scripts/generate_performance_charts.py` with Matplotlib installed in a virtual environment. Each graphic states its measurement scope.
