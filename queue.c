#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"


void quick_sort(struct list_head *left_edge,
                struct list_head *right_edge,
                bool descend);

char *value_of(struct list_head *target);

int comparison(struct list_head *lh1, struct list_head *lh2, bool descend);

void swap(struct list_head *lh1, struct list_head *lh2);

struct list_head *merge(struct list_head *left,
                        struct list_head *right,
                        bool descend);

void delete_node(struct list_head *node);

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        free(entry->value);
        free(entry);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;
    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }
    list_add(&new->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;
    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }
    list_add_tail(&new->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || head->next == head)
        return NULL;
    element_t *e = list_entry(head->next, element_t, list);
    if (sp) {
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(head->next);
    return e;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || head->prev == head)
        return NULL;
    element_t *e = list_entry(head->prev, element_t, list);
    if (sp) {
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(head->prev);
    return e;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *li;
    list_for_each (li, head) {
        len++;
    }
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || head->next == head)
        return false;
    struct list_head **indir = &head->next;
    for (struct list_head *fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next) {
        indir = &(*indir)->next;
    }
    delete_node(*indir);
    return true;
}

void delete_node(struct list_head *node)
{
    element_t *e = list_entry(node, element_t, list);
    list_del(node);
    free(e->value);
    free(e);
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;
    struct list_head *cur = head->next;
    while (cur != head) {
        bool has_deleted = false;
        struct list_head *next = cur->next;
        while (next != head && comparison(cur, next, false) == 0) {
            has_deleted = true;
            cur = next;
            next = next->next;
            delete_node(cur->prev);
        }
        cur = next;
        if (has_deleted) {
            delete_node(cur->prev);
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    struct list_head *first = head->next;
    struct list_head *second = first->next;
    while (first != head && second != head) {
        struct list_head *temp = first->prev;
        first->prev->next = second;
        second->next->prev = first;
        first->prev = second;
        first->next = second->next;
        second->prev = temp;
        second->next = first;
        first = first->next;
        second = first->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    struct list_head *node = head->next;
    struct list_head *safe = node->next;
    struct list_head *finished = head;
    struct list_head *end = head->prev;
    while (node != end) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        finished->prev->next = node;
        node->prev = finished->prev;
        finished->prev = node;
        node->next = finished;
        finished = node;
        node = safe;
        safe = node->next;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || k < 2)
        return;
    struct list_head *left = head;
    struct list_head *right = left->next;
    while (1) {
        for (int i = 1; i <= k; i++) {
            if (right == head) {
                return;
            }
            right = right->next;
        }
        for (struct list_head *cur = left->next; cur != right;) {
            struct list_head *next = cur->next;
            cur->next = cur->prev;
            cur->prev = next;
            cur = next;
        }
        right->prev->prev = left;
        left->next->next = right;
        struct list_head *temp = right->prev;
        right->prev = left->next;
        left->next = temp;
        left = right->prev;
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head)
        return;
    quick_sort(head->next, head->prev, descend);
}

void quick_sort(struct list_head *left_edge,
                struct list_head *right_edge,
                bool descend)
{
    if (left_edge == right_edge)
        return;
    struct list_head *key = left_edge;
    struct list_head *left = left_edge->next;
    struct list_head *right = right_edge;
    while (1) {
        while (left != right->next) {
            if (comparison(key, left, descend) < 0)
                break;
            left = left->next;
        }
        while (left != right->next) {
            if (comparison(key, right, descend) > 0)
                break;
            right = right->prev;
        }
        if (left == right->next)
            break;
        swap(left, right);
    }
    swap(key, right);
    if (right != left_edge)
        quick_sort(left_edge, right->prev, descend);
    if (right != right_edge)
        quick_sort(right->next, right_edge, descend);
}

char *value_of(struct list_head *target)
{
    return list_entry(target, element_t, list)->value;
}

int comparison(struct list_head *lh1, struct list_head *lh2, bool descend)
{
    int cmp = strcmp(value_of(lh1), value_of(lh2));
    return descend ? -cmp : cmp;
}

void swap(struct list_head *lh1, struct list_head *lh2)
{
    element_t *e1 = list_entry(lh1, element_t, list);
    element_t *e2 = list_entry(lh2, element_t, list);
    char *temp = e1->value;
    e1->value = e2->value;
    e2->value = temp;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    for (struct list_head *cur = head->prev; cur != head; cur = cur->prev) {
        while (cur->prev != head && comparison(cur, cur->prev, false) < 0) {
            delete_node(cur->prev);
        }
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    for (struct list_head *cur = head->prev; cur != head; cur = cur->prev) {
        while (cur->prev != head && comparison(cur, cur->prev, false) > 0) {
            delete_node(cur->prev);
        }
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    if (head->next == head)
        return 0;
    queue_contex_t *entry, *safe;
    struct list_head *target = NULL;
    list_for_each_entry_safe (entry, safe, head, chain) {
        target = merge(target, entry->q, descend);
    }
    return q_size(target);
}

struct list_head *merge(struct list_head *left,
                        struct list_head *right,
                        bool descend)
{
    if (left == NULL)
        return right;
    struct list_head *left_head = left;
    left = left->next;
    struct list_head *right_head = right;
    right = right->next;
    struct list_head *cur = left_head;
    while (left != left_head && right != right_head) {
        if (comparison(left, right, descend) < 0) {
            left = left->next;
        } else {
            right = right->next;
            left->prev = right->prev;
            cur->next = right->prev;
            right->prev->next = left;
            right->prev->prev = cur;
        }
        cur = cur->next;
    }
    if (right != right_head) {
        cur->next = right;
        right->prev = cur;
        left_head->prev = right_head->prev;
        right_head->prev->next = left_head;
    }
    right_head->prev = right_head;
    right_head->next = right_head;
    return left_head;
}
