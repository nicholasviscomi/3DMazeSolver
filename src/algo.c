#include "algo.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>

void PrintQ(Queue q) {
    printf("------------------------------\n");
    Node *curr = q.head;
    while (curr != NULL) {
        printf("%d ——> ", curr->val);
        curr = curr->next;
    }
    printf("END");
    printf("\n------------------------------\n");
}

Queue NewQueue() {
    Queue q;// = malloc(sizeof(Queue));
    q = (Queue) {
        .head = NULL,
        .tail = NULL,
        .size = 0
    };
    return q;
}

void Enqueue(Queue *q, int val) {
    Node* n = malloc(sizeof(Node *));
    *n = (Node) {
        .next = NULL,
        .val = val
    };

    if (q->head == NULL) {
        q->head = n;
        q->tail = n;
    } else {
        q->tail->next = n;
        q->tail = n;
    }
    q->size += 1;
}

Node* Dequeue(Queue *q) {
    if (q->head == NULL) {
        q->size = 0;
        return NULL;
    }

    Node* res = q->head;
    if (q->head->next == NULL) {
        // head is only element, return it
        q->head = NULL;
        q->tail = NULL;
        q->size = 0;
    } else {
        // return head and point it to next node
        q->head = q->head->next;
        q->size -= 1;
    }

    return res;
}



// given a starting node, add all children to end of linked list
void AddChildren() {
    
}

void BreadthFirstSearch() {
    Queue q = NewQueue();

}

int main(int argc, char const *argv[])
{
    Queue q = NewQueue();

    Enqueue(&q, 1);
    Enqueue(&q, 2);
    Enqueue(&q, 3);
    Enqueue(&q, 4);
    Enqueue(&q, 5);

    PrintQ(q);

    Dequeue(&q);
    PrintQ(q);

    Dequeue(&q);
    Dequeue(&q);
    Dequeue(&q);
    Dequeue(&q);
    Dequeue(&q);
    Dequeue(&q);
    Dequeue(&q);
    Dequeue(&q);
    PrintQ(q);

    Enqueue(&q, 2);
    PrintQ(q);
    
    return 0;
}


