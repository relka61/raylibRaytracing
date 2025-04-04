#include "MenuSystem.h"

MenuSystem::MenuSystem(CustomCamera& cameraRef) 
    : isVisible(false), camera(cameraRef), samples(10), maxBounces(3), gamma(1.6f), backgroundOpacity(1.0f) {
    menuRect = { 50, 50, 450, 500 };
}

void MenuSystem::toggleVisibility() {
    isVisible = !isVisible;
}

void MenuSystem::update() {
    if (!isVisible) return;

    // Adjust FOV using arrow keys
    if (IsKeyDown(KEY_DOWN)) camera.camera.fovy = fmin(camera.camera.fovy + 1.0f, 160.0f);
    if (IsKeyDown(KEY_UP)) camera.camera.fovy = fmax(camera.camera.fovy - 1.0f, 10.0f);

    // Adjust samples using LEFT/RIGHT keys
    if (IsKeyDown(KEY_LEFT)) samples = fmax(samples - 1, 1);
    if (IsKeyDown(KEY_RIGHT)) samples++;

    // Adjust maxBounces using Z/X keys
    if (IsKeyPressed(KEY_Z)) maxBounces = fmax(maxBounces - 1, 0);
    if (IsKeyPressed(KEY_X)) maxBounces++;

    // Adjust gamma using G/H keys
    if (IsKeyPressed(KEY_G)) gamma = fmax(gamma - 0.1f, -5.0f);
    if (IsKeyPressed(KEY_H)) gamma += 0.1f;

    // Adjust background opacity using B/N keys
    if (IsKeyPressed(KEY_B)) backgroundOpacity = fmax(backgroundOpacity - 0.1f, 0.0f);
    if (IsKeyPressed(KEY_N)) backgroundOpacity = fmin(backgroundOpacity + 0.1f, 1.0f);

    // Adjust defocusAngle using K/L keys
    if (IsKeyPressed(KEY_K)) defocusAngle = fmax(defocusAngle - 0.001f, 0.0f);
    if (IsKeyPressed(KEY_L)) defocusAngle = fmin(defocusAngle + 0.001f, 10.0f);
}

void MenuSystem::draw() {
    if (!isVisible) return;

    DrawRectangleRec(menuRect, GRAY); // Menu background
    DrawText("Menu", menuRect.x + 10, menuRect.y + 10, 20, BLACK);

    int lineSpacing = 30; // Spacing between each line
    int baseY = menuRect.y + 50; // Starting Y position for the text

    DrawText(TextFormat("FOV: %.1f", camera.camera.fovy), menuRect.x + 10, baseY, 20, BLACK);
    DrawText(TextFormat("Samples: %d", samples), menuRect.x + 10, baseY + lineSpacing, 20, BLACK);
    DrawText(TextFormat("Max Bounces: %d", maxBounces), menuRect.x + 10, baseY + 2 * lineSpacing, 20, BLACK);
    DrawText(TextFormat("Gamma: %.1f", gamma), menuRect.x + 10, baseY + 3 * lineSpacing, 20, BLACK);
    DrawText(TextFormat("Background Opacity: %.1f", backgroundOpacity), menuRect.x + 10, baseY + 4 * lineSpacing, 20, BLACK);
    DrawText(TextFormat("Defocus Angle: %.2f", defocusAngle), menuRect.x + 10, baseY + 5 * lineSpacing, 20, BLACK);

    int instructionsBaseY = baseY + 6 * lineSpacing + 10; // Add extra spacing before instructions
    DrawText("Use UP/DOWN to adjust FOV", menuRect.x + 10, instructionsBaseY, 20, DARKGRAY);
    DrawText("Use LEFT/RIGHT to adjust Samples", menuRect.x + 10, instructionsBaseY + lineSpacing, 20, DARKGRAY);
    DrawText("Use Z/X to adjust Max Bounces", menuRect.x + 10, instructionsBaseY + 2 * lineSpacing, 20, DARKGRAY);
    DrawText("Use G/H to adjust Gamma", menuRect.x + 10, instructionsBaseY + 3 * lineSpacing, 20, DARKGRAY);
    DrawText("Use B/N to adjust Background Opacity", menuRect.x + 10, instructionsBaseY + 4 * lineSpacing, 20, DARKGRAY);
    DrawText("Use K/L to adjust Defocus Angle", menuRect.x + 10, instructionsBaseY + 5 * lineSpacing, 20, DARKGRAY);
    DrawText("Press P to close menu", menuRect.x + 10, instructionsBaseY + 6 * lineSpacing, 20, DARKGRAY);
}

bool MenuSystem::isMenuVisible() const {
    return isVisible;
}

int MenuSystem::getSamples() const {
    return samples;
}

int MenuSystem::getMaxBounces() const {
    return maxBounces;
}

float MenuSystem::getGamma() const {
    return gamma;
}

float MenuSystem::getBackgroundOpacity() const {
    return backgroundOpacity;
}

float MenuSystem::getDefocusAngle() const {
    return defocusAngle;
}