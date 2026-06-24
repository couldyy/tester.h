#ifndef TESTER_H
#define TESTER_H

#define _GNU_SOURCE     // pipe2
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* 
    TODO:
     - macros to get real number of test that has to be built 
     - use arena for allocations
     - interactive printing: don't clear whole screen, just rows you use
     - print internal errors (such as no such file, etc.) alongside of test
     - options: silent (dont print anything at all), verbose (print stderr on test failure), intercative (print only last interation if true), colors (no colors if false)
     - macro for exit()
     - TESTER_LOG(level, fmt, ...)
     - create pipe for stdout in any case? (since it probably doesnt need to print anything to the terminal)
     
*/

#define TESTER_COLOR_GREEN "\033[32;1m"
#define TESTER_COLOR_RED "\033[31;1m"
#define TESTER_COLOR_MAGENTA "\033[35;1m"
#define TESTER_COLOR_RESET "\033[0m"
#define TESTER_TERMINAL_CLEAR_WHOLE "\033[2J"
#define TESTER_TERMINAL_CLEAR "\033[J"
#define TESTER_TERMINAL_CURSOR_TOP_LEFT "\033[H"

#define TESTER_SAVE_CURSOR "\0337"
#define TESTER_RESTORE_CURSOR "\0338"

#define TESTER_DA_DEFAULT_CAPACITY 512
#define TESTER_DA_CAPACITY_MUL_FACTOR 1.5f

#ifdef TESTER_DEBUG
#define INIT_VALUE 0xCD 
#else
#define INIT_VALUE 0
#endif // TESTER_DEBUG

#ifndef TESTER_ASSERT
#include <assert.h>
#define TESTER_ASSERT assert
#endif // TESTER_ASSERT

#define TESTER_PIPE_READ_END 0
#define TESTER_PIPE_WRITE_END 1
#define TESTER_UNUSED(x) (void)(x)

#define TESTER_JUDGE_BY_OUTPUT 1
#define TESTER_JUDGE_BY_EXIT_CODE 2

// C compiler for rebuild
#define TESTER_CC "cc"

#define TESTER_TEST_BUFFSIZE 65536
#define TESTER_READ_BATCH_SIZE 8192

#define TESTER_CMD_VEC(...) (char*[]){__VA_ARGS__, NULL}

#define TESTER_BUSYLOOP_SLEEP_INTERVAL_MS 1
#define TESTER_MICRO_TO_MILI 1000


#define TESTER_STATUSES \
    ENTRY(TEST_STATUS_CREATED = 0) \
    ENTRY(TEST_STATUS_BUILDING = 1) \
    ENTRY(TEST_STATUS_RUNNING = 2) \
    ENTRY(TEST_STATUS_ERROR_BUILD = 3) \
    ENTRY(TEST_STATUS_ERROR_OUTPUT = 4) \
    ENTRY(TEST_STATUS_ERROR_EXIT_CODE = 8) \
    ENTRY(TEST_STATUS_BUILD_OK = 16) \
    ENTRY(TEST_STATUS_FINISHED_OK = 32) \
    ENTRY(TEST_STATUS_INTERNAL_ERROR = 64) \

typedef enum {
#define ENTRY(x) TESTER_##x,
    TESTER_STATUSES
#undef ENTRY
} Tester_test_status;

typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} Tester_buff;

typedef struct Tester_proc Tester_proc;

typedef struct {
    char* name;
    char** cmd;
    char** build_cmd;
    int judge_by;
    uint8_t expected_exit_code;
    char* expected_output;

    Tester_test_status _status;
    Tester_proc* _proc;
} Tester_test;

typedef struct Tester_proc {
    int pid;
    Tester_test* test;
    int pipe_stdout_r;
    int pipe_stderr_r;

    Tester_buff buff_stdout;
    Tester_buff buff_stderr;
    int wstatus;
} Tester_proc;


typedef struct {
    bool async;                 // run tests simultaneously
    bool interactive;           // clear whole screen and print status of ALL tests (if test is not finished, it will just print ...)
    bool test_silent_stderr;    // on test error don't print stderr data from pipe
    bool silent;                // dont print anything at all ('interactive' and 'test_silent_stderr' are ignored if this option is true
} Tester_run_opt;

void tester_rebuild_yourself(int argc, char** argv, char** source_files);
int tester_do_rebulid(int argc, char** argv, char** source_files);
int tester_cmp_stat_timespec(struct stat a, struct stat b);

