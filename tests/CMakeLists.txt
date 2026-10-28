add_executable(tensile_tests tests.cpp)

target_link_libraries(
  tensile_tests PRIVATE
  GTest::gtest_main
  tensile::tensile
)
tensile_enable_sanitizers(tensile_tests)

include(GoogleTest)
gtest_discover_tests(tensile_tests)
