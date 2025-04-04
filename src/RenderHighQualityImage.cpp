#include "RenderHighQualityImage.h"
#include "raylib.h"

void renderHighQualityImage(Shader shader, int seedOffsetLoc, int screenWidth, int screenHeight, const char* outputFileName, MenuSystem& menuSystem) {
    RenderTexture2D renderTexture = LoadRenderTexture(screenWidth, screenHeight);
    Image accumulatedImage = GenImageColor(screenWidth, screenHeight, BLACK);

    int originalSamples = menuSystem.getSamples();
    int totalSamples = 1000;
    int passSampleCount = 500;
    int numPasses = round(totalSamples / passSampleCount);
    SetShaderValue(shader, GetShaderLocation(shader, "samples"), &passSampleCount, SHADER_UNIFORM_INT);


    for (int pass = 0; pass < numPasses; pass++) {
        Vector2 seedOffset = { (float)GetRandomValue(1, 1000), (float)GetRandomValue(-1000, 0)};
        SetShaderValue(shader, seedOffsetLoc, &seedOffset, SHADER_UNIFORM_VEC2);

        BeginTextureMode(renderTexture);
            BeginShaderMode(shader);
                DrawRectangle(0, 0, screenWidth, screenHeight, PINK);
            EndShaderMode();
        EndTextureMode();

        Image passImage = LoadImageFromTexture(renderTexture.texture);
        ImageFlipVertical(&passImage);
        ImageDraw(&accumulatedImage, passImage, Rectangle{0, 0, (float)screenWidth, (float)screenHeight}, Rectangle{0, 0, (float)screenWidth, (float)screenHeight}, WHITE);
        UnloadImage(passImage);

        // Draw the accumulated image as a texture
        Texture2D accumulatedTexture = LoadTextureFromImage(accumulatedImage);
        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexture(accumulatedTexture, 0, 0, WHITE);

            // Draw progress bar as an overlay
            DrawText("Rendering High-Quality Image...", screenWidth / 2 - 150, screenHeight / 2 - 60, 20, WHITE);
            DrawRectangle(screenWidth * 0.25, screenHeight * 0.5, screenWidth * 0.5, screenHeight * 0.04, GRAY);
            DrawRectangle(screenWidth * 0.25, screenHeight * 0.5, (int)(screenWidth * 0.5 * (float)(pass + 1) / numPasses), screenHeight * 0.04, GREEN);
            DrawRectangleLines(screenWidth * 0.25, screenHeight * 0.5, screenWidth * 0.5, screenHeight * 0.04, WHITE);
        EndDrawing();
        UnloadTexture(accumulatedTexture);
    }

    // Export the accumulated image to a file
    ExportImage(accumulatedImage, outputFileName);

    // Clean up
    UnloadImage(accumulatedImage);
    UnloadRenderTexture(renderTexture);

    // Reset the sample count to the original value
    int defaultSampleCount = originalSamples;
    SetShaderValue(shader, GetShaderLocation(shader, "samples"), &defaultSampleCount, SHADER_UNIFORM_INT);
    Vector2 seedOffset = { 0.0f, 0.0f};
    SetShaderValue(shader, seedOffsetLoc, &seedOffset, SHADER_UNIFORM_VEC2);

    TraceLog(LOG_INFO, "High-quality image saved to %s", outputFileName);
}