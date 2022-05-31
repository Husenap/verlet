#pragma once

#include <vector>

#include <glm/glm.hpp>

struct VerletObject {
	glm::vec2 currentPosition  = {};
	glm::vec2 previousPosition = {};
	glm::vec2 acceleration     = {};
	float     radius           = 5.f;

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
	void addObject() { objects.emplace_back(); }

	void update(float dt) {
		applyGravity();
		applyConstraint();
		updatePositions(dt);
	}

	template <typename Fn>
	void apply(Fn fn) const {
		for (const auto& o : objects) {
			fn(o);
		}
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
		static constexpr glm::vec2 center = {-100,0};
		static constexpr float radius = 100.f;

		for (auto& o : objects) {
			const auto toObj = o.currentPosition - center;
			const float dist  = glm::length(toObj);
			if (dist > radius - o.radius) {
				const auto n = toObj / dist;
				o.currentPosition = center + n * (radius - o.radius);
			}
		}
	}

	std::vector<VerletObject> objects;
};
