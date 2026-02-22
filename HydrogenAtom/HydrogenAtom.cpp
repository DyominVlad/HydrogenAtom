#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <random>
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
	int WIDTH = 1100, HEIGHT = 1100;

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
			excitedTimer += 0.1f;

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
	Atom(vec2 p, vec2 v) : pos(p), v(v) {
		particles.emplace_back(pos, 1);
		particles.emplace_back(vec2(pos.x - orbitDistance, pos.y), -1);
	
	}
	void update(float dt) {
		pos += v * dt;  // Движение по инерции
	}
};

vector<Atom> atoms { };

// Физические константы
const double BOLTZMANN_CONST = 1;  // k = 1.380649e-23 Дж/К
const double ATOMIC_MASS_UNIT = 1; // 1 а.е.м. = 1.660539e-27 кг

// Модифицированная функция с температурой в Кельвинах
vec2 maxwellVelocity2D(float temperatureKelvin, float massAMU = 1.0f) {
	// temperatureKelvin - температура в Кельвинах
	// massAMU - масса частицы в атомных единицах массы (по умолчанию 1 для водорода)

	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::normal_distribution<> normal(0.0, 1.0);

	// Переводим массу из а.е.м. в кг
	double massKg = massAMU * ATOMIC_MASS_UNIT;

	// Вычисляем sigma = sqrt(kT/m)
	// kT - тепловая энергия в Джоулях
	double kT = BOLTZMANN_CONST * temperatureKelvin;
	double sigma = sqrt(kT / massKg);

	// Возвращаем скорость в м/с
	return vec2(
		normal(gen) * sigma,
		normal(gen) * sigma);
}

int main() 
{
	srand(time(NULL));

	// Initialize 20 atoms in a circle at the center
	int num_atoms = 200;
	float radius = 100.0f; // Radius of the circle
	float maxSpeed = 5.0f;
	float T = 5.0f;

	for (int i = 0; i < num_atoms; i++) {

		vec2 pos(
			((float)rand() / RAND_MAX - 0.5f) * (engine.WIDTH - 200),
			((float)rand() / RAND_MAX - 0.5f) * (engine.HEIGHT - 200)
		);

		vec2 v = maxwellVelocity2D(T);

		atoms.emplace_back(pos, v);
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
			// Обновляем позицию
			a.update(dt);

			// УПРУГОЕ СТОЛКНОВЕНИЕ СО СТЕНКАМИ
			float halfW = engine.WIDTH / 2.0f;
			float halfH = engine.HEIGHT / 2.0f;

			// Левая стенка
			if (a.pos.x < -halfW) {
				a.pos.x = -halfW;  // Корректируем позицию
				a.v.x = -a.v.x;                // Меняем направление (упругий удар)
			}

			// Правая стенка
			if (a.pos.x > halfW) {
				a.pos.x = halfW;
				a.v.x = -a.v.x;
			}

			// Нижняя стенка
			if (a.pos.y < -halfH) {
				a.pos.y = -halfH;
				a.v.y = -a.v.y;
			}

			// Верхняя стенка
			if (a.pos.y > halfH) {
				a.pos.y = halfH;
				a.v.y = -a.v.y;
			}
		}
		// --- Рисуем границы сосуда ---
		glLineWidth(2.0f);
		glColor3f(0.5f, 0.5f, 0.5f);
		glBegin(GL_LINE_LOOP);
		glVertex2f(float(-engine.WIDTH) / 2 + 10, float(-engine.HEIGHT) / 2 + 10);
		glVertex2f(float(engine.WIDTH )/ 2 - 10, float(-engine.HEIGHT) / 2 + 10);
		glVertex2f(float(engine.WIDTH )/ 2 - 10, float(engine.HEIGHT) / 2 - 10);
		glVertex2f(float(-engine.WIDTH) / 2 + 10, float(engine.HEIGHT) / 2 - 10);
		glEnd();


		// --- Draw particles ----
		for (Atom& a : atoms) {
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