int tester_procs_run_opt(Tester_test* tests, size_t n_tests, Tester_test_status expected_status, Tester_run_opt opt);
#define tester_tests_build(tests, n_tests, ...) tester_procs_run_opt(tests, n_tests, TESTER_TEST_STATUS_BUILD_OK, (Tester_run_opt) {__VA_ARGS__})
#define tester_tests_run(tests, n_tests, ...) tester_procs_run_opt(tests, n_tests, TESTER_TEST_STATUS_FINISHED_OK, (Tester_run_opt) {__VA_ARGS__})

Tester_proc* tester__proc_create(char** cmd);
Tester_proc* tester_build_proc_create(Tester_test* test);
Tester_proc* tester_test_proc_create(Tester_test* test);

int tester_procs_wait_async(Tester_proc** procs, size_t n_procs, Tester_proc** rprocs_finished, size_t n_rprocs_finished);
int tester_proc_wait_sync(Tester_proc* proc);
int tester_proc_wait_async_and_read_pipes_async(Tester_proc* proc);
ssize_t tester_pipe_read_to_buff(int pipe_fd, Tester_buff* buff);

void tester_test_eval_result(Tester_test* test);
void tester_test_print_result_opt(Tester_test* test, Tester_run_opt opt);
void tester_tests_print_result_all_opt(Tester_test* tests, size_t n_tests, Tester_run_opt);

void tester_buff_init(Tester_buff* buff, size_t size);
char* tester_str_concat(const char* str1, const char* str2);

#ifdef TESTER_STRIP_PREFIXES
#define rebuild_yourself tester_rebuild_yourself
#define do_rebulid tester_do_rebulid
#define cmp_stat_timespec tester_cmp_stat_timespec

#define procs_run_opt tester_procs_run_opt
#define tests_build tester_tests_build
#define tests_run tester_tests_run

#define _proc_create tester__proc_create
#define build_proc_create tester_build_proc_create
#define test_proc_create tester_test_proc_create

#define procs_wait_async tester_procs_wait_async
#define proc_wait_sync tester_proc_wait_sync
#define proc_wait_async_and_read_pipes_async tester_proc_wait_async_and_read_pipes_async
#define pipe_read_to_buff tester_pipe_read_to_buff

#define test_eval_result tester_test_eval_result
#define test_print_result_opt tester_test_print_result_opt
#define tests_print_result_all_opt tester_tests_print_result_all_opt

#define buff_init tester_buff_init
#define str_concat tester_str_concat

typedef enum {
#define ENTRY(x) x,
    TESTER_STATUSES
#undef ENTRY
} Test_status;

//typedef Buff Tester_buff;
////typedef struct Tester_proc Tester_proc;
//typedef Test Tester_test;
//typedef Proc Tester_proc;
//typedef Run_opt Tester_run_opt;

#define Buff Tester_buff
#define Test Tester_test
#define Proc Tester_proc
#define Run_opt Tester_run_opt

#endif //TESTER_STRIP_PREFIXES


#endif // TESTER_H

#ifdef TESTER_IMPLEMENTATION

#define TESTER_CMD_PRINT(cmd) \
    do { \
        for (int i = 0; cmd[i] != NULL; i++) { \
            printf("%s ", cmd[i]); \
        } \
        printf("\n"); \
    } while (0)

// 'cmd' MUST containt at least 1 argument, which is a program name AND end with a NULL pointer
// Creates 'Tester_proc' and fills fields: pid, pipe_stdout_r, pipe_stderr_r, buff_stdout, buff_stderr
Tester_proc* tester__proc_create(char** cmd)
{
    TESTER_ASSERT(cmd != NULL);

    //TESTER_CMD_PRINT(cmd);
    Tester_proc* proc = malloc(sizeof(Tester_proc));
    TESTER_ASSERT(proc != NULL && "malloc() failed");
    memset(proc, INIT_VALUE, sizeof(Tester_proc));


    // stderr pipe is created anyway, stdout only if judging by output
    int pipe_stderr[2];
    if(pipe2(pipe_stderr, O_NONBLOCK) < 0) {
        fprintf(stderr, "pipe2(): %s\n", strerror(errno));
        exit(1);
    }
    proc->pipe_stderr_r = pipe_stderr[TESTER_PIPE_READ_END];
    tester_buff_init(&(proc->buff_stderr), TESTER_TEST_BUFFSIZE);

    // It is probably better to create an stdout pipe anyway, since we dont want uncontrolled output in the terminal
    int pipe_stdout[2];
    if(pipe2(pipe_stdout, O_NONBLOCK) < 0) {
        fprintf(stderr, "pipe2(): %s\n", strerror(errno));
        exit(1);
    }
    proc->pipe_stdout_r = pipe_stdout[TESTER_PIPE_READ_END];
    tester_buff_init(&(proc->buff_stdout), TESTER_TEST_BUFFSIZE);

    pid_t child_pid = fork();
    if (child_pid == 0) {
        // stderr pipe is created anyway, stdout only if judging by output
        close(pipe_stderr[TESTER_PIPE_READ_END]);
        if (dup2(pipe_stderr[TESTER_PIPE_WRITE_END], STDERR_FILENO) < 0) {
            fprintf(stderr, "dup2(): %s\n", strerror(errno));
            exit(1);
        }
        close(pipe_stdout[TESTER_PIPE_READ_END]);
        if (dup2(pipe_stdout[TESTER_PIPE_WRITE_END], STDOUT_FILENO) < 0) {
            fprintf(stderr, "dup2(): %s\n", strerror(errno));
            exit(1);
        }

        if (execvp(cmd[0], cmd) < 0) {
            fprintf(stderr, "execvp(): %s\n", strerror(errno));
            exit(1);
        }
    }
    else {
        proc->pid = child_pid;
        // closes only here because child inherits opened fds from parent and it need write end on pipe
        close(pipe_stdout[TESTER_PIPE_WRITE_END]);
        close(pipe_stderr[TESTER_PIPE_WRITE_END]);
        return proc;
    }
}

