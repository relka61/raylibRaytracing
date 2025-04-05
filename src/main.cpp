// Include libraries
#include "raylib.h"
#include "raymath.h"
#include "CustomCamera.h"
#include "MenuSystem.h"
#include <cmath>
#include "RenderHighQualityImage.h"
#include "JsonLoader.h"

// Define maximum number of materials, spheres, and quads
#define MAX_MATERIALS 32
#define MAX_SPHERES 32
#define MAX_QUADS 32

// Entry point
int main(void) {
    // ----------------------
    // --- Initialization ---
    // ----------------------

    // Window Initialization
    InitWindow(1920, 1080, "Pathtracer");
    int monitor = GetCurrentMonitor();
    int screenWidth = GetMonitorWidth(monitor);
    int screenHeight = GetMonitorHeight(monitor);
    SetWindowSize(screenWidth, screenHeight);
    ToggleBorderlessWindowed();
    DisableCursor();
    SetTargetFPS(60);

    BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Loading...", screenWidth / 2 - 50, screenHeight / 2 - 20, 20, WHITE);
    EndDrawing();
    
    // Camera Initialization
    CustomCamera customCamera(screenWidth, screenHeight, 62.3458f);
    
    // Menu System Initialization
    MenuSystem menuSystem(customCamera);
    
    // Shader Initialization
    Shader shader = LoadShader(0, TextFormat("src/raytracing.frag", 330));



    // ----------------------------
    // --- Set shader locations ---
    // ----------------------------
    
    // Camera
    int pixel00Loc = GetShaderLocation(shader, "pixel00");
    int pixelULoc = GetShaderLocation(shader, "pixelU");
    int pixelVLoc = GetShaderLocation(shader, "pixelV");
    int cameraCenterLoc = GetShaderLocation(shader, "cameraCenter");
    int defocusAngleLoc = GetShaderLocation(shader, "defocusAngle");
    
    //Settings
    int samplesLoc = GetShaderLocation(shader, "samples");
    int maxBouncesLoc = GetShaderLocation(shader, "maxBounces");
    int gammaLoc = GetShaderLocation(shader, "gamma");
    int backgroundOpacityLoc = GetShaderLocation(shader, "backgroundOpacity");
    int seedOffsetLoc = GetShaderLocation(shader, "seedOffset");
    
    // Materials
    int materialTypeLoc = GetShaderLocation(shader, "materialType");
    int materialAlbedoLoc = GetShaderLocation(shader, "materialAlbedo");
    int materialEmmisiveColorLoc = GetShaderLocation(shader, "materialEmmisiveColor");
    int materialFuzzLoc = GetShaderLocation(shader, "materialFuzz");
    int materialRefractionIndexLoc = GetShaderLocation(shader, "materialRefractionIndex");
    
    // Spheres
	int spheresAmountLoc = GetShaderLocation(shader, "spheresAmount");
	int spheresLoc = GetShaderLocation(shader, "spheres");
    int spheresMaterialIndiciesLoc = GetShaderLocation(shader, "sphereMaterialIndicies");
    
    // Quad
    int quadsAmountLoc = GetShaderLocation(shader, "quadsAmount");
    int quadOriginsLoc = GetShaderLocation(shader, "quadOrigins");
    int quadEdgesULoc = GetShaderLocation(shader, "quadEdgesU");
    int quadEdgesVLoc = GetShaderLocation(shader, "quadEdgesV");
    int quadMaterialIndiciesLoc = GetShaderLocation(shader, "quadMaterialIndicies");
    


    // ----------------------------
    // --- World Initialization ---
    // ----------------------------
    
    // Setup materials variables
    int materialType[MAX_MATERIALS] = { 0 };
    Vector3 materialAlbedo[MAX_MATERIALS] = { { 0.0f, 0.0f, 0.0f } };
    Vector3 materialEmmisiveColor[MAX_MATERIALS] = { { 0.0f, 0.0f, 0.0f } };
    float materialFuzz[MAX_MATERIALS] = { 0.0f };
    float materialRefractionIndex[MAX_MATERIALS] = { 0.0f };
    std::string materialTexturePaths[MAX_MATERIALS];

    // Setup spheres variables
    int spheresAmount = 0;
    Vector4 spheres[MAX_SPHERES] = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    int sphereMaterialIndicies[MAX_SPHERES] = { 0 };
    
    // Setup quads variables
    int quadsAmount = 0;
    Vector3 quadOrigins[MAX_QUADS] = { { 0.0f, 0.0f, 0.0f } };
    Vector3 quadEdgesU[MAX_QUADS] = { { 0.0f, 0.0f, 0.0f } };
    Vector3 quadEdgesV[MAX_QUADS] = { { 0.0f, 0.0f, 0.0f } };
    int quadMaterialIndicies[MAX_QUADS] = { 0 };
    
    // Load world data from JSON files
    loadMaterials("world/materials.json", materialType, materialAlbedo, materialEmmisiveColor, materialFuzz, materialRefractionIndex, materialTexturePaths);
    loadSpheres("world/spheres.json", spheres, sphereMaterialIndicies, spheresAmount);
    loadQuads("world/quads.json", quadOrigins, quadEdgesU, quadEdgesV, quadMaterialIndicies, quadsAmount);

    // Load textures
    Texture materialTextures[MAX_MATERIALS];
    int materialTexturesLoc = GetShaderLocation(shader, "materialTextures");
    for (int i = 0; i < MAX_MATERIALS; i++) {
        if (!materialTexturePaths[i].empty()) {
            materialTextures[i] = LoadTexture(materialTexturePaths[i].c_str());
        }  
    }

    // Send world data to the shader
    // Materials
    SetShaderValueV(shader, materialTypeLoc, materialType, SHADER_UNIFORM_INT, MAX_MATERIALS);
    SetShaderValueV(shader, materialAlbedoLoc, materialAlbedo, SHADER_UNIFORM_VEC3, MAX_MATERIALS);
    SetShaderValueV(shader, materialEmmisiveColorLoc, materialEmmisiveColor, SHADER_UNIFORM_VEC3, MAX_MATERIALS);
    SetShaderValueV(shader, materialFuzzLoc, materialFuzz, SHADER_UNIFORM_FLOAT, MAX_MATERIALS);
    SetShaderValueV(shader, materialRefractionIndexLoc, materialRefractionIndex, SHADER_UNIFORM_FLOAT, MAX_MATERIALS);
    
    // Spheres
	SetShaderValue(shader, spheresAmountLoc, &spheresAmount, SHADER_UNIFORM_INT);
	SetShaderValueV(shader, spheresLoc, spheres, SHADER_UNIFORM_VEC4, spheresAmount);
    SetShaderValueV(shader, spheresMaterialIndiciesLoc, sphereMaterialIndicies, SHADER_UNIFORM_INT, spheresAmount);
    
    // Quads
    SetShaderValue(shader, quadsAmountLoc, &quadsAmount, SHADER_UNIFORM_INT);
    SetShaderValueV(shader, quadOriginsLoc, quadOrigins, SHADER_UNIFORM_VEC3, quadsAmount);
    SetShaderValueV(shader, quadEdgesULoc, quadEdgesU, SHADER_UNIFORM_VEC3, quadsAmount);
    SetShaderValueV(shader, quadEdgesVLoc, quadEdgesV, SHADER_UNIFORM_VEC3, quadsAmount);
    SetShaderValueV(shader, quadMaterialIndiciesLoc, quadMaterialIndicies, SHADER_UNIFORM_INT, quadsAmount);

    

    // -----------------
    // --- Main Loop ---
    // -----------------
    
    // Stop when the window is closed
    while (!WindowShouldClose()) {
        // Update menu system
        menuSystem.update();
        if (IsKeyPressed(KEY_P)) {
            menuSystem.toggleVisibility();
        }

        // Only when the menu is not visible
        if (!menuSystem.isMenuVisible()) {
            // Update camera movement
            customCamera.handleInput(GetFrameTime());
            // Render a high-quality render
            if (IsKeyPressed(KEY_H)) {
                renderHighQualityImage(shader, seedOffsetLoc, screenWidth, screenHeight, "render.png", menuSystem);
            }
            // Toggle fullscreen
            if (IsKeyPressed(KEY_T)) {
                ToggleBorderlessWindowed();
                SetWindowPosition(0, 0);
            }
        }

        //Update camera and set according shader values
        customCamera.update(screenWidth, screenHeight);
        customCamera.setShaderValues(shader, pixel00Loc, pixelULoc, pixelVLoc, cameraCenterLoc);

        // Update shader values from menuSystem
        int samples = menuSystem.getSamples();
        int maxBounces = menuSystem.getMaxBounces();
        float gamma = menuSystem.getGamma();
        float backgroundOpacity = menuSystem.getBackgroundOpacity();
        float defocusAngle = menuSystem.getDefocusAngle();
        SetShaderValue(shader, samplesLoc, &samples, SHADER_UNIFORM_INT);
        SetShaderValue(shader, maxBouncesLoc, &maxBounces, SHADER_UNIFORM_INT);
        SetShaderValue(shader, gammaLoc, &gamma, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, backgroundOpacityLoc, &backgroundOpacity, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, defocusAngleLoc, &defocusAngle, SHADER_UNIFORM_FLOAT);

        // Drawing
        BeginDrawing();
            // Begin the shader mode
        	BeginShaderMode(shader);
                // Set the texture uniform
                for (int i = 0; i < MAX_MATERIALS; i++) {
                    SetShaderValueTexture(shader, materialTexturesLoc + i, materialTextures[i]);
                }
                // Draw to the screen
        		DrawRectangle(0, 0, screenWidth, screenHeight, PINK); // Fallback color
        	EndShaderMode();
            // Draw the menu
        	menuSystem.draw();
        EndDrawing();
    }

    // De-Initialization
    EnableCursor(); // Reenable the cursor
    UnloadShader(shader); // Unload the shader
    for (int i = 0; i < MAX_MATERIALS; i++) {
        UnloadTexture(materialTextures[i]); // Unload the textures
    }
    CloseWindow(); // Close the window and OpenGL context

    return 0; // Exit the program
}
