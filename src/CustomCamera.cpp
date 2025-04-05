#include "CustomCamera.h"
#include <cmath>

CustomCamera::CustomCamera(int screenWidth, int screenHeight, float fovy) {
    camera.position = { 0.0f, 0.0f, 1.0f }; // Start slightly away from the origin
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = fovy;
    camera.projection = CAMERA_PERSPECTIVE;

    aspectRatio = (float)screenWidth / screenHeight;
    update(screenWidth, screenHeight);
}

void CustomCamera::update(int screenWidth, int screenHeight) {
    Vector3 w = Vector3Normalize(Vector3Subtract(camera.position, camera.target));
    Vector3 u = Vector3Normalize(Vector3CrossProduct(camera.up, w));
    Vector3 v = Vector3CrossProduct(w, u);

    float theta = camera.fovy * PI / 180.0f;
    float h = tanf(theta / 2.0f);
    viewportHeight = 2.0f * h;
    viewportWidth = viewportHeight * aspectRatio;

    viewportU = Vector3Scale(u, viewportWidth);
    viewportV = Vector3Scale(v, viewportHeight);
    viewportUpperLeft = Vector3Subtract(camera.position, Vector3Scale(w, 1.0f));
    viewportUpperLeft = Vector3Subtract(viewportUpperLeft, Vector3Scale(viewportU, 0.5f));
    viewportUpperLeft = Vector3Subtract(viewportUpperLeft, Vector3Scale(viewportV, 0.5f));

    pixelU = Vector3Scale(viewportU, 1.0f / screenWidth);
    pixelV = Vector3Scale(viewportV, 1.0f / screenHeight);
    pixel00 = viewportUpperLeft;
}

void CustomCamera::setShaderValues(Shader& shader, int pixel00Loc, int pixelULoc, int pixelVLoc, int cameraCenterLoc) {
    SetShaderValue(shader, pixel00Loc, &pixel00, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, pixelULoc, &pixelU, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, pixelVLoc, &pixelV, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, cameraCenterLoc, &camera.position, SHADER_UNIFORM_VEC3);
}

void CustomCamera::handleInput(float deltaTime) {
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

    // Movement controls
    if (IsKeyDown(KEY_W)) {
        Vector3 offset = Vector3Scale(forward, moveSpeed * deltaTime);
        camera.position = Vector3Add(camera.position, offset);
        camera.target = Vector3Add(camera.target, offset);
    }
    if (IsKeyDown(KEY_S)) {
        Vector3 offset = Vector3Scale(forward, moveSpeed * deltaTime);
        camera.position = Vector3Subtract(camera.position, offset);
        camera.target = Vector3Subtract(camera.target, offset);
    }
    if (IsKeyDown(KEY_A)) {
        Vector3 offset = Vector3Scale(right, moveSpeed * deltaTime);
        camera.position = Vector3Subtract(camera.position, offset);
        camera.target = Vector3Subtract(camera.target, offset);
    }
    if (IsKeyDown(KEY_D)) {
        Vector3 offset = Vector3Scale(right, moveSpeed * deltaTime);
        camera.position = Vector3Add(camera.position, offset);
        camera.target = Vector3Add(camera.target, offset);
    }
    if (IsKeyDown(KEY_E)) {
        camera.position.y += moveSpeed * deltaTime;
        camera.target.y += moveSpeed * deltaTime;
    }
    if (IsKeyDown(KEY_Q)) {
        camera.position.y -= moveSpeed * deltaTime;
        camera.target.y -= moveSpeed * deltaTime;
    }

    // Mouse rotation
    Vector2 mouseDelta = GetMouseDelta();
    if (mouseDelta.x != 0 || mouseDelta.y != 0) {
        float yaw = -mouseDelta.x * rotationSpeed * deltaTime;
        float pitch = -mouseDelta.y * rotationSpeed * deltaTime;

        Matrix yawRotation = MatrixRotateY(yaw);
        Matrix pitchRotation = MatrixRotate(right, pitch);

        Vector3 direction = Vector3Subtract(camera.target, camera.position);
        direction = Vector3Transform(direction, yawRotation);
        direction = Vector3Transform(direction, pitchRotation);
        
        camera.target = Vector3Add(camera.position, direction);
    }
}