Tester_proc* tester_test_proc_create(Tester_test* test)
{
    TESTER_ASSERT(test != NULL);
    TESTER_ASSERT(test->cmd != NULL);

    if (test->judge_by == 0) {
        fprintf(stderr, "ERROR: (test: '%s') 'judge_by' isn't set to any value\n", test->name);
        exit(1);
    }

    if ((test->judge_by & TESTER_JUDGE_BY_OUTPUT) && (test->expected_output == NULL)) {
        fprintf(stderr, "ERROR: 'judge_by' set to 'output' but 'expected_output' wasnt provided\n");
        exit(1);
    }
    Tester_proc* proc = tester__proc_create(test->cmd);
    proc->test = test;

    // leave errors as is and clear if test was build or just created
    // NOTE: if test was run previously and has a status TESTER_TEST_STATUS_FINISHED_OK it will NOT run again
    if (test->_status == TESTER_TEST_STATUS_CREATED || test->_status == TESTER_TEST_STATUS_BUILD_OK) {
        test->_status = TESTER_TEST_STATUS_RUNNING;
    }
    test->_proc = proc;
    return proc;
}

Tester_proc* tester_build_proc_create(Tester_test* test) 
{
    TESTER_ASSERT(test != NULL);
    TESTER_ASSERT(test->build_cmd != NULL);

    Tester_proc* proc = tester__proc_create(test->build_cmd);
    proc->test = test;

    // leave errors as is and clear if test was build or just created
    // NOTE: if test was run previously and has a status TESTER_TEST_STATUS_FINISHED_OK it will NOT run again
    if (test->_status == TESTER_TEST_STATUS_CREATED ) {
        test->_status = TESTER_TEST_STATUS_BUILDING;
    }
    test->_proc = proc;
    return proc;
}

typedef struct {
    Tester_proc** items;
    size_t count;
    size_t capacity;
} Tester_procs_array;

/* Example of struct for dynamic array:
typedef struct {
    int* items;
    size_t capacity;    // number of elements (actual size is capacity * sizeof(*items)
    size_t count;       // same goes for count
} Nums;
*/


#define tester_da_init(arr_ptr, new_capacity) \
    do { \
        TESTER_ASSERT((arr_ptr) != NULL && (new_capacity) > 0); \
        (arr_ptr)->items = malloc(sizeof(*((arr_ptr)->items)) * (new_capacity)); \
        TESTER_ASSERT((arr_ptr)->items != NULL && "malloc() failed"); \
        (arr_ptr)->capacity = (new_capacity); \
    } while (0)


#define tester_da_grow(arr_ptr) \
    do { \
        TESTER_ASSERT((arr_ptr) != NULL); \
        size_t _new_capacity = (arr_ptr)->capacity * TESTER_DA_CAPACITY_MUL_FACTOR; \
        (arr_ptr)->items = realloc((arr_ptr)->items, _new_capacity * sizeof(*((arr_ptr)->items))); \
        TESTER_ASSERT((arr_ptr)->items != NULL && "realloc() failed"); \
        (arr_ptr)->capacity = _new_capacity; \
    } while (0)


#define tester_da_append(arr_ptr, elem) \
    do { \
        if ((arr_ptr)->items == NULL || (arr_ptr)->capacity == 0) { \
            tester_da_init((arr_ptr), TESTER_DA_DEFAULT_CAPACITY); \
        } \
        if ((arr_ptr)->count >= (arr_ptr)->capacity) { \
            tester_da_grow((arr_ptr)); \
        } \
        (arr_ptr)->items[(arr_ptr)->count++] = (elem); \
    } while (0) 

