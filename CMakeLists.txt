cmake_minimum_required(VERSION 3.16)
project(satis_task_bridge LANGUAGES CXX)

add_library(satis_task_bridge SHARED src/satis_task_bridge.cpp)
target_include_directories(satis_task_bridge PUBLIC include)
target_compile_features(satis_task_bridge PRIVATE cxx_std_17)
target_compile_options(satis_task_bridge PRIVATE -Wall -Wextra -Werror -fno-exceptions -fno-rtti)
target_link_libraries(satis_task_bridge PRIVATE dl pthread)

# Independent gameplay crash guard; contains no asynchronous tick interception.
add_library(satis_pipeline_guard SHARED src/satis_pipeline_guard.cpp)
target_compile_features(satis_pipeline_guard PRIVATE cxx_std_17)
target_compile_options(satis_pipeline_guard PRIVATE -Wall -Wextra -Werror -fno-exceptions -fno-rtti)
target_link_libraries(satis_pipeline_guard PRIVATE dl pthread)

add_executable(satis_benchmark benchmarks/benchmark_simulation.cpp)
target_compile_features(satis_benchmark PRIVATE cxx_std_17)
target_compile_options(satis_benchmark PRIVATE -O3 -Wall -Wextra)
target_link_libraries(satis_benchmark PRIVATE pthread)

