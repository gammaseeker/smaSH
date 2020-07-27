#include <stdio.h>
#include <stdlib.h>
#include "job_control.h"

void test_job_ds(){
    job_init(&jobs);
    job_insert(&jobs, 69, 1, "foo.sh");
    job_insert(&jobs, 420, 0, "bar.sh");
    job_insert(&jobs, 6969, 0, "paz.sh");
    job_print(&jobs);
    job_delete(&jobs, 420);
    job_print(&jobs);
    job_delete(&jobs, 69);
    job_print(&jobs);
}
int main(){
    test_job_ds();

    return 0;
}
