#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ZOOM_MAX 600
#define ZOOM_MIN 10

#define MIN_WIDTH 3
#define MAX_WIDTH 5

#define MIN_HEIGHT 3
#define MAX_HEIGHT 5

#define MIN_LAYERS 5
#define MAX_LAYERS 7

#define DROPOUT 0.3333333333

#define vadd Vector3Add

const int WIDTH = 1100, HEIGHT = 600;

float theta = 0;
int horiz_offset = 3;


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
int n_layers = 0;

float spacing = 30;
float layer_spacing = 80;

Color colors[] = { RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE };
float radius = 3;


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
GNode** nodes;
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
            width = randRange(MIN_WIDTH, MAX_WIDTH);
            height = randRange(MIN_HEIGHT, MAX_HEIGHT); 
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
            // bias nodes more towards having no children (negative value)
            int new_size = randRange((int) -n * DROPOUT, (int) n * DROPOUT);

            if (i == n_layers) { // special dropout rules for layer leading to end node

                // ~40% chance of not connecting to end node
                if (randRange(1, 10) <= 5) {
                    curr_layer[j]->children = NULL;
                    curr_layer[j]->n_children = 0;
                }

            } else if (i != 0) { // don't dropout children on root
                if (new_size <= 0) {
                    curr_layer[j]->children = NULL;
                    curr_layer[j]->n_children = 0;
                } else {
                    curr_layer[j]->children = realloc(curr_layer[j]->children, sizeof(GNode*) * new_size);
                    curr_layer[j]->n_children = new_size;
                }
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
        Color c = g->node.c;
        if (g->n_children == 0 && g != &end_node) {
            c = ColorAlpha(BLACK, 0.15);
        }
        DrawSphere(vadd(g->node.pos, offset), g->node.radius, c);
        
        for (int j = 0; j < g->n_children; j++) {
            GNode** children = g->children;
            float alpha = 0.7; double rad = 1.25;
            if (children[j]->n_children == 0 && children[j] != &end_node) {
                alpha = 0.15;
                rad = 1;               

                DrawCylinderWiresEx(
                    vadd(g->node.pos, offset), 
                    vadd(children[j]->node.pos, offset), 
                    rad, rad, 10, 
                    ColorAlpha(BLACK, 0.2)
                );
            }

            DrawCylinderEx(
                vadd(g->node.pos, offset), 
                vadd(children[j]->node.pos, offset), 
                rad, rad, 20, 
                ColorAlpha(c, alpha)
            );

        }
    }

}

typedef struct {
    void (*handler)(); // event handler: no params, no return
    char* text;
    int x, y, width, height, font_size;
    Color bg, tcolor;
} Button;

Button NewButton(char* text, int x, int y, int font_size, void (*handler)()) {
    return (Button) {
        .text = text,
        .x = x,
        .y = y,
        .width = MeasureText(text, font_size),
        .height = font_size,
        .font_size = font_size,
        .handler = handler,
        .bg = BLUE
    };
}

Button CenterY(Button b, int height) {
    Button new = b;
    new.y = height/2 - b.height/2;

    return new;
}

Button CenterX(Button b, int width) {
    Button new = b;
    new.x = width/2 - b.width/2;

    return new;
}

Button RightX(Button b, int width) {
    Button new = b;
    new.x = width - b.width - 10;

    return new;
}

void Invoke(Button b) {
    b.handler();
}

Rectangle Border(Rectangle r, int padding) {
    return (Rectangle) {
        .x = r.x - padding,
        .y = r.y - padding,
        .width = r.width + padding * 2, // times 2 because it's account for padding on both sides
        .height = r.height + padding * 2
    };
}

void Draw(Button b) {
    Rectangle r = (Rectangle) {
        .x = b.x - 3,
        .y = b.y - 2,
        .width = b.width + 7,
        .height = b.height + 3
    };
    DrawRectangleRounded(Border(r, 2), 0.50, 20, BLACK);
    DrawRectangleRounded(r, 0.50, 20, b.bg);
    DrawText(b.text, b.x, b.y, b.font_size, WHITE);
}

void RegenerateGraph() {
    free(nodes);
    nnodes = 0;
    nodes = malloc(MAX_LAYERS * MAX_HEIGHT * MAX_WIDTH * sizeof(GNode *));
    n_layers = randRange(MIN_LAYERS, MAX_LAYERS);

    InitNodes();
}

void BreadthFirstSearch() {
    //https://www.youtube.com/watch?v=vGlvTWUctTQ (timers)
}

Camera3D camera = { 0 };
void ResetView() {
    camera.position = (Vector3) {-61.207848, 241.732605, 456.531769};
    horiz_offset = 3;
}

int main(void) {
    srand(time(NULL));

    InitWindow(WIDTH, HEIGHT, "игра");

    camera.position =   (Vector3) {-61.207848, 241.732605, 456.531769};
    camera.target =     (Vector3) { 0.0f, 0.0f, 0.0f };      // Camera looking at point (0, 0, 0)
    camera.up =         (Vector3) { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy =       45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type    

    n_layers = randRange(MIN_LAYERS, MAX_LAYERS);
    nnodes = 0;
    nodes = malloc(MAX_LAYERS * MAX_HEIGHT * MAX_WIDTH * sizeof(GNode *));
    InitNodes(); 

    int toolbar_height = 80;

    Button buttons[] = {
        CenterY(CenterX(NewButton("(R)egenerate", 0, 10, 20, RegenerateGraph), WIDTH), toolbar_height),
        NewButton("Breadth First Search", 10, 10, 15, BreadthFirstSearch),
        RightX(NewButton("ResetView", 0, 10, 15, ResetView), WIDTH)
    };
    int n_buttons = sizeof(buttons)/sizeof(Button);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {        
        // RainbowRectangles();
        
        if (IsKeyPressed('O') || IsKeyPressedRepeat('O')) {
            camera.position = Zoom(camera.position, 1.10);
            vecprint(camera.position); printf("\n");
        }
        if (IsKeyPressed('I') || IsKeyPressedRepeat('I')) {
            camera.position = Zoom(camera.position, 0.90);
            vecprint(camera.position); printf("\n");
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

        if (IsKeyPressed('R')) {
            RegenerateGraph();
        }
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 pos = GetMousePosition();
            int x = pos.x, y = pos.y;
            for (int i = 0; i < n_buttons; i++) {
                Button b = buttons[i];
                if (x > b.x && x < b.x + b.width && y > b.y && y < b.y + b.height) {
                    Invoke(b); // call event handler on the button just pressed
                }
            }
        }
        
        BeginDrawing();

            ClearBackground(DARKGRAY);

            BeginMode3D(camera);
                // DrawAxes();
                // CubeSpheres();
                // set to all zeros
                
                DrawNodes();

            EndMode3D();


            DrawRectangle(0, 0, WIDTH, toolbar_height, LIGHTGRAY);

            for (int i = 0; i < n_buttons; i++) {
                Draw(buttons[i]);
            }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
// gcc src/main.c $(pkg-config --libs --cflags raylib) -o bin/main && ./bin/main