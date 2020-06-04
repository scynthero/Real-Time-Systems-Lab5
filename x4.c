#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <alchemy/sem.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

RT_TASK threads[2];
RT_SEM sem[2];

typedef struct task
{
    long long int time_exec1, time_exec2, exec;
} task;

void serve1(void *arg)
{
    RT_TASK_INFO info;
    rt_task_inquire(rt_task_self(), &info);
    time_t now = clock();
    printf("task nr %s on executor 1: start time: %lf\n", info.name, now / (double)CLOCKS_PER_SEC);
    rt_timer_spin(*((int *)arg) * (long long int)1000000000);
    now = clock();
    printf("task nr %s on executor 1: end time: %lf\n", info.name, now / (double)CLOCKS_PER_SEC);
    rt_sem_v(&sem[0]);
}
void serve2(void *arg)
{
    RT_TASK_INFO info;
    rt_task_inquire(rt_task_self(), &info);
    time_t now = clock();
    printf("task nr %s on executor 2: start time: %lf\n", info.name, now / (double)CLOCKS_PER_SEC);
    rt_timer_spin(*((int *)arg) * (long long int)1000000000);
    now = clock();
    printf("task nr %s on executor 2: end time: %lf\n", info.name, now / (double)CLOCKS_PER_SEC);
    rt_sem_v(&sem[1]);
}
void exec1(void *arg)
{
    RT_TASK task1[10];
    for (int i = 0; i < 10; i++)
    {
        
        if (((task*)arg)[i].exec == 1)
        {
            char info1[15];
            sprintf(info1, "%i", i+1);
            rt_task_create(&task1[i], info1, 0, 0, 0);
            rt_task_start(&task1[i], &serve1, &((task*)arg)[i].time_exec1);
            rt_sem_p(&sem[0], TM_INFINITE);
        }
    }
}

void exec2(void *arg)
{
    RT_TASK task2[10];
    for (int i = 0; i < 10; i++)
    {
        
        if (((task*)arg)[i].exec == 2)
        {
            char info2[15];
            sprintf(info2, "%i", i+1);
            rt_task_create(&task2[i], info2, 0, 0, 0);
            rt_task_start(&task2[i], &serve2, &((task*)arg)[i].time_exec2);
            rt_sem_p(&sem[1], TM_INFINITE);
        }
    }
}
int main(int argc, char *argv[])
{
    mlockall(MCL_CURRENT | MCL_FUTURE);
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    rt_sem_create(&sem[0], "semafor1", 0, S_PULSE);
    rt_sem_create(&sem[1], "semafor2", 0, S_PULSE);
    task tasks[10];
    int counter = 0;
    long long int t_count1 = 0;
    long long int t_count2 = 0;
    fp = fopen("tasks4.txt", "r");
    if (fp == NULL)
    {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        long long int time_exec1, time_exec2;
        time_exec1 = atoll(strtok(line, ","));
        time_exec2 = atoll(strtok(NULL, ","));
        tasks[counter].time_exec1 = time_exec1;
        tasks[counter].time_exec2 = time_exec2;
        tasks[counter].exec = 0;
        ++counter;
    }
    for (int i = 0; i < 10; i++)
    {
        if (t_count1 + tasks[i].time_exec1 < t_count2 + tasks[i].time_exec2)
        {
            t_count1 = t_count1 + tasks[i].time_exec1;
            tasks[i].exec = 1;
        }
        else
        {
            t_count2 = t_count2 + tasks[i].time_exec2;
            tasks[i].exec = 2;
        }
        printf("task nr %li -> executor 1 time: %li, executor 2 time: %li, assigned executor: %li\n", i + 1, tasks[i].time_exec1, tasks[i].time_exec2, tasks[i].exec);
    }
    fclose(fp);
    printf("\n=============================================\n");
    rt_task_create(&threads[0], NULL, 0, 0, 0);
    rt_task_start(&threads[0], &exec1, &tasks);
    rt_task_create(&threads[1], NULL, 0, 0, 0);
    rt_task_start(&threads[1], &exec2, &tasks);
    pause();
}