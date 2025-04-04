#ifndef CUSTOM_CAMERA_H
#define CUSTOM_CAMERA_H

#include "raylib.h"
#include "raymath.h"

class CustomCamera {
public:
    Camera3D camera;
    Vector3 viewportU, viewportV, viewportUpperLeft;
    Vector3 pixelU, pixelV, pixel00;
    float aspectRatio;
    float viewportWidth, viewportHeight;
    float moveSpeed = 5.0f;
    float rotationSpeed = 0.1f;

    CustomCamera(int screenWidth, int screenHeight, float fovy);
    void update(int screenWidth, int screenHeight);
    void setShaderValues(Shader& shader, int pixel00Loc, int pixelULoc, int pixelVLoc, int cameraCenterLoc);
    void handleInput(float deltaTime);
};

#endif // CUSTOM_CAMERA_H