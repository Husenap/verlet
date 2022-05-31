#include <dubu_opengl_app/dubu_opengl_app.hpp>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>

constexpr glm::ivec2 SIZE{2048, 2048};

class App : public dubu::opengl_app::AppBase {
public:
	App()
	    : dubu::opengl_app::AppBase({.appName = "Example App"}) {}
	virtual ~App() = default;

protected:
	virtual void Init() override {
		GLuint      shader       = glCreateShader(GL_COMPUTE_SHADER);
		const char* shaderSource = R"(
			#version 460

			layout(local_size_x = 1, local_size_y = 1) in;
			layout(rgba32f, binding = 0) uniform image2D img_output;

			layout(location = 0) uniform float offset;

			void main() {
				vec2 pixel_coords = gl_GlobalInvocationID.xy;
				vec4 pixel = vec4(mod(vec2(gl_GlobalInvocationID.xy) / 2048.0 + offset, 1.0), 0.0, 1.0);

				vec4 prev = imageLoad(img_output, ivec2(pixel_coords));

				imageStore(img_output, ivec2(pixel_coords), mix(prev, pixel, 0.01));
			}
		)";
		glShaderSource(shader, 1, &shaderSource, NULL);
		glCompileShader(shader);

		mProgram = glCreateProgram();
		glAttachShader(mProgram, shader);
		glLinkProgram(mProgram);

		glDeleteShader(shader);

		glGenTextures(1, &mImage);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mImage);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D,
		             0,
		             GL_RGBA32F,
		             SIZE.x,
		             SIZE.y,
		             0,
		             GL_RGBA,
		             GL_FLOAT,
		             NULL);
		glBindImageTexture(
		    0, mImage, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	}

	virtual void Update() override {
		glUseProgram(mProgram);
		glUniform1f(0, mOffset);
		glDispatchCompute(SIZE.x, SIZE.y, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		if (ImGui::Begin("Viewport")) {
			ImVec2 regionMin = ImGui::GetWindowContentRegionMin();
			ImVec2 regionMax = ImGui::GetWindowContentRegionMax();
			ImVec2 offset    = regionMin;
			ImVec2 regionSize =
			    ImVec2(regionMax.x - regionMin.x, regionMax.y - regionMin.y);
			ImVec2 imageSize = {static_cast<float>(SIZE.x),
			                    static_cast<float>(SIZE.y)};

			float regionRatio = regionSize.x / regionSize.y;
			float imageRatio =
			    static_cast<float>(SIZE.x) / static_cast<float>(SIZE.y);

			if (regionRatio > imageRatio) {
				imageSize.x *= regionSize.y / imageSize.y;
				imageSize.y = regionSize.y;
			} else {
				imageSize.y *= regionSize.x / imageSize.x;
				imageSize.x = regionSize.x;
			}

			ImGui::SetCursorPosX((regionSize.x - imageSize.x) * 0.5f +
			                     offset.x);
			ImGui::SetCursorPosY((regionSize.y - imageSize.y) * 0.5f +
			                     offset.y);

			ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(mImage)),
			             imageSize,
			             {0, 1},
			             {1, 0});
		}
		ImGui::End();

		if (ImGui::Begin("Parameters")) {
			ImGui::DragFloat("Offset", &mOffset, 0.01f);
		}
		ImGui::End();
	}

private:
	GLuint mProgram;
	GLuint mImage;
	float  mOffset = 0.f;
};

int main() {
	App app;

	app.Run();

	return 0;
}
