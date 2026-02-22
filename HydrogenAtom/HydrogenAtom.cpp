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

using namespace std;
using namespace glm;

// --- variables --- 
float orbitDistance = 15.0f;

// --- engine ---
struct Engine {

	GLFWwindow* window;
	int WIDTH = 1000, HEIGHT = 1000;

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

// --- Waves
struct WavePoint { vec2 localPos; vec2 dir; };
struct Wave {

	vec2 pos, dir;
	vec3 col;

	float energy, wavelenght, frequency;
	float sigma = 40.0f, k = 0.4f, phase = 0.0f, amp = 10.0f, angleR;
	vector<WavePoint> points;

	Wave(float e, vec2 pos, vec2 dir, vec3 col = vec3(0.0, 1.0, 1.0)) : energy(e), pos(pos), dir(dir), col(col) {
		
		this->dir = normalize(dir);

		for (float x = -sigma; x <= sigma; x += 0.1f)
			points.push_back({ pos + x * this->dir, this->dir * 200.0f });

		angleR = atan2(this->dir.y, this->dir.x);
	}

	void draw() {

		glLineWidth(2.0f);
		glColor3f(col.r, col.g, col.b);
		
		glBegin(GL_LINE_STRIP);
		int drawPionts = 0;

		for (WavePoint& p : points) {

			vec2 perp(-p.dir.y, p.dir.x);
			perp = normalize(perp);

			float y_disp = amp * sin(k * length(p.localPos - pos) - phase);

			vec2 drawPos = p.localPos + perp * y_disp;
			glVertex2f(drawPos.x, drawPos.y);
		}
		glEnd();
	}


	bool update(float dt) {
		phase += 30.0f * dt;

		bool allOutofBounds = true;

		for (WavePoint& p : points) {
			//move along velocity
			p.localPos += p.dir * dt * 0.5f;

			if (p.localPos.x < -engine.WIDTH / 2.0f || p.localPos.x > -engine.WIDTH / 2.0f ||
				p.localPos.y < -engine.HEIGHT / 2.0f || p.localPos.y > -engine.HEIGHT / 2.0f)
				allOutofBounds = false;
		}
		return allOutofBounds;
	}

	bool isDead() {
		// Проверяем, ушла ли волна далеко за экран
		float margin = 200.0f;  // Запас

		// Проверяем центр волны
		if (pos.x < float(-engine.WIDTH) / 2 - margin ||
			pos.x > float(engine.WIDTH) / 2 + margin ||
			pos.y < float(-engine.HEIGHT) / 2 - margin ||
			pos.y > float(engine.HEIGHT) / 2 + margin) {
			return true;
		}

		// Дополнительно проверяем, не "зависла" ли волна
		if (energy == 0.0f) {
			return true;
		}

		return false;
	}
};

vector<Wave> waves { };

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;

	double mx, my;
	glfwGetCursorPos(window, &mx, &my);

	Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));

	// screen → world (matches your glOrtho setup)
	float worldX = (float)mx - engine->WIDTH / 2.0f;
	float worldY = engine->HEIGHT / 2.0f - (float)my;
	vec2 spawnPos(worldX, worldY);

	// spawn 25 waves in all directions
	float energyN1toN2 = -13.6f / (2 * 2) - (-13.6f);
	for (int i = 0; i < 25; i++) {
		float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
		vec2 dir(cos(angle), sin(angle));

		waves.push_back(
			Wave(energyN1toN2, spawnPos, dir)
		);
	}
}

struct Particle {
	vec2 pos;
	int charge;
	float  angle = 0.0;
	int n = 1;
	float excitedTimer = 0.0f;
	Particle(vec2 pos, int charge) : pos(pos), charge(charge) {}

