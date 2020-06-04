#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <alchemy/task.h>
#include <alchemy/heap.h>
#include <alchemy/mutex.h>
#include <alchemy/sem.h>
#include <time.h>

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
    now = clock();
    printf("thread %s: endtime: %lf\n", info.name, now / (double)CLOCKS_PER_SEC);
    rt_sem_v(&sem);
}

int main(int a, char **b)
{
    time_t now = clock();
    float new_now = now / (double)CLOCKS_PER_SEC;
    int *ptr;
    int candid[2];
    // candid[0] = position
    // candid[1] = deadline
    candid[0] = 3;
    candid[1] = 10000;
    int schedule = 0;
    int picked = 0;
    mlockall(MCL_CURRENT | MCL_FUTURE);
    rt_sem_create(&sem, "semafor", 0, S_PULSE);
    printf("czekam na stertę\n");
    rt_heap_bind(&heap, "myheap", TM_INFINITE);
    printf("czekam na mutex\n");
    rt_mutex_bind(&mutex, "mymutex", TM_INFINITE);
    rt_heap_alloc(&heap, 0, TM_INFINITE, (void **)&ptr);
    printf("all ok\n");
    while (1)
    {
        rt_mutex_acquire(&mutex, TM_INFINITE);
        //check if there are any tasks to execute, if there are, stop checking and set flag to 1
        for (int k = ptr[0]; k < ptr[1]; k = k + 4)
        {
            if (ptr[k + 3] == 1)
            {
                schedule = 1;
                break;
            }
        }

        if (schedule == 1)
        {
            for (int i = ptr[0]; i < ptr[1]; i = i + 4)
            {
                if (ptr[i + 3] == 1)
                {
                    now = clock();
                    new_now = now / (double)CLOCKS_PER_SEC;

                    if ((ptr[i] <= new_now) && (ptr[i + 1] + new_now <= ptr[i] + ptr[i + 2]))
                    {
                        if (candid[1] >= ptr[i + 2])
                        {
                            picked = 1;
                            candid[0] = i;
                            candid[1] = ptr[i + 2];
                        }
                    }
                }
            }
            if (picked == 1)
            {
                picked = 0;
                printf("current candidate is %i with deadline %i\n", (candid[0] - 3) / 4, candid[1]);
                char info[10];
                sprintf(info, "%i", (candid[0] - 3) / 4); // current task real number
                rt_task_create(&task, info, 0, 0, 0);
                rt_task_start(&task, &serve, &ptr[candid[0] + 1]);
                rt_sem_p(&sem, TM_INFINITE);
            }
            ptr[candid[0] + 3] = 2;
            schedule = 0;
        }
        candid[0] = -1;
        candid[1] = 10000;
        rt_mutex_release(&mutex);
    }
    pause();
}