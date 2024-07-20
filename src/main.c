#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


#define ZOOM_MAX 300
#define ZOOM_MIN 10

#define MAX_WIDTH 5
#define MAX_HEIGHT 5

#define MAX_LAYERS 2
#define MIN_LAYERS 2

#define DROPOUT 0.3

#define vadd Vector3Add

const int WIDTH = 900, HEIGHT = 600;

float theta = 0;
int horiz_offset = 1;


float veclen(Vector3 v) {
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

Vector3 vecmul(Vector3 v, float m) {
    return (Vector3) { v.x * m, v.y * m, v.z * m };
}

// prints vector only
void vecprint(Vector3 v) {
    printf("{%f, %f, %f}", v.x, v.y, v.z);
}

// zoom the camera position in/out by a factor
// multiply the x, y, z components by the factor
// E.g. factor of 2 will double the distance from the camera target (aka zoom out by 100%)
Vector3 Zoom(Vector3 v, float factor) {
    Vector3 new_v = {
        v.x * factor,
        v.y * factor,
        v.z * factor,
    };

    // if out max/min zoom limit, return OG vector (no more zooming)
    if (veclen(new_v) > ZOOM_MAX || veclen(new_v) < ZOOM_MIN) {
        return v;
    }
    
    return new_v;
}

// update x and z vectors for a theta radian rotation around the y axis
Vector3 RotateY(Vector3 pos, float theta) {
    // | cos θ    0   sin θ| |x|   | x cos θ + z sin θ|   |x'|
    // |   0      1       0| |y| = |         y        | = |y'|
    // |−sin θ    0   cos θ| |z|   |−x sin θ + z cos θ|   |z'|
    float new_x =  pos.x * cos(theta) + pos.z * sin(theta);
    float new_z = -pos.x * sin(theta) + pos.z * cos(theta);

    return (Vector3) { new_x, pos.y, new_z };
}


typedef struct {
    Vector3 pos;
    Color c;
    float radius;
} Sphere;
Sphere spheres[1000];
int *layer_sizes;
int n_layers = 0;
int n_spheres = 0;

float spacing = 20;
float layer_spacing = 50;

Color colors[] = { RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE };
float radius = 2;


void DrawAxes() {
    DrawCylinderEx((Vector3) {0, 0, 0}, (Vector3) {50, 0, 0}, 0.5, 0.5, 30, GRAY);  // x-axis
    DrawCylinderEx((Vector3) {0, 0, 0}, (Vector3) {0, 0, 50}, 0.5, 0.5, 30, BLACK); // z-axis
}

// random integer between two bounds, inclusive
int randRange(int low, int high) {
    return (rand() % (high + 1 - low)) + low;
}

typedef struct {
    Sphere node;
    void* children; 
    int n_children;
    // unique id to prevent drawing the same node multiple times; 
    // will be the index of the sphere in spheres array
    int id; 
} GNode;
GNode root, end_node; // must be declared out here or malloc'ed so it doesn't get destroyed!
GNode *nodes[MAX_LAYERS * MAX_HEIGHT * MAX_WIDTH];
int nnodes = 0;

/*
GNode* curr_layer = [root]

for i in 0...n_layers:
    
    ---- make next layer ----
    GNode* next_layer = malloc(sizeof(GNode) * width * height)
    for x in width:
        for z in height:
            next_layer[x * width + z] = (GNode) {...}


    ---- connect current to next ----
    for (j = 0; j < (width*height); j++):
        curr_node = curr_layer[j] // node is an address
        for (int k = 0; k < width * height; k++):
            GNode* child = next_layer[k]
            (*node).children.append(child)   


    curr_layer = next_layer
*/

/*
Used for making an array of GNode pointers for one layer in the maze. Width and height are 
for the number of balls in the row or column; depth is the number of the current layer;
these will be used to calculate the correct position so they can be easily displayed
*/
GNode** make_layer(size_t width, size_t height, size_t depth) {
    GNode** next_layer = malloc(sizeof(GNode*) * width * height);

    float w_offset = ((width - 1) * spacing) / 2; 
    float h_offset = ((height - 1) * spacing) / 2;
    for (size_t z = 0; z < width; z++) {
        for (size_t y = 0; y < height; y++) {
            //! must malloc new space and then assign the value or else it will use the same 
            //! memory location in multiple iterations
            GNode* new_node = malloc(sizeof(GNode));
            
            *new_node = (GNode) {
                .node = (Sphere) {
                    (Vector3) { depth * layer_spacing, y * spacing - h_offset, z * spacing - w_offset},
                    colors[depth % 6], radius
                },
                .children = NULL,
                .n_children = 0,
                .id = 0 // will be updated in InitNodes()
            };
            vecprint((*new_node).node.pos);
            next_layer[z * height + y] = new_node;
        }
    }

    return next_layer;
}

void shuffle(GNode** arr, int n) {

    for (int i = n - 1; i > 0; i--) {
        int j = randRange(0, i);
        GNode* temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }

}

void InitNodes() {
    root = (GNode) {
        (Sphere) {
            (Vector3) {-1 * layer_spacing, 0, 0},
            WHITE, radius + 2
        },
        NULL,
        0
    };

    end_node = (GNode) {
        (Sphere) {
            (Vector3) { n_layers * layer_spacing, 0, 0 },
            WHITE,
            radius + 2
        },
        .children = NULL,
        .n_children = 0
    };

    GNode** curr_layer = malloc(sizeof(GNode *));
    curr_layer[0] = &root;
    int curr_size = 1;

    for (size_t i = 0; i <= n_layers; i++) {
        GNode** next_layer; int width, height;

        if (i == n_layers) {
            // if at the last layer, connect them to the final node
            next_layer = malloc(sizeof(GNode *));
            next_layer[0] = &end_node;
            width = 1; height = 1;
        } else {
            width = randRange(2, MAX_WIDTH);
            height = randRange(2, MAX_HEIGHT); 
            next_layer = make_layer(width, height, i);
        }

        for (int j = 0; j < curr_size; j++) {

            curr_layer[j]->children = malloc(sizeof(GNode) * width * height);

            curr_layer[j]->id = nnodes;
            //* add to array for easy display of nodes 
            nodes[nnodes++] = curr_layer[j];

            for (int k = 0; k < width * height; k++) {
                //! since some nodes are getting skipped, k is NOT the next contiguous index
                int idx = curr_layer[j]->n_children; 

                // de-reference curr_layer[j] to get curr_node; cast children to GNode pointer array from void*
                ((GNode **) curr_layer[j]->children)[idx] = next_layer[k]; 
                curr_layer[j]->n_children += 1;
            }

            shuffle((GNode**) curr_layer[j]->children, curr_layer[j]->n_children);

            int n = curr_layer[j]->n_children;
            int new_size = ((int) n * DROPOUT);

            if (new_size == 0) {
                /*
                When connecting to the last layer, if DROPOUT < 0.5, new_size will always be 0 because
                any number times <0.5 rounded to an integer will be 0. Therefore we need to redefine 
                new_size to have 1/3 chance of being 1 or 0
                */
                new_size = randRange(1, 3) == 1 ? 0 : 1;
            }
            
            if (i > 0) { // don't dropout children on root
                curr_layer[j]->children = realloc(curr_layer[j]->children, sizeof(GNode*) * new_size);
                curr_layer[j]->n_children = new_size;
            }
        }


        curr_layer = next_layer;
        curr_size = width * height;
    }

    nodes[nnodes++] = &end_node;
}


void DrawNodes() {
    Vector3 offset = (Vector3) { -layer_spacing * horiz_offset, 0, 0};


    for (int i = 0; i < nnodes; i++) {
        GNode* g = nodes[i];
        DrawSphere(vadd(g->node.pos, offset), g->node.radius, g->node.c);

        for (int j = 0; j < g->n_children; j++) {
            GNode** children = g->children;

            DrawCylinderEx(
                vadd(g->node.pos, offset), 
                vadd(children[j]->node.pos, offset), 
                radius/5, radius/5, 20, 
                (Color) {0,0,0,50}
            );

        }
    }


}

int main(void) {
    srand(time(NULL));

    InitWindow(WIDTH, HEIGHT, "игра");

    Camera3D camera =   { 0 };
    camera.position =   (Vector3) { 61.945072, 61.634838, 148.785477 };
    camera.target =     (Vector3) { 0.0f, 0.0f, 0.0f };      // Camera looking at point (0, 0, 0)
    camera.up =         (Vector3) { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy =       45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type    

    n_layers = randRange(MIN_LAYERS, MAX_LAYERS);
    printf("%d layers\n", n_layers);
    InitNodes(); 

    // SetTargetFPS(60);

    while (!WindowShouldClose()) {        
        // RainbowRectangles();
        
        if (IsKeyPressed('O') || IsKeyPressedRepeat('O')) {
            camera.position = Zoom(camera.position, 1.10);
            vecprint(camera.position);
        }
        if (IsKeyPressed('I') || IsKeyPressedRepeat('I')) {
            camera.position = Zoom(camera.position, 0.90);
            vecprint(camera.position);
        }
        if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
            Vector3 new = (Vector3) { camera.position.x, camera.position.y + 10, camera.position.z };
            
            camera.position = vecmul(new, veclen(camera.position)/veclen(new));
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) {
            Vector3 new = (Vector3) { camera.position.x, camera.position.y - 10, camera.position.z };

            camera.position = vecmul(new, veclen(camera.position)/veclen(new));
        }

        // Change horizontal offset which controls how many spaces the balls get shifted
        // along the x-axis. Create the same effect as moving the camera by moving the objects
        // because idk how to shift the camera tbh
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
            horiz_offset -= 1;
        }
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
            horiz_offset += 1;
        }


        float m = GetMouseWheelMove();
        if (m > 0) {
            Vector3 new = RotateY(camera.position, (-2 * PI / 100) * m);
            camera.position = new;
            // vecprint(camera.position);
        } else if (m < 0) {
            // ! m < 0, so use its abs to scaled the rotation according to scroll velocity
            Vector3 new = RotateY(camera.position,  (2 * PI / 100) * fabsf(m));
            camera.position = new;
            // vecprint(camera.position);
        }  

        BeginDrawing();

            ClearBackground(DARKGRAY);

            BeginMode3D(camera);
                // DrawAxes();
                // CubeSpheres();
                // set to all zeros
                
                DrawNodes();

            EndMode3D();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
// gcc src/main.c $(pkg-config --libs --cflags raylib) -o bin/main && ./bin/main