// Uses OpenGL version 330 core
#version 330 core

// Fragment shader output color
out vec4 finalColor;

// Camera uniforms
uniform vec3 pixel00;
uniform vec3 pixelU;
uniform vec3 pixelV;
uniform vec3 cameraCenter;

// Settings uniforms
uniform int samples;
uniform int maxBounces;
uniform int seedOffset;
uniform float backgroundOpacity;
uniform float gamma;
uniform float defocusAngle;

// Material uniforms
#define MAX_MATERIALS 32
uniform int materialType[MAX_MATERIALS];
uniform vec3 materialAlbedo[MAX_MATERIALS];
uniform vec3 materialEmmisiveColor[MAX_MATERIALS];
uniform float materialFuzz[MAX_MATERIALS];
uniform float materialRefractionIndex[MAX_MATERIALS];
uniform sampler2D materialTextures[MAX_MATERIALS];

// Sphere uniforms
#define MAX_SPHERES 32
uniform int spheresAmount;
uniform vec4 spheres[MAX_SPHERES];
uniform int sphereMaterialIndicies[MAX_SPHERES];

// Quad uniforms
#define MAX_QUADS 32
uniform int quadsAmount;
uniform vec3 quadOrigins[MAX_QUADS];
uniform vec3 quadEdgesU[MAX_QUADS];
uniform vec3 quadEdgesV[MAX_QUADS];
uniform int quadMaterialIndicies[MAX_QUADS];

// Constants
const float infinity = pow(2.0, 31.0);
const float pi = 3.14159265359;
const float smallValue = pow(2.0, -12.0);

// Sun settings
const vec3 sunDirection = vec3(1.0, 0.6, 0.5);
const vec3 sunColor = vec3(10.0, 10.0, 8.0);
const float sunSize = 0.02;


// Ray structure
struct Ray {
    vec3 origin; // origin of the ray in world space
    vec3 direction; // direction of the ray in world space
};

// HitRecord structure
struct HitRecord {
    float t; // distance to the hit point
    vec3 point; // hit point in world space
    vec3 normal; // normal at the hit point
    bool frontFace; // true if the ray hit the front face of the object
    bool hit; // true if the ray hit something
    vec3 color; // current color of the ray
    vec3 emmisiveColor; // emmisive color of the hit object
    int materialIndex; // index of the material of the hit object
    vec2 uv; // texture coordinates of the hit point
};






// ------------------------
// --- Random Functions ---
// ------------------------


