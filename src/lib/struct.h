#ifndef STRUCT_C
#define STRUCT_C

#include "raylib.h"

//============================
// TYTPE DEFINITIONS
//============================
typedef struct {
    float total;    // both start out as # of seconds until completion; 
    float lifetime; // liftetime gets decremented to 0 accordingly
} Timer;

typedef struct {
    Vector3 pos;
    Color c;
    float radius;
} Sphere;

typedef struct GNode {
    Sphere node;
    void* children; 
    int n_children;
    // unique id to prevent drawing the same node multiple times; 
    // will be the index of the sphere in spheres array
    int id; 
    int weight; // how expensive it is to travel to this Node
} GNode;
//============================

typedef struct Node {
    GNode *val;
    struct Node* next;
} Node;

typedef struct {
    Node *head, *tail;
    int size;
} Queue;

typedef struct {
    void (*handler)(); // event handler: no params, no return
    char* text;
    int x, y, width, height, font_size;
    Color bg, tcolor;
    char key; // letter to press on keyboard to call handler (0 for none)
} Button;

#endif