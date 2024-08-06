#ifndef ALGO_H
#define ALGO_H

#include "struct.h"

void PrintQ(Queue q);

Queue NewQueue();

void Enqueue(Queue *q, GNode *val);

Node* Dequeue(Queue *q);

void BreadthFirstSearch(GNode *head, int nnodes);

#endif