	void draw(vec2 center, int segments = 50) {

		/// --- draw outline ---
		if (charge == -1) {
			glLineWidth(0.4f);
			glBegin(GL_LINE_LOOP);
			glColor3f(0.4f, 0.4f, 0.4f);

			for (int i = 0; i <= segments; i++) {
				float angle = 2.0f * M_PI * i / segments;
				float x = cos(angle) * n * orbitDistance;
				float y = sin(angle) * n * orbitDistance;
				glVertex2f(x + center.x, y + center.y);
			}
			glEnd();
		}

		float r;
		if (charge == -1)		{ r = 2; glColor3f(0.0f, 1.0f, 1.0f); }
		else if (charge == 1)	{ r = 5; glColor3f(1.0f, 0.0f, 0.0f); }
		else					{ r = 5; glColor3f(0.5f, 0.5f, 0.5f); }

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
	void update(vec2 c) {

		float r = n * orbitDistance;
		angle += 0.001f;
		pos = vec2(cos(angle) * r + c.x, sin(angle) * r + c.y);

		if (excitedTimer <= 0.0 && n > 1) {
			n--;
			excitedTimer += 0.1;

			float waveDirX = float(rand() / RAND_MAX) * 2.0f - 1.0f;
			float waveDirY = float(rand() / RAND_MAX) * 2.0f - 1.0f;

			vec2 dir(waveDirX, waveDirY);
			if (length(dir) > 0.01f) {
				dir = normalize(dir);
				float energyDiff = -13.6f / ((n + 1) * (n + 1)) - (-13.6f / (n * n));

				waves.emplace_back(energyDiff, pos, dir, vec3(1.0f, 1.0f, 0.0f));
			}
		}
	}

};

struct Atom {
	vec2 pos;
	vec2 v = vec2(0.0);
	std::vector<Particle> particles = { };
	Atom(vec2 p) : pos(p) {
		particles.emplace_back(pos, 1);
		particles.emplace_back(vec2(pos.x - orbitDistance, pos.y), -1);
	
	}
};

vector<Atom> atoms { };

int main() 
{
	srand(time(nullptr));

	// Initialize 20 atoms in a circle at the center
	{
		int num_atoms = 150;
		float radius = 100.0f; // Radius of the circle
		for (int i = 0; i < num_atoms; i++) {
			float angle = 2.0f * M_PI * i / num_atoms;
			float x = cos(angle) * radius;
			float y = sin(angle) * radius;
			atoms.emplace_back(vec2(x, y));
		}
	}

	// callbacks
	glfwSetWindowUserPointer(engine.window, &engine);
	glfwSetMouseButtonCallback(engine.window, mouseButtonCallback);

	// --- initialise waves ---
	float energy_N1toN2 = -13.6f / (2 * 2) - (-13.6f);


	while (!glfwWindowShouldClose(engine.window)) {
		engine.run();

		float dt = 0.1f; // Шаг времени

		for (Atom& a : atoms) {
			// --- Сохраняем старую позицию для проверки коллизий ---
			vec2 oldPos = a.pos;

			// --- Применяем силы ---

			// 1. Отталкивание между атомами (как молекулы газа)
			for (Atom& a2 : atoms) {
				if (&a2 == &a) continue;

				vec2 delta = a.pos - a2.pos;
				float dist = length(delta);
				float minDist = 100.0f; // Минимальное расстояние (диаметр атома)

				if (dist < minDist && dist > 0.01f) {
					vec2 dir = normalize(delta);
					// Сильное отталкивание при столкновении
					float overlap = minDist - dist;
					a.pos += dir * overlap * 0.5f; // Раздвигаем атомы
					a2.pos -= dir * overlap * 0.5f;

					// Обмен импульсами (упругое столкновение)
					vec2 v1 = a.v;
					vec2 v2 = a2.v;
					a.v = v1 - 2.0f * dot(v1 - v2, dir) * dir / 2.0f;
					a2.v = v2 - 2.0f * dot(v2 - v1, -dir) * (-dir) / 2.0f;
				}
			}

			// 2. Гравитация (опционально)
			// a.v.y -= 0.1f * dt;

			// 3. Обновляем позицию
			a.pos += a.v * dt;

			// --- ЖЕСТКИЕ СТЕНКИ (как у газового сосуда) ---
			float halfW = engine.WIDTH / 2.0f;
			float halfH = engine.HEIGHT / 2.0f;
			float atomRadius = 50.0f; // Радиус атома

			// Левая стенка
			if (a.pos.x - atomRadius < -halfW) {
				a.pos.x = -halfW + atomRadius;
				a.v.x = -a.v.x * 1.0f; // 1.0f = абсолютно упругий удар
			}

			// Правая стенка
			if (a.pos.x + atomRadius > halfW) {
				a.pos.x = halfW - atomRadius;
				a.v.x = -a.v.x * 1.0f;
			}

			// Нижняя стенка
			if (a.pos.y - atomRadius < -halfH) {
				a.pos.y = -halfH + atomRadius;
				a.v.y = -a.v.y * 1.0f;
			}

			// Верхняя стенка
			if (a.pos.y + atomRadius > halfH) {
				a.pos.y = halfH - atomRadius;
				a.v.y = -a.v.y * 1.0f;
			}

			// --- Рисуем "стенки сосуда" для наглядности ---
			glLineWidth(3.0f);
			glColor3f(0.7f, 0.7f, 0.7f);
			glBegin(GL_LINE_LOOP);
			glVertex2f(-halfW + 2, -halfH + 2);
			glVertex2f(halfW - 2, -halfH + 2);
			glVertex2f(halfW - 2, halfH - 2);
			glVertex2f(-halfW + 2, halfH - 2);
			glEnd();
		}

		// --- Draw particles ----
		for (Atom& a: atoms) {
			for (Particle& p : a.particles) {
				p.draw(a.pos);

				if (p.charge == 1)
					p.pos = a.pos;
				if (p.charge == -1) {
					if (p.excitedTimer > 0.0f)
						p.excitedTimer -= 0.001f;
					p.update(a.pos);

					for (Wave& w : waves) {
						for (WavePoint& wp : w.points) {
							float dist = length(p.pos - wp.localPos);
							float energyforUp = -13.6f / ((p.n + 1) * (p.n + 1)) - (-13.6f / (p.n * p.n));

							if (dist < 20.0f && w.energy == energyforUp && w.col != vec3(1.0f, 1.0f, 0.0f)) {
								w.energy = 0.0f;
								p.n++;
								p.excitedTimer += 0.003f;

								break;
							}
						}
					}
				}
			}
		}

		// --- draw waves ---

		for (auto it = waves.begin(); it != waves.end(); ) {
			it->update(0.01f);

			if (it->energy != 0.0)
				it->draw();

			if (it->energy == 0.0f || it->isDead()) {
				it = waves.erase(it);
			}
			else
				++it;
		}

		glfwSwapBuffers(engine.window);
		glfwPollEvents();
	}

	return 0;
}