add_executable(tensile_benchmarks benchmarks.cpp)

target_link_libraries(tensile_benchmarks PRIVATE benchmark::benchmark tensile::tensile)
tensile_enable_sanitizers(tensile_benchmarks)
