#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>


#define ZOOM_MAX 300
#define ZOOM_MIN 10
#define MAX_WIDTH 5
#define MAX_HEIGHT 7

const int WIDTH = 900, HEIGHT = 600;

float theta = 0;
int horiz_offset = 2;


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
float layer_spacing = 30;

Color colors[] = { RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE };
float radius = 1.5;


int y = 0;
int BLOCK_HEIGHT = 10;
void RainbowRectangles() {
    BeginDrawing();
    ClearBackground(GRAY);

    Color colors[] = { RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE };
    int ci = 0;
    for (int i = 0; i < HEIGHT/BLOCK_HEIGHT; i++) {
        Color c = colors[ci++];
        if (ci > 5) ci = 0;

        int new_y = y + (i * BLOCK_HEIGHT);

        // wrap fully off the screen back to the top
        if (new_y >= HEIGHT) {
            DrawRectangle(0, new_y - HEIGHT, WIDTH, BLOCK_HEIGHT, c);

        } else if (new_y + BLOCK_HEIGHT > HEIGHT) {
            // wrap partially off the screen back to the top
            DrawRectangle(0, new_y, WIDTH, HEIGHT - new_y, c); // partial at bottom
            
            int bottom = new_y + BLOCK_HEIGHT;
            DrawRectangle(0, 0, WIDTH, bottom - HEIGHT, c);
        } else {
            // draw main rectangles on the screen
            DrawRectangle(0, new_y, WIDTH, BLOCK_HEIGHT, c);
        }
    }

    y += 1;

    if (y >= HEIGHT) y = 0;

    EndDrawing();
}

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

    n_layers = randRange(7, 15); // + 2 for start and end nodes
    printf("%d\n", n_layers);
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

                // width guaranteed odd --> # of spaces to shift = (width - 1)/2 --> multiply by spacing
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
    Sphere* children;
} GNode;
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
                Vector3Add(spheres[i].pos, offset), 
                Vector3Add(spheres[j].pos, offset), 
                radius/5, radius/5, 20, ColorAlphaBlend(spheres[i].c, spheres[j].c, (Color) {0,0,0,0})
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

float veclen(Vector3 v) {
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

Vector3 vecmul(Vector3 v, float m) {
    return (Vector3) { v.x * m, v.y * m, v.z * m };
}

void vecprint(Vector3 v) {
    printf("[%f, %f, %f]\n", v.x, v.y, v.z);
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


int main(void) {
    srand(time(NULL));

    InitWindow(WIDTH, HEIGHT, "игра");

    Camera3D camera = { 0 };
    float start_zoom = 40;
    camera.position = (Vector3){ 51.945072, 51.634838, 138.785477 };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point (0, 0, 0)
    camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type    

    InitSpheres();
    int li = 0;
    int counter = 0;
    // for (int i = 0; i < n_spheres - 1; i++) { // don't print first or last spheres
    //     vecprint(spheres[i].pos);
    //     counter += 1;
    //     if (counter >= layer_sizes[li]) {
    //         li += 1;
    //         counter = 0;
    //     }
    // }
    printf("%d\n", n_layers);
    for (int i = 0; i < n_layers; i++) {
        int size = layer_sizes[i];
        printf("%d, ", size);
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {        
        // RainbowRectangles();
        
        if (IsKeyPressed('O') || IsKeyPressedRepeat('O')) {
            camera.position = Zoom(camera.position, 1.10);
        }
        if (IsKeyPressed('I') || IsKeyPressedRepeat('I')) {
            camera.position = Zoom(camera.position, 0.90);
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

            ClearBackground(LIGHTGRAY);

            BeginMode3D(camera);
                // DrawAxes();
                // CubeSpheres();
                
                DrawSpheres();
                ConnectSpheres();

            EndMode3D();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
// gcc src/main.c $(pkg-config --libs --cflags raylib) -o bin/main && ./bin/main