#define tester_da_reset(arr_ptr) \
    do { \
        TESTER_ASSERT((arr_ptr) != NULL); \
        (arr_ptr)->count = 0; \
    } while (0)

//#define tester_da_foreach(type, var, arr_ptr) for (type var = (arr_ptr)->items; var < (arr_ptr)->items + (arr_ptr)->count; var++)

#define tester_update_name_if_none(test_ptr) \
    do { \
        if ((test_ptr)->name == NULL) {  \
            (test_ptr)->name = (test_ptr)->cmd[0]; \
        } \
    } while (0) \

//#define tester_tests_build(tests, n_tests, ...) tester_procs_run_opt(tests, n_tests, TESTER_TEST_STATUS_BUILD_OK, (Tester_run_opt) {__VA_ARGS__})
int tester_procs_run_opt(Tester_test* tests, size_t n_tests, Tester_test_status expected_status, Tester_run_opt opt)
{
    TESTER_ASSERT(tests != NULL);
    TESTER_ASSERT(n_tests > 0);

    Tester_procs_array procs_arr = {0};
    // TODO: which size for init?
    tester_da_init(&procs_arr, n_tests);

    for (size_t i = 0; i < n_tests; i++) {
        Tester_test* current_test = &tests[i];

        tester_update_name_if_none(current_test);

        if (current_test->_status != TESTER_TEST_STATUS_CREATED && current_test->_status != TESTER_TEST_STATUS_BUILD_OK) { continue; }
        Tester_proc* proc;
        switch (expected_status) {
            case TESTER_TEST_STATUS_FINISHED_OK:
                proc = tester_test_proc_create(current_test);
            break;
            case TESTER_TEST_STATUS_BUILD_OK:
                if (current_test->build_cmd == NULL) { continue; }
                proc = tester_build_proc_create(current_test);
            break;
            default:
                fprintf(stderr, "ERROR: invalid 'expected_status' value, status for successfull build or finish are supported\n"); 
                exit(1);

        }
        
        tester_da_append(&procs_arr, proc);
        if (!opt.async) {
            if (opt.interactive) tester_tests_print_result_all_opt(tests, n_tests, opt);
            tester_proc_wait_sync(proc);
            tester_test_eval_result(current_test);
            if (!opt.interactive) tester_test_print_result_opt(current_test, opt);
        }
    }

    if (opt.interactive) tester_tests_print_result_all_opt(tests, n_tests, opt);
    if (opt.async) {
        Tester_proc** rprocs_finished = malloc(sizeof(Tester_proc*) * procs_arr.count);
        int finished; 

        while ((finished = tester_procs_wait_async(procs_arr.items, procs_arr.count, rprocs_finished, procs_arr.count)) > 0) {
            for (int i = 0; i < finished; i++) {
                tester_test_eval_result(rprocs_finished[i]->test);
                if (opt.interactive) tester_tests_print_result_all_opt(tests, n_tests, opt); 
                else tester_test_print_result_opt(rprocs_finished[i]->test, opt);
            }
        }
        if (finished < 0) {
            fprintf(stderr, "procs_wait_async(): insufficient arrays size for 'rprocs_finished'\n");
            exit(1);
        }
    }

    int failed_cnt = 0;
    for (int i = 0; i < procs_arr.count; i++) {
        if (procs_arr.items[i]->test->_status == TESTER_TEST_STATUS_ERROR_BUILD) {
            failed_cnt++;
        }
    }
    return failed_cnt;
}

