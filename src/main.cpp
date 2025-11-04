#include "config.h"
#include "shader.h"
#include "camera.h"

GLFWwindow *window;

// Shaders
GLuint renderShader;
GLuint computeShader;

// Textures
GLuint screenTex;

// VAO
GLuint VAO, VBO_pos, VBO_uvs, EBO;

// Arrays
std::vector<float> positions;
std::vector<float> uvs;
std::vector<unsigned int> indices;

// Constantes
const int width = 800;
const int height = 600;

// Camera
Camera * camera;
float fov = glm::radians(45.0f);
glm::vec3 initialCamPos = glm::vec3(0, 0, 0);
glm::vec3 initialEulers = glm::vec3(0, 0, 0);

// Uniform locations
GLuint fovLocation;
GLuint camPosLocation;
GLuint forwardsLocation;
GLuint upLocation;
GLuint rightLocation;

float random() { return (float)rand() / RAND_MAX; }

void windowSizeCallback(GLFWwindow* window, int width, int height) {
  camera->setAspectRatio(static_cast<float>(width)/static_cast<float>(height));
  glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

bool initOpenGL() {
    if (!glfwInit()) {
        std::cerr << "Error: failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Renderze RTX - 0FPS", nullptr, nullptr);
    if (!window) {
        std::cerr << "Error: failed to create window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSetWindowSizeCallback(window, windowSizeCallback);
    glViewport(0, 0, width, height);

    return true;
}

void initShaders() {
    renderShader = createShaderProgram({
        {"shaders/vertex.glsl", ShaderType::VERTEX},
        {"shaders/fragment.glsl", ShaderType::FRAGMENT},
    });

    computeShader = createShaderProgram({
        {"shaders/compute.glsl", ShaderType::COMPUTE},
    });
}

void initCamera() {
    camera = new Camera();
    camera->setPosition(initialCamPos);
    camera->setRotation(initialEulers);
    camera-> setAspectRatio(width/height);
    camera->setFoV(fov);
    camera->setWindowSize(width, height);
    
    glfwSetCursorPos(window, width / 2, height / 2);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void initCpuGeometry() {
    positions = {
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
    };

    uvs = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    };


    indices = {
        0, 1, 2,
        0, 2, 3,
    };
}


void initTextures() {
    glGenTextures(1, &screenTex);
    glBindTexture(GL_TEXTURE_2D, screenTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, (void*)0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


void initGpuGeometry() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO_pos);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &VBO_uvs);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_uvs);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), uvs.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void getLocations() {
    fovLocation = glGetUniformLocation(computeShader, "FOV");
    camPosLocation = glGetUniformLocation(computeShader, "camPos");
    forwardsLocation = glGetUniformLocation(computeShader, "forwards");
    upLocation = glGetUniformLocation(computeShader, "up");
    rightLocation = glGetUniformLocation(computeShader, "right");
}

float lastTime = 0.0f;
float timeAccumulator = 0.0f;
int frameAccumulator = 0;

void update(const float currentTimeInSec) {
            if (timeAccumulator >= 1.0f) {
            std::string titleWithFPS = "Renderze RTX - " + std::to_string(frameAccumulator) + "FPS";
            glfwSetWindowTitle(window, titleWithFPS.c_str());

            frameAccumulator = 0;
            timeAccumulator -= 1.0f;
    }
    float dt = currentTimeInSec - lastTime;
    timeAccumulator += dt;
    lastTime = currentTimeInSec;
    frameAccumulator++;

    camera->moveCamera(window, dt);
    camera->rotateCamera(window);
}

void render() {
    // --- COMPUTE PASS ---
    glUseProgram(computeShader);

    // Pass uniform variables
    glm::vec3 camPos = camera->getPosition();
    glm::vec3 forwards = camera->getForwards();
    glm::vec3 up = camera->getUp();
    glm::vec3 right = camera->getRight();

    glUniform1f(fovLocation, camera->getFov());
    glUniform3f(camPosLocation, camPos.x, camPos.y, camPos.z);
    glUniform3f(forwardsLocation, forwards.x, forwards.y, forwards.z);
    glUniform3f(upLocation, up.x, up.y, up.z);
    glUniform3f(rightLocation, right.x, right.y, right.z);

    // Bind image to unit 0 for compute shader to write into
    glBindImageTexture(0, screenTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Dispatch compute shader
    GLuint groups_x = (GLuint)ceil(width / 8.0f);
    GLuint groups_y = (GLuint)ceil(height / 4.0f);
    glDispatchCompute(groups_x, groups_y, 1);

    // Make sure writes are visible to subsequent read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // --- RENDER PASS ---
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(renderShader);

    // Bind the texture for the fragment shader
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTex);
    glUniform1i(glGetUniformLocation(renderShader, "screen"), 0);

    // Draw fullscreen quad
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void endProgram() {
    glDeleteProgram(renderShader);
    glDeleteProgram(computeShader);
    glfwTerminate();
}

int main() {
    // Initialization
    initOpenGL();
    initCamera();
    initShaders();
    initCpuGeometry();
    initTextures();
    initGpuGeometry();

    // Render Loop
    getLocations();
    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        update(static_cast<float>(glfwGetTime()));
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
        glfwSwapInterval(1);
    }

    // Terminate
    endProgram();
    return 0;
}
