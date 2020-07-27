#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "job_control.h"

void job_init(struct job **head){
    *head = NULL;
}

int job_get_spec(struct job **head){
    int c = 1;
    struct job *current = *head;
    while(current != NULL){
        current = current->next;
        c++;
    }
    return c;
}

void job_update_spec(struct job **head){
    struct job *current = *head;
    int c = 1;
    while(current != NULL){
        current->job_spec = c;
        current = current->next;
        c++;
    }
}

void job_insert(struct job **head, pid_t pid, int job_state, char *job_name){
    struct job *new_job = malloc(sizeof(struct job));
    new_job->pid = pid;
    new_job->job_state = job_state;
    new_job->job_name = (char *)malloc(strlen(job_name));
    strcpy(new_job->job_name, job_name);
    new_job->next = NULL;

    if(*head == NULL){
        new_job->job_spec = 1;
        *head = new_job;
    }
    else{
        new_job->job_spec = job_get_spec(head);
        struct job *current = *head;
        while(current->next != NULL){
            current = current->next;
        }
        current->next = new_job;
    }
}

void job_delete(struct job **head, pid_t pid){
    struct job *current = *head;
    // Case 1: Delete head
    if(current->pid == pid){
        free(current->job_name);
        *head = current->next;
        free(current);
    }else{
        // Case 2: Delete in middle of list or last item
        struct job *back = *head;
        current = current->next;
        while(current->pid != pid){
            current = current->next;
            back = back->next;
        }
        back->next = current->next;
        free(current->job_name);
        free(current);
    }
    job_update_spec(head);
}

void job_update(struct job **head, int pid, int job_state){
    struct job *current = *head;
    while(current->pid != pid){
        current = current->next;
    }
    current->job_state = job_state;
}

struct job *job_get(struct job **head, int job_spec){
    struct job *current = *head;
    while(current->job_spec != job_spec && current != NULL){
        current = current->next;
    }
    if(current == NULL){
        perror("Job does not exist\n");
        return NULL;
    }
    return current;
}

struct job *job_get_by_pid(struct job **head, int pid){
    struct job *current = *head;
    while(current->pid != pid && current != NULL){
        current = current->next;
    }
    if(current == NULL){
        perror("Job does not exist\n");
        return NULL;
    }
    return current;
}

void job_free_all(struct job **head){
    if(*head == NULL){
        return;
    }
    if((*head)->next == NULL){
        free((*head)->job_name);
        free(*head);
        *head = NULL;
        return;
    }
    struct job *back = *head;
    struct job *current = (*head)->next;
    while(current){
        free(back->job_name);
        free(back);
        back = current;
        current = current->next;
    }
    free(back->job_name);
    free(back);
}

int job_print(struct job **head){
    struct job *current = *head;
    if(current == NULL){
        printf("No jobs\n");
    }
    while(current){
        char *job_state = "SUSPENDED";
        if(current->job_state){
            job_state = "RUNNING";
        }
        printf("[%d] %d %s   %s\n", 
            current->job_spec, current->pid, job_state, current->job_name);
        current = current->next;
    }
    return 0;
}
