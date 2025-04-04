#include "raylib.h"
#include "JsonLoader.h"
#include <sstream>
#include <string>

#define MAX_MATERIALS 32
#define MAX_SPHERES 32
#define MAX_QUADS 32

// Helper function to read a file into a string using raylib
std::string readFile(const std::string& filePath) {
    char* fileData = LoadFileText(filePath.c_str());
    if (!fileData) {
        TraceLog(LOG_ERROR, "Failed to open file: %s", filePath.c_str());
        return "";
    }
    std::string fileContent(fileData);
    UnloadFileText(fileData);
    return fileContent;
}

// Function to parse materials from JSON
void loadMaterials(const std::string& filePath, int* materialType, Vector3* materialAlbedo, float* materialFuzz, float* materialRefractionIndex, std::string* materialTextures) {
    std::string json = readFile(filePath);
    size_t start = 0, end = 0;
    int index = 0;

    while ((start = json.find("{", end)) != std::string::npos && index < MAX_MATERIALS) {
        end = json.find("}", start);
        std::string material = json.substr(start, end - start + 1);

        // Parse material type
        size_t typePos = material.find("\"type\":");
        if (typePos != std::string::npos) {
            materialType[index] = std::stoi(material.substr(typePos + 7));
        }

        // Parse albedo
        size_t albedoPos = material.find("\"albedo\":");
        if (albedoPos != std::string::npos) {
            size_t albedoStart = material.find("[", albedoPos);
            size_t albedoEnd = material.find("]", albedoStart);
            std::string albedo = material.substr(albedoStart + 1, albedoEnd - albedoStart - 1);

            std::stringstream albedoStream(albedo);
            albedoStream >> materialAlbedo[index].x;
            albedoStream.ignore(1); // Skip comma
            albedoStream >> materialAlbedo[index].y;
            albedoStream.ignore(1); // Skip comma
            albedoStream >> materialAlbedo[index].z;
        }

        // Parse fuzz
        size_t fuzzPos = material.find("\"fuzz\":");
        if (fuzzPos != std::string::npos) {
            materialFuzz[index] = std::stof(material.substr(fuzzPos + 7));
        }

        // Parse refraction index
        size_t refractionPos = material.find("\"refractionIndex\":");
        if (refractionPos != std::string::npos) {
            materialRefractionIndex[index] = std::stof(material.substr(refractionPos + 18));
        }

        // Parse texture
        size_t texturePos = material.find("\"texture\":");
        if (texturePos != std::string::npos) {
            size_t textureStart = material.find("\"", texturePos + 10) + 1;
            size_t textureEnd = material.find("\"", textureStart);
            if (textureStart != std::string::npos && textureEnd != std::string::npos) {
                std::string textureFile = material.substr(textureStart, textureEnd - textureStart);
                materialTextures[index] = "world/" + textureFile;
            } else {
                materialTextures[index] = "";
            }
        }

        index++;
    }
}

// Function to parse spheres from JSON
void loadSpheres(const std::string& filePath, Vector4* spheres, int* sphereMaterialIndices, int& spheresAmount) {
    std::string json = readFile(filePath);
    size_t start = 0, end = 0;
    spheresAmount = 0;

    while ((start = json.find("{", end)) != std::string::npos && spheresAmount < MAX_SPHERES) {
        end = json.find("}", start);
        std::string sphere = json.substr(start, end - start + 1);

        // Parse position
        size_t positionPos = sphere.find("\"position\":");
        if (positionPos != std::string::npos) {
            size_t positionStart = sphere.find("[", positionPos);
            size_t positionEnd = sphere.find("]", positionStart);
            std::string position = sphere.substr(positionStart + 1, positionEnd - positionStart - 1);

            std::stringstream positionStream(position);
            positionStream >> spheres[spheresAmount].x;
            positionStream.ignore(1); // Skip comma
            positionStream >> spheres[spheresAmount].y;
            positionStream.ignore(1); // Skip comma
            positionStream >> spheres[spheresAmount].z;
        }

        // Parse radius
        size_t radiusPos = sphere.find("\"radius\":");
        if (radiusPos != std::string::npos) {
            spheres[spheresAmount].w = std::stof(sphere.substr(radiusPos + 9));
        }

        // Parse material index
        size_t materialIndexPos = sphere.find("\"materialIndex\":");
        if (materialIndexPos != std::string::npos) {
            sphereMaterialIndices[spheresAmount] = std::stoi(sphere.substr(materialIndexPos + 16));
        } else {
            TraceLog(LOG_WARNING, "Sphere %d does not have a materialIndex field!", spheresAmount);
        }

        spheresAmount++;
    }
}

// Function to parse quads from JSON
void loadQuads(const std::string& filePath, Vector3* quadOrigins, Vector3* quadEdgesU, Vector3* quadEdgesV, int* quadMaterialIndices, int& quadsAmount) {
    std::string json = readFile(filePath);
    size_t start = 0, end = 0;
    quadsAmount = 0;

    while ((start = json.find("{", end)) != std::string::npos && quadsAmount < MAX_QUADS) {
        end = json.find("}", start);
        std::string quad = json.substr(start, end - start + 1);

        // Parse origin
        size_t originPos = quad.find("\"origin\":");
        if (originPos != std::string::npos) {
            size_t originStart = quad.find("[", originPos);
            size_t originEnd = quad.find("]", originStart);
            std::string origin = quad.substr(originStart + 1, originEnd - originStart - 1);

            std::stringstream originStream(origin);
            originStream >> quadOrigins[quadsAmount].x;
            originStream.ignore(1); // Skip comma
            originStream >> quadOrigins[quadsAmount].y;
            originStream.ignore(1); // Skip comma
            originStream >> quadOrigins[quadsAmount].z;
        }

        // Parse edgeU
        size_t edgeUPos = quad.find("\"edgeU\":");
        if (edgeUPos != std::string::npos) {
            size_t edgeUStart = quad.find("[", edgeUPos);
            size_t edgeUEnd = quad.find("]", edgeUStart);
            std::string edgeU = quad.substr(edgeUStart + 1, edgeUEnd - edgeUStart - 1);

            std::stringstream edgeUStream(edgeU);
            edgeUStream >> quadEdgesU[quadsAmount].x;
            edgeUStream.ignore(1); // Skip comma
            edgeUStream >> quadEdgesU[quadsAmount].y;
            edgeUStream.ignore(1); // Skip comma
            edgeUStream >> quadEdgesU[quadsAmount].z;
        }

        // Parse edgeV
        size_t edgeVPos = quad.find("\"edgeV\":");
        if (edgeVPos != std::string::npos) {
            size_t edgeVStart = quad.find("[", edgeVPos);
            size_t edgeVEnd = quad.find("]", edgeVStart);
            std::string edgeV = quad.substr(edgeVStart + 1, edgeVEnd - edgeVStart - 1);

            std::stringstream edgeVStream(edgeV);
            edgeVStream >> quadEdgesV[quadsAmount].x;
            edgeVStream.ignore(1); // Skip comma
            edgeVStream >> quadEdgesV[quadsAmount].y;
            edgeVStream.ignore(1); // Skip comma
            edgeVStream >> quadEdgesV[quadsAmount].z;
        }

        // Parse material index
        size_t materialIndexPos = quad.find("\"materialIndex\":");
        if (materialIndexPos != std::string::npos) {
            quadMaterialIndices[quadsAmount] = std::stoi(quad.substr(materialIndexPos + 16));
        }

        quadsAmount++;
    }
}