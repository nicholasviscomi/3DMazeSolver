#include "lib/struct.h"
#include <stdio.h>
#include <stdlib.h>

void PrintQ(Queue q) {}

Queue NewQueue() {
    Queue q;// = malloc(sizeof(Queue));
    q = (Queue) {
        .head = NULL,
        .tail = NULL,
        .size = 0
    };
    return q;
}

void Enqueue(Queue *q, GNode *val) {
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

/*
Track paths:
GNode *paths[nnodes]:
    - initialize to NULL
    - indexed by GNode.id
    - each entry is the previous node in the path to the current node 
        - "current node" meaning the node with id == index; find this node in nodes array in main.c

    - to trace path:
        - go to paths[len(paths) - 1], the end node 
        - this points to the previous node in the path; say it has id 50
        - go to paths[50], find id of previous node, use that index
        - continue until id == 0 (meaning the root node was found)
------------------

Track visited nodes:
    - Node is visited if paths[i] != NULL
-------------------
*/
void BreadthFirstSearch(GNode *head, int nnodes) {
    Queue search = NewQueue();
    GNode *paths[nnodes];

}