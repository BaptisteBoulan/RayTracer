#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (rgba32f, binding = 0) uniform image2D screen;

// === CPU PARAMETERS ===
uniform float FOV;
uniform vec3 forwards;
uniform vec3 up;
uniform vec3 right;
uniform vec3 camPos;

// === OTHER PARAMETERS ===
const int NUM_SPHERES = 3;
const int MAX_BOUNCES = 5;
const float EPSILON = 0.001;
const float SKY_INTENSITY = 1.0;
const float FAR = 10000;

// === STRUCTURES ===
struct Sphere {
    vec3 origin;
    float radius;
    vec3 albedo;
    vec3 specular;
};

Sphere spheres[NUM_SPHERES];

// === GLOBALS ===
vec3 sky_color_top = vec3(1.0);
vec3 sky_color_bottom = vec3(0.0);

// === INITIALIZATION ===
void initSpheres() {
    spheres[0] = Sphere(
        vec3(-10.0, 0.1, 2.0),
        2.0,
        vec3(1.0, 0.0, 0.0),
        vec3(0.5, 0.25, 0.25)
    );

    spheres[1] = Sphere(
        vec3(-10.0, -0.3, -2.0),
        1.0,
        vec3(0.0, 1.0, 0.0),
        vec3(0.25, 0.5, 0.25)
    );

    spheres[2] = Sphere(
        vec3(-6.0, -3.3, 0.0),
        1.5,
        vec3(0.0, 0.0, 1.0),
        vec3(0.25, 0.25, 0.5)
    );
    
}

// === INTERSECTION ===
bool intersectSphere(Sphere s, vec3 ro, vec3 rd, out float t, out vec3 normal) {
    vec3 oc = ro - s.origin;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - s.radius * s.radius;
    float h = b * b - c;
    if (h < 0.0) return false;

    h = sqrt(h);
    t = -b - h;
    if (t < 0.0) t = -b + h;
    if (t < 0.0) return false;

    vec3 p = ro + rd * t;
    normal = normalize(p - s.origin);
    return true;
}

// === SKY ===
vec3 sky(vec3 rd) {
    float t = rd.y * 0.5 + 0.5;
    return t * sky_color_top + (1-t) * sky_color_bottom;
}

// === TRACE ===
// A simple recursive ray tracer with reflection, Fresnel, and soft tint blending.
vec3 traceRay(vec3 ro, vec3 rd) {
    vec3 finalColor = vec3(0.0);
    vec3 throughput = vec3(1.0);

    for (int bounce = 0; bounce < MAX_BOUNCES; bounce++) {
        float closestT = 1e6;
        int hitIndex = -1;
        vec3 nHit = vec3(0.0);

        // --- Find Closest Sphere ---
        for (int i = 0; i < NUM_SPHERES; i++) {
            float t;
            vec3 n;
            if (intersectSphere(spheres[i], ro, rd, t, n)) {
                if (t < closestT) {
                    closestT = t;
                    hitIndex = i;
                    nHit = n;
                }
            }
        }

        // --- Sky / Background ---
        if (hitIndex == -1) {
            finalColor += throughput * sky(rd);
            break;
        }

        // --- Hit Found ---
        Sphere s = spheres[hitIndex];
        vec3 hitPos = ro + rd * closestT + nHit * EPSILON;

        // --- Shading Enhancements ---
        vec3 specular = s.specular;
        vec3 albedo = s.albedo * dot(nHit, vec3(0,1,0));
        vec3 reflectionDir = reflect(rd, nHit);

        // Blend surface color into reflection
        throughput *= specular;
        throughput += albedo;

        // --- Update ray origin/direction for next bounce ---
        ro = hitPos;
        rd = normalize(reflectionDir);

        // --- Final bounce: sample sky reflection ---
        if (bounce == MAX_BOUNCES - 1) {
            finalColor += throughput * sky(rd);
        }
    }

    return finalColor;
}


// === MAIN ===
void main() {
    initSpheres();

    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(screen);

    vec2 uv = (vec2(pixelCoords) + 0.5) / vec2(dims);
    uv = uv * 2.0 - 1.0;

    float aspect = float(dims.x) / float(dims.y);
    uv.x *= aspect;
    
    float fovScale = tan(FOV * 0.5);
    vec3 ro = camPos;
    vec3 rd = uv.x * fovScale * right + uv.y * fovScale * up + forwards;
    rd = normalize(rd);

    vec3 color = traceRay(ro, rd);

    imageStore(screen, pixelCoords, vec4(color, 1.0));
}
