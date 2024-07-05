#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>


#define ZOOM_MAX 300
#define ZOOM_MIN 10
#define MAX_WIDTH 3
#define MAX_HEIGHT 3

#define MAX_LAYERS 4
#define MIN_LAYERS 2

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

void InitSpheres() {
    spheres[0] = (Sphere) {
        (Vector3) {-1 * layer_spacing, 0, 0},
        WHITE, radius + 2
    };
    n_spheres = 1;

    int ci = 0;

    n_layers = randRange(MIN_LAYERS, MAX_LAYERS); // + 2 for start and end nodes
    layer_sizes = malloc((n_layers + 2) * sizeof(int));
    layer_sizes[0] = 1; // 1 start node

    for (size_t x = 0; x < n_layers; x++) {
        Color c = colors[ci++];
        if (ci > 5) ci = 0;

        // * both need to initialized outside of the height loop 
        int width = randRange(2, MAX_WIDTH); // printf("%d\t", width);
        int height = randRange(1, MAX_HEIGHT); 
        Sphere *layer = malloc(width * height * sizeof(Sphere));
        
        for (size_t z = 0; z < width; z++) {
            for (size_t y = 0; y < height; y++) {

                float w_offset = ((width - 1) * spacing) / 2; 
                float h_offset = ((height - 1) * spacing) / 2;

                spheres[n_spheres] = (Sphere) {
                    (Vector3) { x * layer_spacing, y * spacing - h_offset, z * spacing - w_offset},
                    c, radius
                };
                
                // printf("(%f, %f, %f)\t", spheres[i].x, spheres[i].y, spheres[i].z);
                n_spheres += 1;

            }
        }
        layer_sizes[x + 1] = width * height;  // x + 1 to offset for start node
    }

    // add end node to layer_sizes and spheres
    layer_sizes[n_layers + 1] = 1; // add it to the very end, which is one plus the number of internal layers

    Sphere last = spheres[n_spheres - 1];
    spheres[n_spheres++] = (Sphere) {
        (Vector3) {last.pos.x + layer_spacing * 1, 0, 0},
        BLACK, radius + 2
    };

    n_layers += 2; // update size of layer_sizes
}

void DrawSpheres() {
    int ci = 0; 

    for (int i = 0; i < n_spheres; i++) {
        Color c;
        if (i == 0 || i == n_spheres - 1) {
            c = GRAY;
        } else {
            c = colors[ci++];
        }

        Sphere s = spheres[i];
        s.pos.x = s.pos.x - horiz_offset*layer_spacing;

        DrawSphere(s.pos, s.radius, s.c);
        DrawSphere(s.pos, s.radius + 0.1, ColorAlpha(WHITE, 0.4));

        if (ci > 5) {
            ci = 0;
        }
    }
}


typedef struct {
    Sphere node;
    struct GNode* children; 
    int n_children;
} GNode;
GNode root;

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


void InitNodes() {
    root = (GNode) {
        (Sphere) {
            (Vector3) {-1 * layer_spacing, 0, 0},
            WHITE, radius + 2
        },
        malloc(MAX_WIDTH * MAX_HEIGHT * sizeof(GNode)),
        0
    };

    GNode** curr_layer = malloc(sizeof(GNode*));
    curr_layer[0] = &root;
    int curr_size = 1;

    for (int i = 0; i < n_layers; i++) {
        // make layer
        int width = randRange(2, MAX_WIDTH);
        int height = randRange(1, MAX_HEIGHT); 
        GNode** next_layer = malloc(sizeof(GNode*) * width * height);

        float w_offset = ((width - 1) * spacing) / 2; 
        float h_offset = ((height - 1) * spacing) / 2;
        for (size_t z = 0; z < width; z++) {
            for (size_t y = 0; y < height; y++) {
                
                next_layer[z * width + y] = &(GNode) {
                    (Sphere) {
                        (Vector3) { i * layer_spacing, y * spacing - h_offset, z * spacing - w_offset},
                        BLACK, radius
                    },
                    NULL,
                    0
                };

            }
        }
        //----------------


        for (int j = 0; j < curr_size; j++) {
            GNode* curr_node = curr_layer[j];
            (*curr_node).children = malloc(sizeof(GNode*) * width * height);

            for (int k = 0; k < width * height; k++) {
                // (*curr_node).children[k] = 
                next_layer[k];
            }

        }

        curr_layer = next_layer;
        curr_size = width * height;
    }

}

