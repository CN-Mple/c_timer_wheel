#include "../include/timer_wheel.h"

static void list_node_insert_front(struct List_Node *new, struct List_Node *node)
{
        new->next = node;
        new->prev = node->prev;

        node->prev->next = new;
        node->prev = new;
}

static void list_node_insert_back(struct List_Node *new, struct List_Node *node)
{
        new->next = node->next;
        new->prev = node;

        node->next->prev = new;
        node->next = new;
}

struct List_Node *list_node_delete(struct List_Node *node)
{
        struct List_Node *next_node = node->next;
        struct List_Node *prev_node = node->prev;

        prev_node->next = next_node;
        next_node->prev = prev_node;

        return node;
}

int timer_wheel_insert(struct Timer_Wheel *timer_wheel, struct Timer_Task *task, uint32_t delay_ms)
{
        task->due_time = timer_wheel->current_tick + delay_ms;
        uint32_t interval = timer_wheel->tick_ms * timer_wheel->wheel_size;
        if (delay_ms > interval) {
                if (timer_wheel->overflow_wheel == NULL) {
                        return -1;
                }
                timer_wheel->overflow_wheel->current_tick = timer_wheel->current_tick;
                return timer_wheel_insert(timer_wheel->overflow_wheel, task, delay_ms);
        }

        uint32_t index = (task->due_time % (timer_wheel->wheel_size * timer_wheel->tick_ms)) / timer_wheel->tick_ms;

        const struct Timer_Task *head = (struct Timer_Task*)&timer_wheel->slots[index];
        struct Timer_Task *current = (struct Timer_Task*)head->node.next;

        while (current != head) {
                if (task->due_time < current->due_time) {
                        list_node_insert_front((struct List_Node *)task, (struct List_Node *)current);
                        return 0;
                }
                current = (struct Timer_Task*)current->node.next;
        }
        list_node_insert_front((struct List_Node *)task, (struct List_Node *)current);

        return 0;
}

struct Timer_Task *timer_wheel_tick(struct Timer_Wheel *timer_wheel, uint32_t tick)
{
        timer_wheel->current_tick = tick;
        uint32_t index = (tick % (timer_wheel->wheel_size * timer_wheel->tick_ms)) / timer_wheel->tick_ms;
        struct Timer_Task *head;
        struct Timer_Task *current;
        if (!index && timer_wheel->overflow_wheel) {
                /*
                        Get task chain on the slot from high wheel.
                        Recalculate the arrival time to insert the current wheel.
                */
                head = timer_wheel_tick(timer_wheel->overflow_wheel, tick);
                current = (struct Timer_Task *)head->node.next;
                while (current != head) {
                        struct Timer_Task *task = (struct Timer_Task *)list_node_delete((struct List_Node *)current);
                        uint32_t delay_ms = task->due_time - tick;
                        timer_wheel_insert(timer_wheel, task, delay_ms);
                        current = (struct Timer_Task *)head->node.next;
                }
        }
        head = (struct Timer_Task *)&timer_wheel->slots[index];
        return head;
}

void timer_wheel_loop(struct Timer_Wheel *timer_wheel, uint32_t tick)
{
        /* exec task and free task */
        const struct Timer_Task *head = timer_wheel_tick(timer_wheel, tick);
        struct Timer_Task *current = (struct Timer_Task *)head->node.next;
        while (current != head) {
                current->task_entry(current->arg);
                struct Timer_Task *task = (struct Timer_Task *)list_node_delete((struct List_Node *)current);
                current = (struct Timer_Task *)task->node.next;
                timer_task_free(task);
        }
}

int timer_wheel_insert_push(struct Timer_Wheel *timer_wheel, struct Timer_Task *task, uint32_t delay_ms)
{
        uint32_t interval = timer_wheel->tick_ms * timer_wheel->wheel_size;
        /* Commit upward if overflow */
        if (delay_ms > interval) {
                if (timer_wheel->overflow_wheel == NULL) {
                        printf("lose task out of delay.\r\n");
                        return -1;
                }
                return timer_wheel_insert_push(timer_wheel->overflow_wheel, task, delay_ms);
        }
        task->due_time = delay_ms;

        /* Search right place to insert */
        uint32_t index = (timer_wheel->current_tick + delay_ms / timer_wheel->tick_ms) % timer_wheel->wheel_size;
        struct Timer_Task *head = (struct Timer_Task*)&timer_wheel->slots[index];
        struct Timer_Task *current = (struct Timer_Task*)head->node.next;
        while (current != head) {
                if (task->due_time < current->due_time) {
                        break;
                }
                current = (struct Timer_Task*)current->node.next;
        }
        list_node_insert_front((struct List_Node *)task, (struct List_Node *)current);

        return 0;
}

struct Timer_Task *timer_wheel_tick_push(struct Timer_Wheel *timer_wheel)
{
        timer_wheel->current_tick = (timer_wheel->current_tick + 1) % timer_wheel->wheel_size;

        uint32_t index = timer_wheel->current_tick;
        struct Timer_Task *head;
        struct Timer_Task *current;
        if (!index && timer_wheel->overflow_wheel) {
                /*
                        Get task chain on the slot from high wheel.
                        Recalculate the arrival time to insert the current wheel.
                */
                head = timer_wheel_tick_push(timer_wheel->overflow_wheel);
                current = (struct Timer_Task *)head->node.next;
                while (current != head) {
                        struct Timer_Task *task = (struct Timer_Task *)list_node_delete((struct List_Node *)current);
                        uint32_t delay_ms = task->due_time - timer_wheel->overflow_wheel->current_tick * timer_wheel->wheel_size * timer_wheel->tick_ms;
                        timer_wheel_insert_push(timer_wheel, task, delay_ms);
                        current = (struct Timer_Task *)head->node.next;
                }
        }
        /* Return task chain on the slot */
        head = (struct Timer_Task *)&timer_wheel->slots[index];
        return head;
}

void timer_wheel_loop_push(struct Timer_Wheel *timer_wheel)
{
        /* exec task and free task */
        const struct Timer_Task *head = timer_wheel_tick_push(timer_wheel);
        struct Timer_Task *current = (struct Timer_Task *)head->node.next;
        while (current != head) {
                current->task_entry(current->arg);
                struct Timer_Task *task = (struct Timer_Task *)list_node_delete((struct List_Node *)current);
                current = (struct Timer_Task *)task->node.next;
                timer_task_free(task);
        }
}
