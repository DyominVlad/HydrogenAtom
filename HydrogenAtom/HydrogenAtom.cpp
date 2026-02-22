#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <vector>

#ifndef  M_PI
#define M_PI 3.14159265358979323846
#endif

// --- variables --- 
float orbitDistance = 15.0f;

// --- engine ---
struct Wave;
glm::vec2 mouseWorld(0.0f);
struct Engine {

	GLFWwindow* window;
	int WIDTH = 800, HEIGHT = 600;

	Engine() {
		// --- Init GLFW ---
		if (!glfwInit()) {
			std::cerr << "failed to init glfw, LOL";
			exit(EXIT_FAILURE);
		}

			// --- Create Window ---
		window = glfwCreateWindow(WIDTH, HEIGHT, "2D atom simulation", nullptr, nullptr);
		if (!window) {
			std::cerr << "failed to create window";
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		glfwMakeContextCurrent(window);
		int fbWidth, fbHeight;
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
		glViewport(0, 0, fbWidth, fbHeight);
	}

	void run() {
		glClear(GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		// --- set origin to centre
		double halfWidth = WIDTH / 2.0f, halfHeight = HEIGHT / 2.0f;
		glOrtho(-halfWidth, halfWidth, -halfHeight, halfHeight, -1.0, 1.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

};
Engine engine;

struct Particle {
	glm::vec2 pos;
	int charge;
	float angle = 0.0f;
	float excitedTimer = 0.0f;

	Particle(glm::vec2 pos, int charge) : pos(pos), charge(charge) {}

	void draw(int segments = 50) {

		float r;
		if (charge == -1) {
			r = 2;
			glColor3f(0.0f, 1.0f, 1.0f);
		}
		else if (charge == 1) {
			r = 10;
			glColor3f(1.0f, 0.0f, 0.0f);
		}
		else {
			r = 10;
			glColor3f(0.5f, 0.5f, 0.5f);
		}


		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(pos.x, pos.y);

		for (int i = 0; i <= segments; i++) {
			float angle = 2.0f * M_PI * i / segments;
			float x = cos(angle) * r;
			float y = sin(angle) * r;
			glVertex2f(x + pos.x, y + pos.y);
		}
		glEnd();
		 
	}
};
std::vector<Particle> particles = {
	Particle(glm::vec2(0.0f), 1),
	Particle(glm::vec2(-50.0f, 0.0f), -1)
};

static void mouseButtonCallBack(GLFWwindow* window, int button, int action, int mods) {
	if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;

	double mx, my;
	glfwGetCursorPos(window, &mx, &my);

	Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));

	// screen -> world (matches your glOrtho setup)
	float worldX = (float)mx - engine->WIDTH / 2.0f;
	float worldY = engine->HEIGHT / 2.0f - (float)my;
	glm::vec2 spawnPos(worldX, worldY);

	// spawn 25 waves in all directions
	float energyN1tonN2 = -13.6f / (2 * 2) - (-13.6f);
	for (int i = 0; i < 25; i++) {
		float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
		glm::vec2 dir(cos(angle), sin(angle));

	}
}

int main() {

	// callbacks
	//glfwSetWindowUserPointer(engine.window, &engine);
	//glfwSetMouseButtonCallback(engine.window, mouseButtonCallback);

	while (!glfwWindowShouldClose(engine.window)) {

		engine.run();
		for (Particle p : particles)
			p.draw();

		glfwSwapBuffers(engine.window);
		glfwPollEvents();
	}
}