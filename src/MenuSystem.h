#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include "raylib.h"
#include "CustomCamera.h"

class MenuSystem {
private:
    bool isVisible;
    CustomCamera& camera;
    Rectangle menuRect;
    int samples;
    int maxBounces;
    float gamma;
    float backgroundOpacity;
    float defocusAngle = 0.0f; // Default defocus angle

public:
    MenuSystem(CustomCamera& cameraRef);
    void toggleVisibility();
    void update();
    void draw();
    bool isMenuVisible() const;

    int getSamples() const;
    int getMaxBounces() const;
    float getGamma() const;
    float getBackgroundOpacity() const;
    float getDefocusAngle() const;
};

#endif // MENU_SYSTEM_H
