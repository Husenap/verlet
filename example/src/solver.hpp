#pragma once

#include <cmath>
#include <unordered_map>
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

namespace std {
template <>
struct hash<glm::ivec2> {
	inline size_t operator()(const glm::ivec2& v) const {
		std::hash<int> int_hasher;
		return int_hasher(v.x) ^ int_hasher(v.y);
	}
};
}  // namespace std

static float CellSize = 50.f;
struct SpatialPartition {
	std::unordered_map<glm::ivec2, std::vector<int>> cells;

	void clear() { cells.clear(); }

	void insert(const VerletObject& o, int id) {
		const auto [min, max] = getRange(o);
		for (int y = min.y; y <= max.y; ++y) {
			for (int x = min.x; x <= max.x; ++x) {
				cells[glm::ivec2(x, y)].push_back(id);
			}
		}
	}

	std::pair<glm::ivec2, glm::ivec2> getRange(const VerletObject& o) const {
		const auto& p = o.currentPosition;
		const float r = o.radius;
		return {{static_cast<int>(std::floor((p.x - r) / CellSize)),
		         static_cast<int>(std::floor((p.y - r) / CellSize))},
		        {static_cast<int>(std::floor((p.x + r) / CellSize)),
		         static_cast<int>(std::floor((p.y + r) / CellSize))}};
	}

	template <typename Fn>
	void apply(const VerletObject& o, Fn fn) const {
		const auto [min, max] = getRange(o);
		for (int y = min.y; y <= max.y; ++y) {
			for (int x = min.x; x <= max.x; ++x) {
				auto it = cells.find({x, y});
				if (it != cells.end()) {
					for (int id : it->second) {
						fn(id);
					}
				}
			}
		}
	}
};

class Solver {
	static constexpr glm::vec2 Gravity = {0.0f, 982.f};

public:
	void addObject() {
		objects.push_back(VerletObject{
		    .currentPosition = {static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX},
		    .radius =
		        (std::pow(static_cast<float>(rand()) / RAND_MAX, 4.f)) * 6.f +
		        6.f,
		    .color = IM_COL32(rand() % 255, rand() % 255, rand() % 255, 255),
		});

		averageRadius = 0.f;
		for (auto& o : objects) {
			averageRadius += o.radius;
		}
		averageRadius /= objects.size();
	}

	void update(float dt) {
		static constexpr int SubSteps = 8;
		const float          subDt    = dt / static_cast<float>(SubSteps);

		numCollisions = 0;

		for (int i = 0; i < SubSteps; ++i) {
			applyGravity();
			applyConstraint();

			partition.clear();
			for (int id = 0; id < objects.size(); ++id) {
				partition.insert(objects[id], id);
			}
			solveCollisions();

			updatePositions(subDt);
		}

		numCollisions /= SubSteps;
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
			ImGui::DragFloat("Map Radius", &mapRadius);
			ImGui::DragFloat("Cell Size", &CellSize);
			ImGui::Text("Number of Objects: %d", objects.size());
			ImGui::Text("Number of Collisions: %d", numCollisions);
			ImGui::Text("Average Radius: %f", averageRadius);
		}
		ImGui::End();
	}

	const float getMapRadius() const { return mapRadius; }

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
		static constexpr glm::vec2 center = {0, 0};

		for (auto& o : objects) {
			const auto  toObj = o.currentPosition - center;
			const float dist  = glm::length(toObj);
			if (dist > mapRadius - o.radius) {
			    const auto n      = toObj / dist;
			    o.currentPosition = center + n * (mapRadius - o.radius);
			}
		}
	}

	void solveCollisions() {
		for (int i = 0; i < objects.size(); ++i) {
			auto& a = objects[i];
			partition.apply(a, [&](int id) {
				if (id <= i) return;
				auto& b = objects[id];

				const auto collisionAxis =
				    a.currentPosition - b.currentPosition;
				const float dist = glm::length(collisionAxis);
				if (dist > 0.f && dist < a.radius + b.radius) {
					const auto  n     = collisionAxis / dist;
					const float delta = (a.radius + b.radius) - dist;
					a.currentPosition += 0.5f * delta * n;
					b.currentPosition -= 0.5f * delta * n;

					++numCollisions;
				}
			});
		}
	}

	std::vector<VerletObject> objects;
	SpatialPartition          partition;

	float mapRadius     = 450.f;
	float averageRadius = 0.f;
	int   numCollisions = 0;
};
