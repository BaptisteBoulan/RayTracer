#pragma once
#include "config.h"

class Camera {
public:
    inline float getFov() const { return m_fov; }
    inline void setFoV(const float f) { m_fov = f; }
    inline float getAspectRatio() const { return m_aspectRatio; }
    inline void setAspectRatio(const float a) { m_aspectRatio = a; }
    inline void setWindowSize(const float w, const float h) { windowWidth = int(h);  windowHeight = int(h); }
    void setPosition(const glm::vec3 p);
    void setRotation(const glm::vec3 e);
    inline const glm::vec3 getPosition() { return m_pos; }
    inline const glm::vec3 getRotation() { return m_eulers; }

    inline glm::vec3 getForwards() { return forwards;}
    inline glm::vec3 getUp() { return up;}
    inline glm::vec3 getRight() { return right;}

    void moveCamera(GLFWwindow * window, float dt);
    void rotateCamera(GLFWwindow* window);

private:
    glm::vec3 m_pos = glm::vec3(0, 0, 3);
    glm::vec3 m_eulers = glm::vec3(0, 0, 0);
    float m_fov = 45.f;
    float m_aspectRatio = 1.f;

    glm::vec3 forwards = glm::vec3(0, 0, 1);
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec3 right = glm::vec3(-1, 0, 0);

    int windowWidth, windowHeight;
};
