#include "camera.h"

#include <iostream>


void Camera::setPosition(const glm::vec3 p) { 
	m_pos = p; 
}


void Camera::setRotation(const glm::vec3 e) {

	float pitch = e.x;
	float yaw = e.y;
	float roll = e.z;

	// 1.57 is slightly inferior to pi/2
	if (pitch > 1.57f) pitch = 1.57f;
	if (pitch < -1.57f) pitch = -1.57f;

	m_eulers.x = pitch;
	m_eulers.y = yaw;
	m_eulers.z = roll;

	forwards = glm::normalize(glm::vec3(
		cos(pitch) * sin(yaw),
		sin(pitch),
		cos(pitch) * cos(yaw)
	));

	right = glm::normalize(glm::cross(forwards, glm::vec3(0.0f, 1.0f, 0.0f)));

	up = glm::normalize(glm::cross(right, forwards));
}



void Camera::moveCamera(GLFWwindow* window, float dt) {
	float speed = 10 * dt;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		m_pos += up * speed;

	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		m_pos -= up * speed;

	else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		m_pos -= right * speed;

	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		m_pos += right * speed;

	else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		m_pos += forwards * speed;

	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		m_pos -= forwards * speed;
}


void Camera::rotateCamera(GLFWwindow* window) {
	double mouse_x, mouse_y;
	glfwGetCursorPos(window, &mouse_x, &mouse_y);
	glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);

	float sensitivity = 0.01f;

	glm::vec3 de = glm::vec3(
		mouse_y - windowWidth / 2,
		mouse_x - windowHeight / 2,
		0
	);

	setRotation(m_eulers - sensitivity * de);
}