// TODO: remove 'proc', since its pointer is already in 'test'
void tester_test_eval_result(Tester_test* test)
{
    TESTER_ASSERT(test != NULL);
    TESTER_ASSERT(test->_proc != NULL);
    // set by proc_wait_sync() if waitpid() fails
    if (test->_status == TESTER_TEST_STATUS_INTERNAL_ERROR)
        return;

    // if test was building
    if (test->_status == TESTER_TEST_STATUS_BUILDING) {
        if (WIFEXITED(test->_proc->wstatus)) {
            if (WEXITSTATUS(test->_proc->wstatus) == 0) {
                test->_status = TESTER_TEST_STATUS_BUILD_OK;
                return;
            }
        }
        test->_status = TESTER_TEST_STATUS_ERROR_BUILD;
        return;
    }

    test->_status = 0;
    if (!(test->judge_by & TESTER_JUDGE_BY_OUTPUT) && !(test->judge_by & TESTER_JUDGE_BY_EXIT_CODE)) {
        fprintf(stderr, "ERROR: (test: %s) no 'judge_by' was specified, dont know what to do\n", test->name);
        exit(1);
    }

    if (test->judge_by & TESTER_JUDGE_BY_OUTPUT) {
        TESTER_ASSERT(test->expected_output != NULL);
        //TESTER_ASSERT(proc->buff_stdout.capacity > 0);

        // needed because strncmp with 'n' == 0 would always return 0
        if (test->_proc->buff_stdout.capacity == 0 && strlen(test->expected_output) != 0) {
            test->_status |= TESTER_TEST_STATUS_ERROR_OUTPUT;
        }
        else if (strncmp(test->expected_output, test->_proc->buff_stdout.data, test->_proc->buff_stdout.capacity) != 0) {
            test->_status |= TESTER_TEST_STATUS_ERROR_OUTPUT;
            //printf("Test '%s' received output: %.*s\n", proc->buff_stdout.data, (int)proc->buff_stdout.capacity);
        }
        else {
            test->_status = TESTER_TEST_STATUS_FINISHED_OK;
        }
    }

    if (test->judge_by & TESTER_JUDGE_BY_EXIT_CODE) {
        uint8_t exit_code;
        int wstatus = test->_proc->wstatus;
        if (WIFEXITED(wstatus)) {
            exit_code = WEXITSTATUS(wstatus);
            if (exit_code != test->expected_exit_code) {
                test->_status |= TESTER_TEST_STATUS_ERROR_EXIT_CODE;
            }
            else {
                test->_status = TESTER_TEST_STATUS_FINISHED_OK;
            }
        }
        else {
            // TODO: should i handle signal codes? (e.g. .expected_by = SIGNAL_CODE?)
            test->_status |= TESTER_TEST_STATUS_ERROR_EXIT_CODE;
        }
    }

}

#define tester_test_print_result(test, ...) tester_test_print_result_opt(test, (Tester_run_opt) { __VA_ARGS__ })
void tester_test_print_result_opt(Tester_test* test, Tester_run_opt opt)
{
    TESTER_ASSERT(test != NULL);

    //TESTER_ASSERT(test->_proc != NULL);
    TESTER_UNUSED(opt.async);
    TESTER_UNUSED(opt.interactive);
    if (opt.silent) return;

    // created tests means it is not running (for example if it is build stage: tests that dont have 'build_cmd' will 
    //      have status 'TESTER_TEST_STATUS_CREATED', since it is a default value
    if (test->_status != TESTER_TEST_STATUS_CREATED) { 
        printf("Test '%s' ... ", test->name);
    }

    if (test->_status != TESTER_TEST_STATUS_RUNNING && test->_status != TESTER_TEST_STATUS_BUILDING) {
        switch (test->_status) {
            case TESTER_TEST_STATUS_FINISHED_OK: 
                printf("%s", TESTER_COLOR_GREEN"OK"TESTER_COLOR_RESET);
            break;

            case TESTER_TEST_STATUS_BUILD_OK: 
                printf("%s", TESTER_COLOR_GREEN"BUILD"TESTER_COLOR_RESET);
            break;

            case TESTER_TEST_STATUS_INTERNAL_ERROR: 
                printf("%s", TESTER_COLOR_RED"ERROR: tester internal error"TESTER_COLOR_RESET);
            break;

            case TESTER_TEST_STATUS_ERROR_BUILD:
                printf("%s", TESTER_COLOR_RED"ERROR: build failed"TESTER_COLOR_RESET);
                goto print_stderr;
            break;

            default:
                if (test->_status & TESTER_TEST_STATUS_ERROR_OUTPUT) {
                    printf("%s", TESTER_COLOR_RED"ERROR: output mismatch "TESTER_COLOR_RESET);
                }
                if (test->_status & TESTER_TEST_STATUS_ERROR_EXIT_CODE) {
                    //printf("%s (stderr:%.s)", "ERROR: exit code mismatch", c); // TODO: make some flag for diplaying stderr in that case
                    printf("%s", TESTER_COLOR_RED"ERROR: exit code mismatch "TESTER_COLOR_RESET); 
                    goto print_stderr;
                }
print_stderr:
                // FIXME: for some reson proc is NULL
                if (!opt.test_silent_stderr && 
                    test->_proc != NULL &&
                    test->_proc->buff_stderr.capacity > 0 &&
                    test->_proc->buff_stderr.data != NULL) {
                    printf(": %.*s", test->_proc->buff_stderr.capacity, test->_proc->buff_stderr.data);
                }
        }
    }
    printf("\n");
}

