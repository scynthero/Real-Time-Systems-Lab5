#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
RT_TASK threads[100000];
typedef struct task
{
    long long int start_time, span_time, prio, repeat;
} task;
void serve(void *arg)
{
    RT_TASK_INFO info;
    rt_task_inquire(rt_task_self(), &info);
    time_t now = clock();
    printf("thread %s: start time: %lf\n", info.name, now / (double)CLOCKS_PER_SEC);
    rt_timer_spin(*((long long int *)arg));
    now = clock();
    printf("thread %s: endtime: %lf\n", info.name, now / (double)CLOCKS_PER_SEC);
}

int compare(const void *a, const void *b)
{
    task *task1 = (task *)a;
    task *task2 = (task *)b;
    return (task2->start_time - task1->start_time);
}

int main(int argc, char *argv[])
{
    mlockall(MCL_CURRENT | MCL_FUTURE);
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    task tasks[100000];
    int counter = 0;
    fp = fopen("tasks1.txt", "r");
    if (fp == NULL)
    {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }
    while ((read = getline(&line, &len, fp)) != -1)
    {
        long long int start_time, span_time, prio, repeat, clones;
        start_time = atoll(strtok(line, ","));
        span_time = atoll(strtok(NULL, ","));
        prio = atoll(strtok(NULL, ","));
        repeat = atoll(strtok(NULL, ","));
        for (int i = 0; i < repeat + 1; ++i)
        {
            tasks[counter].start_time = start_time;
            tasks[counter].span_time = span_time;
            tasks[counter].prio = prio;
            tasks[counter].repeat = repeat;
            ++counter;
        }
    }
    fclose(fp);
    for (int j = 0; j < counter; ++j)
    {
        char info[10];
        sprintf(info, "%i", j);
        rt_task_create(&threads[j], info, 0, tasks[j].prio, 0);
    }

    for (int i = 0; i < counter; ++i)
    {
        rt_task_start(&threads[i], &serve, &tasks[i].span_time);
    }
    pause();
}