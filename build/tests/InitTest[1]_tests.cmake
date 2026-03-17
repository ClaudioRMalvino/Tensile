add_test([=[INITtest.InitialTest]=]  /home/claudio/Projects/cpp/Tensile/build/tests/InitTest [==[--gtest_filter=INITtest.InitialTest]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[INITtest.InitialTest]=]  PROPERTIES DEF_SOURCE_LINE /home/claudio/Projects/cpp/Tensile/tests/init_test.cpp:3 WORKING_DIRECTORY /home/claudio/Projects/cpp/Tensile/build/tests SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set(  InitTest_TESTS INITtest.InitialTest)
