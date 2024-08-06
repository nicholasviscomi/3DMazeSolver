#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "struct.h"

#define ZOOM_MAX 600
#define ZOOM_MIN 10
#define WIDTH 1100
#define HEIGHT 600

float veclen(Vector3 v);

Vector3 vecmul(Vector3 v, float m);

// prints vector only
void vecprint(Vector3 v);

// zoom the camera position in/out by a factor
// multiply the x, y, z components by the factor
// E.g. factor of 2 will double the distance from the camera target (aka zoom out by 100%)
Vector3 Zoom(Vector3 v, float factor);


// update x and z vectors for a theta radian rotation around the y axis
Vector3 RotateY(Vector3 pos, float theta);

Button NewButton(char* text, int x, int y, int font_size, void (*handler)());

Button CenterY(Button b, int height);

Button CenterX(Button b, int width);

Button RightX(Button b, int width);

void Invoke(Button b);

Rectangle Border(Rectangle r, int padding);

#endif