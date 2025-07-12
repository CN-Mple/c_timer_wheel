#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../include/timer_wheel.h"

void test_task(void *arg)
{
        uint32_t task_id = (uint32_t)arg;
        printf("Task %d executed!\n", task_id);
}

int add_task(struct Timer_Wheel *timer_wheel, uint32_t delay_ms, void (*task_entry)(void *), void *arg)
{
        struct Timer_Task *task = malloc(sizeof(struct Timer_Task));
        if (!task) {
                printf("Error: Failed to allocate memory for task!\n");
                return -1;
        }
        memset(task, 0, sizeof(struct Timer_Task));

        task->task_entry = task_entry;
        task->arg = arg;

        printf("Adding task with delay %u ms\n", delay_ms);
#if 0
        if (timer_wheel_insert(timer_wheel, task, delay_ms) != 0) {
                printf("Error: Failed to insert task into timer wheel!\n");
                free(task);
                return -1;
        }
#else
        if (timer_wheel_insert_push(timer_wheel, task, delay_ms) != 0) {
                printf("Error: Failed to insert task into timer wheel!\n");
                free(task);
                return -1;
        }
#endif
        return 0;
}

struct Timer_Wheel *timer_wheel_create(uint32_t wheel_size)
{
        struct Timer_Wheel *timer_wheel = malloc(sizeof(struct Timer_Wheel));
        if (!timer_wheel) {
                printf("Error: Failed to allocate memory for Timer_Wheel!\n");
                return NULL;
        }
        memset(timer_wheel, 0, sizeof(struct Timer_Wheel));
        timer_wheel->wheel_size = wheel_size;

        timer_wheel->slots = malloc(sizeof(struct List_Node) * timer_wheel->wheel_size);
        if (!timer_wheel->slots) {
                printf("Error: Failed to allocate memory for slots in Timer_Wheel!\n");
                free(timer_wheel);
                return NULL;
        }

        memset(timer_wheel->slots, 0, sizeof(struct List_Node) * timer_wheel->wheel_size);

        for (uint32_t i = 0; i < timer_wheel->wheel_size; i++) {
                timer_wheel->slots[i].prev = &timer_wheel->slots[i];
                timer_wheel->slots[i].next = &timer_wheel->slots[i];
        }

        printf("Timer_Wheel created with size %u\n", wheel_size);

        return timer_wheel;
}

void timer_wheel_destory(struct Timer_Wheel *timer_wheel)
{
        if (!timer_wheel) {
                printf("Error: Timer_Wheel is NULL!\n");
                return;
        }

        if (!timer_wheel->slots) {
                printf("Warning: Timer_Wheel has no slots to free!\n");
                free(timer_wheel);
                return;
        }

        free(timer_wheel->slots);
        free(timer_wheel);

        printf("Timer_Wheel destroyed successfully\n");
}

int timer_wheel_init(struct Timer_Wheel *timer_wheel, uint32_t tick_ms, uint32_t now_tick)
{
        if (!timer_wheel) {
                printf("Error: Timer_Wheel is NULL during initialization!\n");
                return -1;
        }

        timer_wheel->tick_ms = tick_ms;
        timer_wheel->current_tick = now_tick;

        timer_wheel->overflow_wheel = NULL;

        printf("Timer_Wheel initialized with tick_ms: %u, now_tick: %u\n", tick_ms, now_tick);

        return 0;
}

void timer_task_free(struct Timer_Task *task)
{
        if (!task) {
                printf("Error: Task is NULL during free!\n");
                return;
        }

        free(task);
        printf("Task freed successfully\n");
}

int test(void)
{
    printf("Starting test...\n");

    struct Timer_Wheel *timer_wheel_v1 = timer_wheel_create(60);
    if (!timer_wheel_v1) {
        return -1;
    }
    timer_wheel_init(timer_wheel_v1, 1, 0);
    
    struct Timer_Wheel *timer_wheel_v2 = timer_wheel_create(60);
    if (!timer_wheel_v2) {
        return -1;
    }
    timer_wheel_init(timer_wheel_v2, 60, 0);

    struct Timer_Wheel *timer_wheel_v3 = timer_wheel_create(24);
    if (!timer_wheel_v3) {
        return -1;
    }
    timer_wheel_init(timer_wheel_v3, 3600, 0);

    timer_wheel_v1->overflow_wheel = timer_wheel_v2;
    timer_wheel_v2->overflow_wheel = timer_wheel_v3;

    printf("Tasks being added...\n");

    for (uint32_t i = 0; i < 3; i++) {
        if (add_task(timer_wheel_v1, i, test_task, (void *)i) != 0) {
            printf("Error: Failed to add task %u\n", i);
        }
    }

    if (add_task(timer_wheel_v1, 350, test_task, (void *)350) != 0) {
        printf("Error: Failed to add task with delay 350\n");
    }
    if (add_task(timer_wheel_v1, 600, test_task, (void *)600) != 0) {
        printf("Error: Failed to add task with delay 600\n");
    }
    if (add_task(timer_wheel_v1, 650, test_task, (void *)650) != 0) {
        printf("Error: Failed to add task with delay 650\n");
    }
    if (add_task(timer_wheel_v1, 864, test_task, (void *)864) != 0) {
        printf("Error: Failed to add task with delay 864\n");
    }
    if (add_task(timer_wheel_v1, 957, test_task, (void *)957) != 0) {
        printf("Error: Failed to add task with delay 957\n");
    }
    if (add_task(timer_wheel_v1, 24 * 60 * 60 + 1, test_task, (void *)957) != 0) {
        printf("Error: Failed to add task with delay 957\n");
    }

    printf("Starting timer wheel loop...\n");
    for (uint32_t tick = 0; tick < 1000; tick++) {
#if 0
        timer_wheel_loop(timer_wheel_v1, tick);
#else
        timer_wheel_loop_push(timer_wheel_v1);
#endif
        }

    printf("Destroying timer wheels...\n");

    timer_wheel_destory(timer_wheel_v3);
    timer_wheel_destory(timer_wheel_v2);
    timer_wheel_destory(timer_wheel_v1);

    return 0;
}

int main(void)
{
    return test();
}
