#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H
#include <sys/types.h>
#include <unistd.h>

#define SUSPENDED 0
#define RUNNING 1

struct job{
    int job_spec; // index of job node + 1
    pid_t pid;
    int job_state; // SUSPENDED==0 or RUNNING==1
    char *job_name;
    struct job *next;
};

struct job *jobs;

// Zero out jobs linked list
void job_init(struct job **head);

// Compute nex job_spec
int job_get_spec(struct job **head);

// Recompute job spec for all jobs
void job_update_spec(struct job **head);
void job_insert(struct job **head, pid_t pid, int job_state, char *job_name);
void job_delete(struct job **head, pid_t pid);

// Get a job node by pid, and update it's job state to job_state
void job_update(struct job **head, int pid, int job_state);

// Get a job node by matching job_spec
struct job *job_get(struct job **head, int job_spec);

// Get a job node by matching pid
struct job *job_get_by_pid(struct job **head, int pid);
void job_free_all(struct job **head);
int job_print(struct job **head);
#endif /*JOB_CONTROL_H*/