#define tester_tests_print_result_all(tests, n_tests, ...) tester_tests_print_result_all_opt(tests, n_tests, (Tester_run_opt) { __VA_ARGS__ })
void tester_tests_print_result_all_opt(Tester_test* tests, size_t n_tests, Tester_run_opt opt)
{
    TESTER_ASSERT(tests != NULL);
    TESTER_ASSERT(n_tests > 0);

    if (opt.silent) return;

    // TODO: if cursor is at the bottom of the screen, printing becomes a mess
    //static bool first_print = true;
    //if (!first_print) {
    //    printf(TESTER_RESTORE_CURSOR);
    //    printf(TESTER_TERMINAL_CLEAR);
    //    //fflush(stdout);
    //}
    //else {
    //    first_print = false;
    //}
    //printf(TESTER_SAVE_CURSOR);
    
    if (opt.interactive) printf("%s\n", TESTER_TERMINAL_CLEAR_WHOLE TESTER_TERMINAL_CURSOR_TOP_LEFT);

    for (size_t i = 0; i < n_tests; i++) {
        Tester_test* current_test = &(tests[i]);
        tester_test_print_result_opt(current_test, opt);
    }
}

ssize_t tester_pipe_read_to_buff(int pipe_fd, Tester_buff* buff)
{
    ssize_t read_bytes = 0;
    ssize_t total_read_bytes = 0;

    do {
        read_bytes = read(pipe_fd, buff->data + buff->capacity, buff->size - buff->capacity);
        buff->capacity += read_bytes > 0 ? read_bytes : 0;
        total_read_bytes += read_bytes > 0 ? read_bytes : 0;
    } while (read_bytes > 0);
    if (read_bytes == 0)
        return total_read_bytes;
    else
        return -1;
}
// if proc is not finished, returns 0, 
// if proc is finished, returns pid of finished proc
// on error, returns -1
int tester_proc_wait_async_and_read_pipes_async(Tester_proc* proc)
{
    int wstatus;
    // TODO: what if proc is already finished but we try to read from its pipe?

    // read stdout
    ssize_t read_bytes = 0;
    if (proc->pipe_stdout_r && proc->buff_stdout.data != NULL) {
        // TODO: handle if buff.capacity <= 0
        read_bytes = tester_pipe_read_to_buff(proc->pipe_stdout_r, &(proc->buff_stdout));
        if (read_bytes < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
            fprintf(stderr, "ERROR: read() from proc (pid: %d) stdout pipe (fd: %d): %s\n", proc->pid, proc->pipe_stdout_r, strerror(errno));
            // TODO: on error set some var to not check that fd later?
        }
    }

    // TODO: handle if buff.capacity <= 0
    if (proc->pipe_stderr_r && proc->buff_stderr.data != NULL) {
        read_bytes = tester_pipe_read_to_buff(proc->pipe_stderr_r, &(proc->buff_stderr));
        if (read_bytes < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
            fprintf(stderr, "ERROR: read() from proc (pid: %d) stderr pipe (fd: %d): %s\n", proc->pid, proc->pipe_stderr_r, strerror(errno));
            // TODO: on error set some var to not check that fd later?
        }
    }

    // check if proc have finished
    int wait_result = waitpid(proc->pid, &wstatus, WNOHANG);
    if (wait_result < 0 && errno != EAGAIN) {
        fprintf(stderr, "waitpid() failed\n");
        proc->test->_status = TESTER_TEST_STATUS_INTERNAL_ERROR;
        return -1;
    }
    else if ((wait_result < 0 && errno == EAGAIN) || wait_result == 0)
    {
        return 0;
    }
    else {
        proc->wstatus = wstatus;
        return proc->pid;
    }
}

// returns the number of finished procs, which in that case is always 1
int tester_proc_wait_sync(Tester_proc* proc)
{
    // TODO: maybe just pass 'proc', since it contains test anyway?
    //tester_test_eval_result(current_proc->test, current_proc);
    int wait_result;
    while (true) {
        wait_result = tester_proc_wait_async_and_read_pipes_async(proc);
        if (wait_result != 0) 
            break;    // either proc finished or error in wait()
        usleep(TESTER_BUSYLOOP_SLEEP_INTERVAL_MS * TESTER_MICRO_TO_MILI);
    };
    return 1;
}

// spins until at least 1 proc finishes
// checks 'procs' array with 'n_procs' elements, if some procs are finished writes their addresses in 'rprocs_finished' with maximum 'n_rprocs_finished'
// 'n_rprocs_finished' should be the same as 'n_procs' unless you really know what you are doing

// on success return number of finished processes
// if all procs are finished 0 is returned
// on error, if number of returned processes > number of max elements in 'rprocs_finished' (n_rprocs_finished), returns -1