void ConnectSpheres() {
    //* Connect middle layers    
    int li = 0;
    int counter = 0;
    int size_next = 0;
    int start_current_layer = 0; // index of the first sphere in the current layer
    Vector3 offset = { -layer_spacing * horiz_offset, 0, 0};
    for (int i = 0; i < n_spheres - 1; i++) { 
        // for every ball in the current layer, connect it to every ball in the next layer
        // Next layer starts at start_current_layer + size_current_layer (layer_sizes[li])
        
        size_next = layer_sizes[li + 1];
        int start_next = start_current_layer + layer_sizes[li];


        for (int j = start_next; j < start_next + size_next; j++) {
            DrawCylinderEx(
                vadd(spheres[i].pos, offset), 
                vadd(spheres[j].pos, offset), 
                radius/5, radius/5, 20, 
                ColorAlphaBlend(spheres[j].c, spheres[j].c, (Color) {0,0,0,50})
            );
        }

        counter += 1;
        if (counter >= layer_sizes[li]) {
            li += 1;
            counter = 0;
            start_current_layer = i + 1;
        }
    }
}

void CubeSpheres() {
    // ! in this configuration, the y axis goes up/down while x and z come out like the ground  
    Color colors[] = { RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE };
    int ci = 0; 

    int spacing = 4;
    float y_spacing = 2.5;

    int n_row = 3;
    int n_col = 3;
    int n_layers = 3;
    int radius = 1;

    float row_offset = ((n_row-1)/2) * spacing;
    float col_offset = ((n_col-1)/2) * spacing;
    for (float y = 0; y < n_layers; y++) {
        for (float x = 0; x < n_row; x++) {
            for (float z = 0; z < n_col; z++) {
                DrawSphere(
                    (Vector3) {
                        x * spacing - row_offset, 
                        y * y_spacing, 
                        z * spacing - col_offset
                    }, 
                    radius, colors[ci++]
                );

                DrawSphereWires(
                    (Vector3) {
                        x * spacing - row_offset, 
                        y * y_spacing, 
                        z * spacing - col_offset
                    }, 
                    radius, 20, 20,
                    BLACK
                );

                if (ci > 5) {
                    ci = 0;
                }
            }
        }
    }

}


int main(void) {
    srand(time(NULL));

    // InitWindow(WIDTH, HEIGHT, "игра");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 61.945072, 61.634838, 148.785477 };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point (0, 0, 0)
    camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type    

    InitSpheres();
    int li = 0;
    int counter = 0;
    printf("%d\n", n_layers);
    for (int i = 0; i < n_layers; i++) {
        int size = layer_sizes[i];
        printf("%d, ", size);
    }

    InitNodes(); 
    printf("\nn_children: %d\n", root.n_children);
    GNode* children = root.children;
    for (int i = 0; i < root.n_children; i++) {
        printf("Child %d: ", i);
        vecprint(children[i].node.pos);
        printf("\n");
        printf("n grand children: %d\n", children[i].n_children);
    }

    // SetTargetFPS(60);

    // while (!WindowShouldClose()) {        
    //     // RainbowRectangles();
        
    //     if (IsKeyPressed('O') || IsKeyPressedRepeat('O')) {
    //         camera.position = Zoom(camera.position, 1.10);
    //         vecprint(camera.position);
    //     }
    //     if (IsKeyPressed('I') || IsKeyPressedRepeat('I')) {
    //         camera.position = Zoom(camera.position, 0.90);
    //         vecprint(camera.position);
    //     }
    //     if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
    //         Vector3 new = (Vector3) { camera.position.x, camera.position.y + 10, camera.position.z };
            
    //         camera.position = vecmul(new, veclen(camera.position)/veclen(new));
    //     }
    //     if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) {
    //         Vector3 new = (Vector3) { camera.position.x, camera.position.y - 10, camera.position.z };

    //         camera.position = vecmul(new, veclen(camera.position)/veclen(new));
    //     }

    //     // Change horizontal offset which controls how many spaces the balls get shifted
    //     // along the x-axis. Create the same effect as moving the camera by moving the objects
    //     // because idk how to shift the camera tbh
    //     if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
    //         horiz_offset -= 1;
    //     }
    //     if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
    //         horiz_offset += 1;
    //     }


    //     float m = GetMouseWheelMove();
    //     if (m > 0) {
    //         Vector3 new = RotateY(camera.position, (-2 * PI / 100) * m);
    //         camera.position = new;
    //         // vecprint(camera.position);
    //     } else if (m < 0) {
    //         // ! m < 0, so use its abs to scaled the rotation according to scroll velocity
    //         Vector3 new = RotateY(camera.position,  (2 * PI / 100) * fabsf(m));
    //         camera.position = new;
    //         // vecprint(camera.position);
    //     }  

    //     BeginDrawing();

    //         ClearBackground(DARKGRAY);

    //         BeginMode3D(camera);
    //             // DrawAxes();
    //             // CubeSpheres();
                
    //             DrawSpheres();
    //             ConnectSpheres();

    //         EndMode3D();

    //     EndDrawing();
    // }

    // CloseWindow();

    return 0;
}
// gcc src/main.c $(pkg-config --libs --cflags raylib) -o bin/main && ./bin/main