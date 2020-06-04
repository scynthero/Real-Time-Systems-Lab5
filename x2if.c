#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <alchemy/task.h>
#include <alchemy/heap.h>
#include <alchemy/mutex.h>

int main(int a, char **b)
{
    long long int span_time, prio;
    int *ptr;
    RT_HEAP heap;
    RT_MUTEX mutex;

    mlockall(MCL_CURRENT | MCL_FUTURE);
    rt_heap_create(&heap, "myheap", 1000, H_SINGLE | H_PRIO);
    rt_heap_alloc(&heap, 0, TM_INFINITE, (void **)&ptr);
    rt_mutex_create(&mutex, "mymutex");
    // first = ptr[0]
    // last = ptr[1]
    ptr[1] = 2;
    ptr[0] = 2;
    while (1)
    {
        printf("wprowadz czas trwania i priorytet oddzielajc spacja\n");
        scanf("%i %i", &span_time, &prio);
        rt_mutex_acquire(&mutex, TM_INFINITE);
        if (ptr[1] < 1000)
        {
            ptr[ptr[1]] = span_time;
            ptr[ptr[1] + 1] = prio;
            printf("last task at postition %d with span time %d and priority %d\n", ptr[1], ptr[ptr[1]], ptr[ptr[1] + 1]);
            ptr[1] = ptr[1] + 2;
        }
        rt_mutex_release(&mutex);
    }
    pause();
}