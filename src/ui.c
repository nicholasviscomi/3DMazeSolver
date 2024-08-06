#include "lib/ui.h" // NOTE: contains the include for stdio, raylib, etc. and guards multiple include
#include "math.h"
#include <stdio.h>

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