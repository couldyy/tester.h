#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    //asdf
    if (argc < 2) {
        fprintf(stderr, "Error: Name sleep time wasnt provided\n");
        exit(1);
    }
    int sleep_s = atoi(argv[1]);
    assert(sleep_s > 0);
    printf("Sleeping %ds ... ", sleep_s);
    fflush(stdout);
    sleep(sleep_s);
    printf("done\n");
    //printf("Hello, I'm a child proc of %d, my name is %s\n", getppid(), argv[1]);
    return 0;
}


