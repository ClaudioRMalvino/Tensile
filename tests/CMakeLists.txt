add_executable(tensile_tests tests.cpp)

target_link_libraries(
  tensile_tests PRIVATE
  GTest::gtest_main
  tensile::tensilei
)

include(GoogleTest)
gtest_discover_tests(tensile_tests)
