#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <imgui/imgui.h>

struct VerletObject {
	glm::vec2 currentPosition  = {};
	glm::vec2 previousPosition = {};
	glm::vec2 acceleration     = {};
	float     radius           = 5.f;
	uint32_t  color            = 0xffffffff;

	void updatePosition(float dt) {
		const glm::vec2 velocity = currentPosition - previousPosition;

		previousPosition = currentPosition;

		currentPosition += velocity + acceleration * dt * dt;

		acceleration = {};
	}

	void accelerate(glm::vec2 acc) { acceleration += acc; }
};

class Solver {
	static constexpr glm::vec2 Gravity = {0.0f, 982.f};

public:
	void addObject() {
		objects.push_back(VerletObject{
		    .radius = (static_cast<float>(rand()) / RAND_MAX) * 8.f + 1.f,
		    .color  = IM_COL32(rand() % 255, rand() % 255, rand() % 255, 255),
		});
	}

	void update(float dt) {
		static constexpr int SubSteps = 4;
		const float          subDt    = dt / static_cast<float>(SubSteps);
		for (int i = 0; i < SubSteps; ++i) {
			applyGravity();
			applyConstraint();
			solveCollisions();
			updatePositions(subDt);
		}
	}

	void clear() { objects.clear(); }

	template <typename Fn>
	void apply(Fn fn) const {
		for (const auto& o : objects) {
			fn(o);
		}
	}
	
	void debug() {
		if (ImGui::Begin("Verlet Debug")) {
			ImGui::Text("Number of Objects: %d", objects.size());
		}
		ImGui::End();
	}

private:
	void updatePositions(float dt) {
		for (auto& o : objects) {
			o.updatePosition(dt);
		}
	}

	void applyGravity() {
		for (auto& o : objects) {
			o.accelerate(Gravity);
		}
	}

	void applyConstraint() {
		static constexpr glm::vec2 center = {-100, 0};
		static constexpr float     radius = 500.f;

		for (auto& o : objects) {
			const auto  toObj = o.currentPosition - center;
			const float dist  = glm::length(toObj);
			if (dist > radius - o.radius) {
				const auto n      = toObj / dist;
				o.currentPosition = center + n * (radius - o.radius);
			}
		}
	}

	void solveCollisions() {
		for (int i = 0; i < objects.size(); ++i) {
			auto& a = objects[i];
			for (int j = i + 1; j < objects.size(); ++j) {
				auto& b = objects[j];

				const auto collisionAxis =
				    a.currentPosition - b.currentPosition;
				const float dist = glm::length(collisionAxis);
				if (dist > 0.f && dist < a.radius + b.radius) {
					const auto  n     = collisionAxis / dist;
					const float delta = (a.radius + b.radius) - dist;
					a.currentPosition += 0.5f * delta * n;
					b.currentPosition -= 0.5f * delta * n;
				}
			}
		}
	}

	std::vector<VerletObject> objects;
};
