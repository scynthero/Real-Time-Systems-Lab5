#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <alchemy/task.h>
#include <alchemy/heap.h>
#include <alchemy/mutex.h>
#include <alchemy/sem.h>

RT_SEM sem;
RT_TASK task;
RT_HEAP heap;
RT_MUTEX mutex;

void serve(void *arg)
{
    RT_TASK_INFO info;
    rt_task_inquire(rt_task_self(), &info);
    time_t now = clock();
    printf("thread %s: start time: %lf\n", info.name, now / (double)CLOCKS_PER_SEC);
    rt_timer_spin(*((int *)arg) * (long long int)1000000000);
    rt_sem_v(&sem);
    now = clock();
    printf("thread %s: endtime: %lf\n", info.name, now / (double)CLOCKS_PER_SEC);
}

int main(int a, char **b)
{
    int *ptr;
    _Bool flag = 0;
    rt_sem_create(&sem, "semafor", 2, S_PRIO);
    mlockall(MCL_CURRENT | MCL_FUTURE);
    printf("czekam na stertę\n");
    rt_heap_bind(&heap, "myheap", TM_INFINITE);
    printf("czekam na mutex\n");
    rt_mutex_bind(&mutex, "mymutex", TM_INFINITE);
    rt_heap_alloc(&heap, 0, TM_INFINITE, (void **)&ptr);
    // printf("all ok");
    while (1)
    {
        rt_mutex_acquire(&mutex, TM_INFINITE);
        if (ptr[0] < ptr[1])
        {
            flag = 0;
            printf("First task at position %d and last at %d\n", ptr[0], ptr[1] - 2);
            char info[10];
            sprintf(info, "%i", ptr[0] / 2);
            rt_task_create(&task, info, 0, ptr[ptr[0] + 1], 0);
            rt_task_start(&task, &serve, &ptr[ptr[0]]);
            rt_sem_p(&sem, TM_INFINITE);
            ptr[0] = ptr[0] + 2;
        }
        else
        {
            if (flag == 0)
            {
                printf("all tasks done\n");
                flag = 1;
            }
        }
        rt_mutex_release(&mutex);
    }
    pause();
}