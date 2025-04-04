#version 330 core

#ifdef GL_ES
    precision highp float;
#endif

out vec4 finalColor;

uniform vec3 pixel00;
uniform vec3 pixelU;
uniform vec3 pixelV;
uniform vec3 cameraCenter;

uniform int samples;
uniform int maxBounces;
uniform vec2 seedOffset;
uniform float backgroundOpacity;
uniform float gamma;
uniform float defocusAngle;

#define MAX_MATERIALS 32
uniform int materialType[MAX_MATERIALS];
uniform vec3 materialAlbedo[MAX_MATERIALS];
uniform float materialFuzz[MAX_MATERIALS];
uniform float materialRefractionIndex[MAX_MATERIALS];
uniform sampler2D materialTextures[MAX_MATERIALS];

#define MAX_SPHERES 32
uniform int spheresAmount;
uniform vec4 spheres[MAX_SPHERES];
uniform int sphereMaterialIndicies[MAX_SPHERES];

#define MAX_QUADS 32
uniform int quadsAmount;
uniform vec3 quadOrigins[MAX_QUADS];
uniform vec3 quadEdgesU[MAX_QUADS];
uniform vec3 quadEdgesV[MAX_QUADS];
uniform int quadMaterialIndicies[MAX_QUADS];




const float infinity = pow(2.0, 31.0) - 1.0;
const float pi = 3.14159265359;
const float smallValue = 0.001;

vec3 sunDirection = vec3(1.0, 0.6, 0.5);
vec3 sunColor = vec3(10.0, 10.0, 8.0);
float sunSize = 0.02;




struct Ray {
    vec3 origin;
    vec3 direction;
};

struct HitRecord {
    float t;
    vec3 point;
    vec3 normal;
    bool frontFace;
    bool hit;
    vec3 color;
    int materialIndex;
    vec2 uv;
};







// --- Random Functions ---


float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 78.233);
    return fract(p.x * p.y);
}

float random(vec2 state) {
    return hash(state);
}

vec3 randomVector(vec2 seed) {
    float z = random(seed) * 2.0 - 1.0;
    float t = random(seed + 1.0) * 2.0 * pi;
    float r = sqrt(1.0 - z * z);
    return vec3(r * cos(t), r * sin(t), z);
}

vec3 randomInCircle(vec2 seed) {
    float r = sqrt(random(seed));
    float t = 2.0 * pi * random(seed + 1.61803398875);
    vec3 disk = (pixelU * r * cos(t) + pixelV * r * sin(t));
    return (length(disk) > smallValue && length(disk) < infinity) ? normalize(disk) : vec3(0.0);
}

vec3 defocusDiskSample(vec3 center, float defocusAngle, vec2 seed) {
    return center + randomInCircle(seed) * defocusAngle;
}








// --- Collision Detection ---

void hitQuad(Ray ray, inout HitRecord record, float tmin, float tmax, int quadIndex) {
    // Set quad parameters
    vec3 q = quadOrigins[quadIndex];
    vec3 u = quadEdgesU[quadIndex];
    vec3 v = quadEdgesV[quadIndex];

    // Initial calculations
    vec3 n = cross(u, v);
    vec3 normal = normalize(n);
    float D = dot(normal, q);
    vec3 w = n / dot(n, n);

    // Intersection calculations
    float denominator = dot(normal, ray.direction);
    if (abs(denominator) < smallValue) {
        return;
    }

    float t = (D - dot(normal, ray.origin)) / denominator;
    if (t <= tmin || t >= tmax) {
        return;
    }

    vec3 intersection = ray.origin + t * ray.direction;
    vec3 planarHitPoint = intersection - q;
    float alpha = dot(w, cross(planarHitPoint, v));
    float beta = dot(w, cross(u, planarHitPoint));

    // Determine if the intersection is inside the quad
    if ((alpha >= 0.0 && alpha <= 1.0) && (beta >= 0.0 && beta <= 1.0)) {
        record.hit = true;
        record.t = t;
        record.point = intersection;
        record.normal = normal;
        record.materialIndex = quadMaterialIndicies[quadIndex];
        record.frontFace = dot(ray.direction, record.normal) < 0.0;
        if (!record.frontFace) {
            record.normal = -record.normal;
        }
        record.uv = vec2(beta, 1.0 - alpha);
    }
}

void hitSphere(Ray ray, inout HitRecord record, float tmin, float tmax, int sphereIndex) {
    vec3 center = spheres[sphereIndex].xyz;
    float radius = spheres[sphereIndex].w;

    // Calculate initial variables
    vec3 oc = center - ray.origin;
    float a = dot(ray.direction, ray.direction);
    float h = dot(ray.direction, oc);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = h * h - a * c;

    if (discriminant > 0.0) {
        // Find the nearest root
        float sqrtd = sqrt(discriminant);
        float root = (h - sqrtd) / a;
        if (root <= tmin || root >= tmax) {
            root = (h + sqrtd) / a;
            if (root <= tmin || root >= tmax) {
                return;
            }
        }

        record.hit = true;
        record.t = root;
        record.point = ray.origin + root * ray.direction;
        record.normal = (record.point - center) / radius;
        record.materialIndex = sphereMaterialIndicies[sphereIndex];
        record.frontFace = dot(ray.direction, record.normal) < 0.0;
        if (!record.frontFace) {
            record.normal = -record.normal;
        }
        vec3 p = record.normal;
        record.uv = vec2(0.5 + atan(p.z, p.x) / (2.0 * pi), 0.5 - asin(p.y) / pi);
    } 
}








