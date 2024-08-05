#ifndef MAIN
#define MAIN

#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "ui.h"
#include "algo.h"

#define MIN_WIDTH 3
#define MAX_WIDTH 5

#define MIN_HEIGHT 3
#define MAX_HEIGHT 5

#define MIN_LAYERS 5
#define MAX_LAYERS 7

#define vadd Vector3Add

#define DROPOUT 0.3333333

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

typedef struct {
    Sphere node;
    void* children; 
    int n_children;
    // unique id to prevent drawing the same node multiple times; 
    // will be the index of the sphere in spheres array
    int id; 
    int weight; // how expensive it is to travel to this Node
} GNode;
//============================

// Declared here so it can be accessed by algo.c 
GNode root, end_node; // must be declared out here or malloc'ed so it doesn't get destroyed!
GNode** nodes;

#endif