// TODO: handle when n_procs == 0
int tester_procs_wait_async(Tester_proc** procs, size_t n_procs, Tester_proc** rprocs_finished, size_t n_rprocs_finished)
{
    TESTER_ASSERT(procs != NULL);
    TESTER_ASSERT(rprocs_finished != NULL && n_rprocs_finished > 0);
    size_t total_finished = 0;
    for (size_t i = 0; i < n_procs; i++) {
        if (procs[i]->test->_status != TESTER_TEST_STATUS_RUNNING) {
            total_finished++;
        }
    }
    if (total_finished >= n_procs)
        return 0;

    int procs_finished = 0;
    // TODO: hanle when all procs.status != TESTER_TEST_STATUS_RUNNING (will hang in that case)
    while (procs_finished <= 0) {
        for (size_t i = 0; i < n_procs; i++) {
            if (procs[i]->test->_status == TESTER_TEST_STATUS_RUNNING) {
                if (tester_proc_wait_async_and_read_pipes_async(procs[i]) > 0) {
                    if (procs_finished > n_rprocs_finished) 
                        return -1;
                    rprocs_finished[procs_finished] = procs[i];
                    procs_finished += 1;
                }
            }
        }
        usleep(TESTER_BUSYLOOP_SLEEP_INTERVAL_MS * TESTER_MICRO_TO_MILI);
    }
    return procs_finished;
}

/*
      Since Linux 2.5.48, the stat structure supports nanosecond resolution
       for the three file timestamp fields.  The nanosecond components of each
       timestamp are available via names of the form .st_atim.tv_nsec, if
       suitable test macros are defined.  Nanosecond timestamps were
       standardized in POSIX.1-2008, and, starting with glibc 2.12, glibc
       exposes the nanosecond component names if _POSIX_C_SOURCE is defined
       with the value 200809L or greater, or _XOPEN_SOURCE is defined with the
       value 700 or greater.  Up to and including glibc 2.19, the definitions
       of the nanoseconds components are also defined if _BSD_SOURCE or
       _SVID_SOURCE is defined.  If none of the aforementioned macros are
       defined, then the nanosecond values are exposed with names of the form
       .st_atimensec.

*/
//#define _POSIX_C_SOURCE 1
//#define _XOPEN_SOURCE 1
#if _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700
// If 'a' > 'b' in seconds returns 1
// If 'a' < 'b' in seconds returns -1
// If 'a' == 'b' in seconds, compares nanoseconds with subtractino and returns result of that 
//                  ( 'an' > 'bn' - positive
//                    'an' < 'bn' - negative
//                    'an' == 'bn' - 0 )
int tester_cmp_stat_timespec(struct stat a, struct stat b)
{
    // Beacues type of time_t ('tv_sec') is not defined by the standard, we cannot just subtract values, since they can be unsigned values, so
    // becase of that, they are compared as is and return fixed values.
    // However 'tv_sec' is guaranteed to be signed, so they are compared with just a subtraction and returns result of that
    // NOTE: 'st_mtime' is a filed of 'struct timespec' (which contains 'tv_sec' and 'tv_nsec'), but for compatibiliry reasons
    //       'st_mtime' is mapped to 'tv_sec' and for access to 'tv_nsec', we need to access another field of 'stat' structure
    if (a.st_mtim.tv_sec > b.st_mtim.tv_sec) return 1;
    else if (a.st_mtim.tv_sec == b.st_mtim.tv_sec) return a.st_mtim.tv_nsec - b.st_mtim.tv_nsec;
    else return -1;
}
#else
int tester_cmp_stat_timespec(struct stat a, struct stat b)
{
    return a.st_mtime - b.st_mtime;
}
#endif 

// creates new str and copies both str1 and str2 into that
// both string MUST be null-terminated
char* tester_str_concat(const char* str1, const char* str2)
{
    size_t str1_len = strlen(str1);
    size_t str2_len = strlen(str2);
    size_t new_len = str1_len + str2_len + 1;

    char* new_str = malloc(new_len);
    TESTER_ASSERT(new_str != NULL && "malloc() failed");    // could handle that case, but if malloc() fails means you will have bigger problems than tester_str_concat() failure ;)
    memcpy(new_str, str1, str1_len);
    memcpy(new_str + str1_len, str2, str2_len);
    new_str[new_len - 1] = '\0';

    return new_str;
}


void tester_buff_init(Tester_buff* buff, size_t size)
{
    TESTER_ASSERT(buff != NULL);
    buff->data = malloc(size);
    TESTER_ASSERT(buff->data != NULL && "malloc() failed");
    buff->size = size;
    buff->capacity = 0;
}

