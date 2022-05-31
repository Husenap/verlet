#include <algorithm>

#include <dubu_opengl_app/dubu_opengl_app.hpp>
#include <glm/glm.hpp>
#include <imgui/imgui.h>

#include "solver.hpp"

class App : public dubu::opengl_app::AppBase {
public:
	App()
	    : dubu::opengl_app::AppBase({.appName = "Verlet Solver"}) {}
	virtual ~App() = default;

protected:
	virtual void Init() override { solver.addObject(); }

	virtual void Update() override {
		static float previousTime = static_cast<float>(glfwGetTime());
		static float time         = 0.f;
		const float  currentTime  = static_cast<float>(glfwGetTime());
		const float  deltaTime =
		    std::min(currentTime - previousTime, 1.f / 60.f);
		time += deltaTime;
		previousTime = currentTime;

		solver.update(deltaTime);

		ImGui::DockSpaceOverViewport();

		if (ImGui::Begin("Canvas")) {
			ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
			ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
			if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
			if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
			ImVec2 canvas_p1 =
			    ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

			static ImVec2 scrolling(canvas_sz.x * 0.5f, canvas_sz.y * 0.5f);

			// Draw border and background color
			ImGuiIO&    io        = ImGui::GetIO();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRectFilled(
			    canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
			draw_list->AddRect(
			    canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
			                    ImVec2(0,
			                           0));  // Disable padding
			ImGui::PushStyleColor(ImGuiCol_ChildBg,
			                      IM_COL32(50,
			                               50,
			                               50,
			                               255));  // Set a background color
			ImGui::BeginChild(
			    "canvas", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove);
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();

			// This will catch our interactions
			ImGui::InvisibleButton("canvas",
			                       canvas_sz,
			                       ImGuiButtonFlags_MouseButtonLeft |
			                           ImGuiButtonFlags_MouseButtonRight);
			const bool   is_hovered = ImGui::IsItemHovered();  // Hovered
			const bool   is_active  = ImGui::IsItemActive();   // Held
			const ImVec2 origin(
			    canvas_p0.x + scrolling.x,
			    canvas_p0.y + scrolling.y);  // Lock scrolled origin
			const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x,
			                                 io.MousePos.y - origin.y);

			if (is_hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
				solver.addObject();
			}

			// Pan (we use a zero mouse threshold when there's no context menu)
			// You may decide to make that threshold dynamic based on whether
			// the mouse is hovering something etc.
			const float mouse_threshold_for_pan = -1.0f;
			if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right,
			                                        mouse_threshold_for_pan)) {
				scrolling.x += io.MouseDelta.x;
				scrolling.y += io.MouseDelta.y;
			}

			// Context menu (under default mouse threshold)
			ImVec2 drag_delta =
			    ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) &&
			    drag_delta.x == 0.0f && drag_delta.y == 0.0f)
				ImGui::OpenPopupOnItemClick("context");
			if (ImGui::BeginPopup("context")) {
				if (ImGui::MenuItem("Remove all", NULL, false)) {
					solver.clear();
				}
				ImGui::EndPopup();
			}

			const float GRID_STEP = 50.0f;
			for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x;
			     x += GRID_STEP)
				draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y),
				                   ImVec2(canvas_p0.x + x, canvas_p1.y),
				                   IM_COL32(200, 200, 200, 40));
			for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y;
			     y += GRID_STEP)
				draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y),
				                   ImVec2(canvas_p1.x, canvas_p0.y + y),
				                   IM_COL32(200, 200, 200, 40));

			solver.apply([&](const VerletObject& o) {
				draw_list->AddCircleFilled(
				    ImVec2(origin.x + o.currentPosition.x,
				           origin.y + o.currentPosition.y),
				    o.radius,
				    o.color);
			});

			ImGui::EndChild();
		}
		ImGui::End();

		solver.debug();

		ImGui::ShowDemoWindow();
	}

private:
	Solver solver;
};

int main() {
	App app;

	app.Run();

	return 0;
}
