add_executable(InitTest init_test.cpp)

target_link_libraries(
  init_test
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(init_test)