// --- Materials ---

void updateColor(inout HitRecord record) {
    vec3 textureColor = texture(materialTextures[record.materialIndex], record.uv).rgb;
    if (length(textureColor) < smallValue) {
        record.color *= materialAlbedo[record.materialIndex];
    } else {
        record.color *= textureColor;
    }
}

vec3 background(vec3 direction) {
    vec3 unitDirection = normalize(direction);
    float t = 0.5 * (unitDirection.y + 1.0);
    vec3 baseColor = vec3(1.0 - t) + t * vec3(0.4, 0.6, 1.0);

    float angle = acos(dot(unitDirection, normalize(sunDirection)));
    float sunEffect = exp(-pow(angle / sunSize, 0.8));
    vec3 finalColor = mix(baseColor, sunColor, sunEffect);

    return finalColor * backgroundOpacity;
}

float reflectance(float cosine, float refractionIndex) {
    float r0 = (1.0 - refractionIndex) / (1.0 + refractionIndex);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow((1.0 - cosine), 5.0);
}

void lambertian(inout Ray ray, inout HitRecord record, vec2 seed) {
    ray.direction = record.normal + randomVector(seed + record.t);
    // If normal and randomVector are opposite, the vector will be zero and will result in weird behavior
    if (length(ray.direction) < smallValue) {
        ray.direction = record.normal;
    }

    updateColor(record);
}

void metal(inout Ray ray, inout HitRecord record, vec2 seed) {
    ray.direction = ray.direction - 2.0 * dot(ray.direction, record.normal) * record.normal + materialFuzz[record.materialIndex] * randomVector(seed);
    updateColor(record);
}

void dialetric(inout Ray ray, inout HitRecord record, vec2 seed) {
    float ri = record.frontFace ? (1.0 / materialRefractionIndex[record.materialIndex]) : materialRefractionIndex[record.materialIndex];
    vec3 unitDirection = normalize(ray.direction);

    float cosTheta = min(dot(-unitDirection, record.normal), 1.0);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    bool cannotRefract = ri * sinTheta > 1.0;

    if (cannotRefract || reflectance(cosTheta, ri) > random(seed)) {
        // Reflect
        metal(ray, record, seed);
        return;
    }

    // Refract
    vec3 rOutPerp = ri * (unitDirection + cosTheta * record.normal);
    vec3 rOutParallel = -sqrt(abs(1.0 - length(rOutPerp) * length(rOutPerp))) * record.normal;
    vec3 refracted = rOutPerp + rOutParallel;

    ray.direction = refracted;
    updateColor(record);
}

void rayHit(inout Ray ray, inout HitRecord record, inout bool stop, vec2 seed) {
    ray.origin += record.t * ray.direction;

    if (materialType[record.materialIndex] == 0) {
        lambertian(ray, record, seed);
        return;
    }

    if (materialType[record.materialIndex] == 1) {
        metal(ray, record, seed);
        return;
    }

    if (materialType[record.materialIndex] == 2) {
        dialetric(ray, record, seed);
        return;
    }

    if (materialType[record.materialIndex] == 3) {
        updateColor(record);
        stop = true;
        return;
    }
}








// --- Ray Tracing ---

vec3 rayColor(Ray ray, HitRecord record, float tmin, float tmax, vec2 seed) {
    for (int bounce = 0; bounce <= maxBounces; bounce++) {
        record.hit = false;
        record.t = tmax;
        bool stop = false;

        // Check for sphere intersections
        for (int index = 0; index < spheresAmount; index++) {
            hitSphere(ray, record, tmin, record.t, index);
        }

        // Check for quad intersections
        for (int index = 0; index < quadsAmount; index++) {
            hitQuad(ray, record, tmin, record.t, index);
        }

        if (!record.hit) {
            return record.color * background(ray.direction);
        }
        
        rayHit(ray, record, stop, seed);
        if (stop == true) { return record.color; }
        seed += vec2(float(bounce), float(bounce) * 1.61803398875);
    }

    return vec3(0.0);
}








// --- Main ---

void main() {
    vec3 color = vec3(0.0);
    vec2 st = gl_FragCoord.xy;
    vec3 pixelCenter = pixel00 + (st.x * pixelU) + (st.y * pixelV);
    float sampleWeight = 1.0 / float(samples);

    for (int sample = 0; sample < samples; sample++) {
        vec2 seed = random(st) + vec2(sample) + seedOffset;

        Ray ray;
        ray.origin = (defocusAngle <= 0.0) ? cameraCenter : defocusDiskSample(cameraCenter, defocusAngle, seed);
        ray.direction = pixelCenter - ray.origin;

        HitRecord record;
        record.color = vec3(1.0);

        color += sampleWeight * rayColor(ray, record, smallValue, infinity, seed);
    }

    // Gamma correction
    color = pow(color, vec3(1.0 / gamma));

    finalColor = vec4(color, 1.0);
}