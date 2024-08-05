#ifndef ALGO_H
#define ALGO_H

typedef struct Node {
    int val;
    struct Node* next;
} Node;

typedef struct {
    Node *head, *tail;
    int size;
} Queue;

void PrintQ(Queue q);

Queue NewQueue();

void Enqueue(Queue *q, int val);

Node* Dequeue(Queue *q);


void AddChildren();

void BreadthFirstSearch();

#endif