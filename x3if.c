#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <alchemy/task.h>
#include <alchemy/heap.h>
#include <alchemy/mutex.h>

int main(int a, char **b)
{
    long long int release_time, execution_time, deadline;
    int *ptr;
    RT_HEAP heap;
    RT_MUTEX mutex;

    mlockall(MCL_CURRENT | MCL_FUTURE);
    rt_heap_create(&heap, "myheap", 1000, H_SINGLE | H_PRIO);
    rt_heap_alloc(&heap, 0, TM_INFINITE, (void **)&ptr);
    rt_mutex_create(&mutex, "mymutex");
    // first = ptr[0]
    // last = ptr[1]
    // current = ptr[2]
    ptr[0] = 3;
    ptr[1] = 3;
    ptr[2] = 3;
    while (1)
    {
        printf("wprowadz czas rozpoczecia, czas trwania oraz deadline\n");
        scanf("%i %i %i", &release_time, &execution_time, &deadline);
        rt_mutex_acquire(&mutex, TM_INFINITE);
        if (ptr[1] < 1000)
        {
            ptr[ptr[1]] = release_time;
            ptr[ptr[1] + 1] = execution_time;
            ptr[ptr[1] + 2] = deadline;
            ptr[ptr[1] + 3] = 1;
            printf("last added task at postition %d with release time %d, execution time %d and deadline %d (flag %d)\n",ptr[1], ptr[ptr[1]], ptr[ptr[1] + 1], ptr[ptr[1] + 2], ptr[ptr[1] + 3]);
            ptr[1] = ptr[1] + 4;
        }
        rt_mutex_release(&mutex);
    }
    pause();
}