// Hash functtions from https://www.shadertoy.com/view/4djSRW
float hash11(float p) {
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float hash13(vec3 p3) {
	p3  = fract(p3 * .1031);
    p3 += dot(p3, p3.zyx + 31.32);
    return fract((p3.x + p3.y) * p3.z);
}

// Generate a random vector in 3D space
vec3 randomVector(float seed) {
    float z = hash11(seed) * 2.0 - 1.0;
    float t = hash11(seed + 1.0) * 2.0 * pi;
    float r = sqrt(1.0 - z * z);
    return vec3(r * cos(t), r * sin(t), z);
}

// Generate a random point in the circle perpendicular to the viewing direction of the camera
vec3 randomInCircle(float seed) {
    float r = sqrt(hash11(seed));
    float t = 2.0 * pi * hash11(seed + 1.61803398875);
    vec3 disk = pixelU * r * cos(t) + pixelV * r * sin(t);
    return (length(disk) > smallValue && length(disk) < infinity) ? normalize(disk) : vec3(0.0);
}

// Generate a random point in a circle with a given radius
vec3 defocusDiskSample(vec3 center, float defocusAngle, float seed) {
    return center + randomInCircle(seed) * defocusAngle;
}

// Generate a random point in a square from [-0.5, 0.5]
vec3 randomInSquare(float seed) {
    vec3 pU = pixelU * (hash11(seed) - 0.5);
    vec3 pV = pixelV * (hash11(seed + 1.61803398875) - 0.5);
    return pU + pV;
}

// Generate a random point in a square from [-0.5, 0.5] with stratified sampling
vec3 randomInSquareStratified(float seed, int si, int sj, float recipSqrtSamples) {
    vec3 pU = pixelU * (recipSqrtSamples * si * hash11(seed) - 0.5);
    vec3 pV = pixelV * (recipSqrtSamples * sj * hash11(seed + 1.61803398875) - 0.5);
    return pU + pV;
}






// ---------------------------
// --- Collision Detection ---
// ---------------------------

bool hitQuad(float alpha, float beta) {
    return alpha >= 0.0 && alpha <= 1.0 && beta >= 0.0 && beta <= 1.0;
}

bool hitTriangle(float alpha, float beta) {
    return alpha >= 0.0 && alpha <= 1.0 && beta >= 0.0 && (alpha + beta) <= 1.0;
}

bool hitDisk(float alpha, float beta) {
    return sqrt(alpha*alpha + beta*beta) < 0.5;
}

// Ray Quadrilateral intersection algorithm
void hit2DPrimitive(Ray ray, inout HitRecord record, float tmin, float tmax, int quadIndex) {
    // Initial calculations
    vec3 n = cross(quadEdgesU[quadIndex], quadEdgesV[quadIndex]);
    vec3 normal = normalize(n);
    float D = dot(normal, quadOrigins[quadIndex]);
    vec3 w = n / dot(n, n);

    // Determine if the ray is parallel to the plane of the quad
    float denominator = dot(normal, ray.direction);
    if (abs(denominator) < smallValue) {
        // Return if the ray is parallel to the plane of the quad
        return;
    }
    // Calculate the distance from the ray origin to the plane of the quad
    float t = (D - dot(normal, ray.origin)) / denominator;
    // Return if this is outside the range of tmin to tmax
    if (t <= tmin || t >= tmax) {
        return;
    }

    // Calculate the intersection point of the ray and the plane of the quad
    vec3 intersection = ray.origin + t * ray.direction;
    vec3 planarHitPoint = intersection - quadOrigins[quadIndex];
    // Alpha and beta are values that determine the position of the intersection point in the plane of the quad
    float alpha = dot(w, cross(planarHitPoint, quadEdgesV[quadIndex]));
    float beta = dot(w, cross(quadEdgesU[quadIndex], planarHitPoint));

    // Where alpha and beta belong to [0, 1]: the intersection point is inside the quad
    if (hitQuad(alpha, beta)) {
        record.hit = true;
        record.t = t;
        record.point = intersection;
        record.normal = normal;
        record.materialIndex = quadMaterialIndicies[quadIndex];
        record.frontFace = true;
        record.uv = vec2(beta, 1.0 - alpha);
    }
}

// Ray Sphere intersection algorithm
void hitSphere(Ray ray, inout HitRecord record, float tmin, float tmax, int sphereIndex) {
    // First 3 components are the center of the sphere
    // Last component is the radius of the sphere

    // Calculate initial variables
    // Simplified quadratic formula application
    vec3 oc = spheres[sphereIndex].xyz - ray.origin;
    float a = dot(ray.direction, ray.direction);
    float halfb = dot(ray.direction, oc);
    float c = dot(oc, oc) - spheres[sphereIndex].w * spheres[sphereIndex].w;
    float discriminant = halfb * halfb - a * c;

    // The ray only intersects the sphere if the discriminant of the quadratic is greater than 0
    if (discriminant > 0.0) {
        // Find the nearest root
        float sqrtd = sqrt(discriminant);
        float root = (halfb - sqrtd) / a;
        if (root <= tmin || root >= tmax) {
            root = (halfb + sqrtd) / a;
            if (root <= tmin || root >= tmax) {
                // If the root is outside the range of tmin to tmax, return
                return;
            }
        }

        // Successful ray-sphere intersection
        // Update the parameters of the HitRecord accordingly
        record.hit = true;
        record.t = root;
        record.point = ray.origin + root * ray.direction;
        record.normal = (record.point - spheres[sphereIndex].xyz) / spheres[sphereIndex].w;
        record.materialIndex = sphereMaterialIndicies[sphereIndex];
        record.frontFace = dot(ray.direction, record.normal) < 0.0;
        if (!record.frontFace) {
            record.normal = -record.normal; // Flip the normal if the ray is inside the sphere
        }
        record.uv = vec2(0.5 + atan(record.normal.z, record.normal.x) / (2.0 * pi), 0.5 - asin(record.normal.y) / pi);
    } 
}






// -----------------
// --- Materials ---
// -----------------

void updateColor(inout HitRecord record) {
    // Get the texure specified by the material index
    vec3 textureColor = texture(materialTextures[record.materialIndex], record.uv).rgb;
    // Only apply the texture if it isn't small (to only apply when a texture is present)
    if (textureColor != materialAlbedo[record.materialIndex] && length(textureColor) > smallValue) {
        record.color *= textureColor;
    } else {
        record.color *= materialAlbedo[record.materialIndex];
    }

    record.emmisiveColor += materialEmmisiveColor[record.materialIndex];
}

// Determine background color based on the ray direction
vec3 background(vec3 direction) {
    // Determine sky color based on the ray direction
    vec3 unitDirection = normalize(direction);
    float t = 0.5 * (unitDirection.y + 1.0);
    vec3 baseColor = vec3(1.0 - t) + t * vec3(0.2, 0.4, 0.9);

    // Add sun effect
    float angle = acos(dot(unitDirection, normalize(sunDirection)));
    float sunEffect = exp(-pow(angle / sunSize, 0.8));
    vec3 finalColor = mix(baseColor, sunColor, sunEffect);

    return finalColor * backgroundOpacity;
}

// Schlick's approximation for reflectance
float reflectance(float cosine, float refractionIndex) {
    float r0 = (1.0 - refractionIndex) / (1.0 + refractionIndex);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow((1.0 - cosine), 5.0);
}

void lambertian(inout Ray ray, inout HitRecord record, float seed) {
    // Lambertian reflection using cosine-weighted sampling
    ray.direction = record.normal + randomVector(seed);
    // If normal and randomVector are opposite, the vector will be zero and will result in weird behavior
    if (length(ray.direction) < smallValue) {
        ray.direction = record.normal;
    }
}

void metal(inout Ray ray, inout HitRecord record, float seed) {
    // Reflect the ray direction around the normal and add fuzz
    ray.direction = reflect(ray.direction, record.normal) + materialFuzz[record.materialIndex] * randomVector(seed);
}

void dialetric(inout Ray ray, inout HitRecord record, float seed) {
    // Ri is the ratio between the refractive index of the material and the refractive index of the medium the ray is coming from
    float ri = record.frontFace ? (1.0 / materialRefractionIndex[record.materialIndex]) : materialRefractionIndex[record.materialIndex];
    vec3 unitDirection = normalize(ray.direction); // Normalized ray direction

    // Calculate variables
    float cosTheta = min(dot(-unitDirection, record.normal), 1.0);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    bool cannotRefract = ri * sinTheta > 1.0;

    // If the ray cannot refract, reflect it
    if (cannotRefract || reflectance(cosTheta, ri) > hash11(seed)) {
        metal(ray, record, seed);
        return;
    }
    // Otherwise refract the ray as normal
    ray.direction = refract(unitDirection, record.normal, ri);
}

void rayHit(inout Ray ray, inout HitRecord record, float seed) {
    // Update the ray origin to the hit point and update the color
    ray.origin += record.t * ray.direction;
    updateColor(record);

    // Check the type of the material and update the Ray and HitRecord accordingly
    if (materialType[record.materialIndex] == 0) { // Lambertian
        lambertian(ray, record, seed);
        return;
    }
    if (materialType[record.materialIndex] == 1) { // Metal
        metal(ray, record, seed);
        return;
    }
    if (materialType[record.materialIndex] == 2) { // Dielelectric
        dialetric(ray, record, seed);
        return;
    }
}






// -------------------
// --- Ray Tracing ---
// -------------------

vec3 rayColor(Ray ray, HitRecord record, float tmin, float tmax, float seed) {
    record.color = vec3(1.0);
    record.emmisiveColor = vec3(0.0);

    // Trace the ray in the scene to a max amount of bounces
    for (int bounce = 0; bounce <= maxBounces; bounce++) {
        // Setup the hit record
        record.hit = false;
        record.t = tmax;

        // Check for sphere intersections
        for (int index = 0; index < spheresAmount; index++) {
            hitSphere(ray, record, tmin, record.t, index);
        }

        // Check for 2D primitive intersections
        for (int index = 0; index < quadsAmount; index++) {
            hit2DPrimitive(ray, record, tmin, record.t, index);
        }

        // If nothing was hit, return the background color
        if (!record.hit) {
            return record.color * (background(ray.direction) + record.emmisiveColor);
        }
        
        // If the ray hits something, update the Ray and hitRecord accordingly
        rayHit(ray, record, seed);
        seed += bounce; // Update the seed for the next bounce
    }

    // If the ray bounces the max amount of times, return black (as if the ray was absorbed completely)
    return vec3(0.0);
}






// -------------------
// --- Entry Point ---
// -------------------

void main() {
    // Setup initial variables
    vec3 color;
    vec3 pixelCenter = pixel00 + (gl_FragCoord.x * pixelU) + (gl_FragCoord.y * pixelV);

    int sqrtSamples = int(sqrt(float(samples)));
    float recipSqrtSamples = 1.0 / float(sqrtSamples);
    float sampleWeight = 1.0 / float(sqrtSamples * sqrtSamples);
    
    float seed = hash13(gl_FragCoord.xyz) + seedOffset;

    // Stratified sampling
    for (int si = 0; si < sqrtSamples; si++) {
        for (int sj = 0; sj < sqrtSamples; sj++) {
            // Ensure that the seed for the randomness is different for each sample
            seed += hash11(si + sj * pi);
            
            // Generate a Ray from the camera to the pixel center
            Ray ray;
            ray.origin = (defocusAngle <= 0.0) ? cameraCenter : defocusDiskSample(cameraCenter, defocusAngle, seed);
            // randomInSquareStratified is used for antialiasing
            ray.direction = pixelCenter + randomInSquareStratified(seed, si, sj, recipSqrtSamples) - ray.origin;

            // Setup the HitRecord
            HitRecord record;

            // Trace the ray and accumulate the color
            color += sampleWeight * rayColor(ray, record, smallValue, infinity, seed);
        }
    }

    // Gamma correction
    color = pow(color, vec3(1.0 / gamma));

    // Output the final color
    finalColor = vec4(color, 1.0);
}
