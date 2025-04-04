#ifndef RENDER_HIGH_QUALITY_IMAGE_H
#define RENDER_HIGH_QUALITY_IMAGE_H

#include "raylib.h"
#include "MenuSystem.h"

// Function declaration
void renderHighQualityImage(Shader shader, int seedOffsetLoc, int screenWidth, int screenHeight, const char* outputFileName, MenuSystem& menuSystem);

#endif // RENDER_HIGH_QUALITY_IMAGE_H