//void tester_do_rebulid(int argc, char** argv, const char*[] source_files_names)
// returns exit code of build process.
// on internal errors returns -1
int tester_do_rebulid(int argc, char** argv, char** source_files)
{
    TESTER_ASSERT(argc > 0);
    TESTER_ASSERT(argv != NULL);
    TESTER_ASSERT(source_files != NULL);

    struct {
        char** items;
        size_t capacity;
        size_t count;
    } rebuild_cmd = {0};
    tester_da_init(&rebuild_cmd, 10);

    tester_da_append(&rebuild_cmd, TESTER_CC);

    for (size_t i = 0; source_files[i] != NULL; i++) {
        tester_da_append(&rebuild_cmd, source_files[i]);
    }

    if (rebuild_cmd.items[rebuild_cmd.count] != NULL) {
        tester_da_append(&rebuild_cmd, NULL);
    }

    tester_da_append(&rebuild_cmd, "-o");
    tester_da_append(&rebuild_cmd, argv[0]);

    Tester_proc* rebuild_proc = tester__proc_create(rebuild_cmd.items);

    tester_proc_wait_sync(rebuild_proc);
    if (rebuild_proc->buff_stderr.capacity > 0) write(STDERR_FILENO, rebuild_proc->buff_stderr.data, rebuild_proc->buff_stderr.capacity);
    // TODO: is it needed to print stdout?
    //if (rebuild_proc->buff_stdout.capacity > 0) fprintf(stdout, "%.*s\n", rebuild_proc->buff_stdout.capacity, rebuild_proc->buff_stdout.data);
    if (!WIFEXITED(rebuild_proc->wstatus)) {
        return -1;
    }
    return WEXITSTATUS(rebuild_proc->wstatus);
}

//void tester_rebuild_yourself(int argc, char** argv, Rebuild_opt opt, char*[] source_files)
#define TESTER_REBUILD_YOURSELF_ARGS(argc, argv, ...) tester_rebuild_yourself(argc, argv, TESTER_CMD_VEC(__VA_ARGS__))
#define TESTER_REBUILD_YOURSELF(argc, argv) tester_rebuild_yourself(argc, argv, NULL)

//void tester_rebuild_yourself_opt(int argc, char** argv, Rebuild_opt opt)
void tester_rebuild_yourself(int argc, char** argv, char** source_files)
{
    TESTER_ASSERT(argc > 0);
    TESTER_ASSERT(argv != NULL);

    char** source_files_internal; 
    if (source_files == NULL) {
        source_files_internal = TESTER_CMD_VEC(tester_str_concat(argv[0], ".c"));
    }
    else {
        source_files_internal = source_files;
    }

    struct stat binary_stat, source_stat;
    if (stat(argv[0], &binary_stat) < 0) {
        fprintf(stderr, "ERROR: stat(): %s\n", strerror(errno));
        exit(1);
    }

    // if at least 1 file's modification timestamp differs - rebuild
    for (int i = 0; source_files_internal[i] != NULL; i++) {
        memset(&source_stat, 0, sizeof(source_stat));
        char* source_file = source_files_internal[i];
        // since flags to compiler are passed along with source files, we need to skip all arguments, that starts with '-'
        if (source_file != NULL && source_file[0] != '-'){
            if (stat(source_file, &source_stat) < 0) {
                fprintf(stderr, "ERROR: file: '%s' stat(): %s\n", source_file, strerror(errno));
                exit(1);
            }
        }

        if (tester_cmp_stat_timespec(binary_stat, source_stat) < 0) {
            char* binary_name_old_ext = tester_str_concat(argv[0], ".old");
            if (rename(argv[0], binary_name_old_ext) < 0) {
                fprintf(stderr, "ERROR: renaming %s -> %s failed: %s\n", argv[0], binary_name_old_ext, strerror(errno));
                exit(1);
            }

            //printf("Need rebuild because of '%s'\n", source_file);
            if (tester_do_rebulid(argc, argv, source_files_internal) != 0) {
                if (rename(binary_name_old_ext, argv[0]) < 0) {
                    fprintf(stderr, "ERROR: renaming %s -> %s failed: %s\n", binary_name_old_ext, argv[0], strerror(errno));
                }
                fprintf(stderr, "ERROR: Rebuild of '%s' failed\n", argv[0]);
                exit(1);
            }
            if (execvp(argv[0], argv) < 0) {
                fprintf(stderr, "ERROR: execvp() on rebuild binary: %s\n", strerror(errno)); 
                exit(1);
            }
        }
    }
}

#endif //TESTER_IMPLEMENTATION

