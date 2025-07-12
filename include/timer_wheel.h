#ifndef __TIMER_WHEEL_H
#define __TIMER_WHEEL_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

struct List_Node {
        struct List_Node *prev;
        struct List_Node *next;
};

struct Timer_Task {
        struct List_Node node;
        uint32_t due_time;
        void (*task_entry)(void *arg);
        void *arg;
};

struct Timer_Wheel {
        uint32_t tick_ms;
        uint32_t current_tick;
        struct List_Node *slots;
        uint32_t wheel_size;
        struct Timer_Wheel *overflow_wheel;
};

int timer_wheel_insert(struct Timer_Wheel *timer_wheel, struct Timer_Task *task, uint32_t delay_ms);
struct Timer_Task *timer_wheel_tick(struct Timer_Wheel *timer_wheel, uint32_t tick);
void timer_wheel_loop(struct Timer_Wheel *timer_wheel, uint32_t tick);

int timer_wheel_insert_push(struct Timer_Wheel *timer_wheel, struct Timer_Task *task, uint32_t delay_ms);
struct Timer_Task *timer_wheel_tick_push(struct Timer_Wheel *timer_wheel);
void timer_wheel_loop_push(struct Timer_Wheel *timer_wheel);

void timer_task_free(struct Timer_Task *task);
#endif /* __TIMER_WHEEL_H */

