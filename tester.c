#define TESTER_IMPLEMENTATION
#define TESTER_STRIP_PREFIXES   // strips 'Tester' from all types, structs and functions
#include "tester.h"

Test tests[] =  {
{
    .name = "./test",
    .cmd = TESTER_CMD_VEC("./test", "7"),
    .build_cmd = CMD_VEC(TESTER_CC, "-o", "test", "test.c"), // if put later, tester will run outdated version of compiled test
    .judge_by = TESTER_JUDGE_BY_OUTPUT,
    .expected_exit_code = 0,
    .expected_output = "Sleeping 7s ... done\n",
},
{
    //.name = "test",
    .cmd = TESTER_CMD_VEC("./testd", "3"),
    .judge_by = TESTER_JUDGE_BY_OUTPUT | TESTER_JUDGE_BY_EXIT_CODE,
    .expected_exit_code = 0,
    .expected_output = "Sleeping 3s ... done\n",
},
{
    .name = "./test",
    .cmd = TESTER_CMD_VEC("./test"),
    .judge_by = TESTER_JUDGE_BY_EXIT_CODE,
    .expected_exit_code = 0,
}, 
{
    .name = "./test",
    .cmd = TESTER_CMD_VEC("./test", "Not a time value"),
    .judge_by = TESTER_JUDGE_BY_OUTPUT | TESTER_JUDGE_BY_EXIT_CODE,
    .expected_exit_code = 0,
    .expected_output = "Sleeping 3s ... done\n",
},
};

int main(int argc, char** argv) {
    TESTER_REBUILD_YOURSELF(argc, argv);
    //TESTER_REBUILD_YOURSELF_ARGS(argc, argv, "-g", "tester.c");
    size_t tests_cnt = sizeof(tests)/sizeof(Tester_test);

    int build_failed = tests_build(tests, tests_cnt, .async = false);
    printf("Failed %d builds\n", build_failed);

    int failed = tests_run(tests, tests_cnt, .async = true, .interactive = false);
    printf("Failed %d / %d tests\n", failed, tests_cnt);

    return 0;
}
