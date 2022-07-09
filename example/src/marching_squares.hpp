#pragma once

#include <tuple>
#include <vector>

#include <glm/glm.hpp>
#include <imgui/imgui.h>

struct Circle {
	glm::vec2 pos;
	float     radius;
};
class MarchingSquares {
public:
	MarchingSquares() { points.resize((Width + 1) * (Height + 1), 0.f); }
	void newFrame() {
		circles.clear();
		for (auto& p : points) {
			p = std::numeric_limits<float>::infinity();
		}
	}
	void addCircle(glm::vec2 pos, float radius) {
		circles.push_back({pos, std::max(radius, 1.f * CellSize)});
	}

	const float smin(float a, float b, float k) {
		const float h = std::max(k - std::abs(a - b), 0.f) / k;
		return std::min(a, b) - h * h * k * (1.f / 4.f);
	}

	void draw(ImDrawList* draw_list, const glm::mat3& A) {
#pragma omp parallel for
		for (int i = 0; i < points.size(); ++i) {
			const auto x  = i % Stride;
			const auto y  = i / Stride;

			const auto pp = glm::vec2((x - Width / 2) * CellSize,
			                          (y - Height / 2) * CellSize);
			const auto p0 = A * glm::vec3(pp, 1.f);

			for (auto [cp, r] : circles) {
				points[x + y * (Width + 1)] = smin(glm::distance(cp, pp) - r,
				                                   points[x + y * (Width + 1)],
				                                   smoothness);
			}
		}

		std::vector<std::pair<glm::vec3, glm::vec3>> lines;
		std::vector<std::vector<glm::vec3>>          polys;
		for (int y = 0; y < Height; ++y) {
			for (int x = 0; x < Width; ++x) {
				static constexpr float Threshold = 0.f;

				const auto v0   = points[x + y * Stride];
				const auto v1   = points[x + 1 + y * Stride];
				const auto v2   = points[x + 1 + (y + 1) * Stride];
				const auto v3   = points[x + (y + 1) * Stride];
				int8_t     mask = 0;
				if (v0 <= Threshold) mask |= 0x1;
				if (v1 <= Threshold) mask |= 0x2;
				if (v2 <= Threshold) mask |= 0x4;
				if (v3 <= Threshold) mask |= 0x8;

				const auto p0 = A * glm::vec3((x - Width / 2) * CellSize,
				                              (y - Height / 2) * CellSize,
				                              1.f);
				const auto p2 = A * glm::vec3((x + 1 - Width / 2) * CellSize,
				                              (y - Height / 2) * CellSize,
				                              1.f);
				const auto p6 = A * glm::vec3((x - Width / 2) * CellSize,
				                              (y + 1 - Height / 2) * CellSize,
				                              1.f);
				const auto p8 = A * glm::vec3((x + 1 - Width / 2) * CellSize,
				                              (y + 1 - Height / 2) * CellSize,
				                              1.f);

				const auto p1 = glm::mix(p0, p2, (Threshold - v0) / (v1 - v0));
				const auto p3 = glm::mix(p0, p6, (Threshold - v0) / (v3 - v0));
				const auto p5 = glm::mix(p2, p8, (Threshold - v1) / (v2 - v1));
				const auto p7 = glm::mix(p6, p8, (Threshold - v3) / (v2 - v3));

				// const auto p4 = 0.5f * (p0 + p8);

				switch (mask) {
				case 0b1111:
					polys.push_back({p0, p2, p8, p6});
					break;
				case 0b0001:
					polys.push_back({p0, p1, p3});
					break;
				case 0b0010:
					polys.push_back({p1, p2, p5});
					break;
				case 0b0100:
					polys.push_back({p5, p8, p7});
					break;
				case 0b1000:
					polys.push_back({p3, p7, p6});
					break;
				case 0b0011:
					polys.push_back({p0, p2, p5, p3});
					break;
				case 0b1100:
					polys.push_back({p3, p5, p8, p6});
					break;
				case 0b0110:
					polys.push_back({p1, p2, p8, p7});
					break;
				case 0b1001:
					polys.push_back({p0, p1, p7, p6});
					break;

				case 0b0111:
					polys.push_back({p0, p2, p8, p7, p3});
					break;
				case 0b1110:
					polys.push_back({p1, p2, p8, p6, p3});
					break;
				case 0b1101:
					polys.push_back({p0, p1, p5, p8, p6});
					break;
				case 0b1011:
					polys.push_back({p0, p2, p5, p7, p6});
					break;

				case 0b0101:
					polys.push_back({p0, p1, p5, p8, p7, p3});
					break;
				case 0b1010:
					polys.push_back({p1, p2, p5, p7, p6, p3});
					break;
				}
			}
		}

		for (auto& [start, end] : lines) {
			draw_list->AddLine(ImVec2(start.x, start.y),
			                   ImVec2(end.x, end.y),
			                   0xffffff66,
			                   1.f);
		}
		std::vector<ImVec2> pts;
		for (auto& verts : polys) {
			pts.clear();
			for (int i = 0; i < verts.size(); ++i) {
				pts.emplace_back(verts[i].x, verts[i].y);
			}
			draw_list->AddConvexPolyFilled(
			    pts.data(), static_cast<int>(pts.size()), 0xffffff66);
			draw_list->AddPolyline(
			    pts.data(), static_cast<int>(pts.size()), 0xffffff66, 0, 1.f);
		}
	}

	void debug() {
		if (ImGui::Begin("Marching Squares Debug")) {
			ImGui::DragFloat("Smoothness", &smoothness, 0.1f, 0.f, 100.f);
		}
		ImGui::End();
	}

private:
	std::vector<Circle> circles;
	std::vector<float>  points;

	static constexpr int CellSize = 10;
	static constexpr int Width    = 100;
	static constexpr int Height   = 100;
	static constexpr int Stride   = Width + 1;

	float smoothness = 50.f;
};