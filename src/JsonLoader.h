#ifndef JSONLOADER_H
#define JSONLOADER_H

#include "raylib.h"
#include <string>

// Function declarations
void loadMaterials(const std::string& filePath, int* materialType, Vector3* materialAlbedo, Vector3* materialEmmisiveColor, float* materialFuzz, float* materialRefractionIndex, std::string* materialTextures);
void loadSpheres(const std::string& filePath, Vector4* spheres, int* sphereMaterialIndices, int& spheresAmount);
void loadQuads(const std::string& filePath, Vector3* quadOrigins, Vector3* quadEdgesU, Vector3* quadEdgesV, int* quadMaterialIndices, int& quadsAmount);

#endif // JSONLOADER_H
