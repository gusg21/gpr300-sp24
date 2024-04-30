#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <ew/procGen.h>

#include <gg/framebuffer.h>
#include "world.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;


void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI(ew::Camera* cam, ew::CameraController* camCtrl, gg::FrameBuffer* buffers, const gg::FrameBuffer& shadowFb);
void resetCamera(ew::Camera* camera, ew::CameraController* controller);

// Lights defs
constexpr uint32_t NUM_LIGHTS = 64;
glm::vec3 lightDirection = glm::vec3(0.f, -1.f, 0.f);

typedef struct light {
	glm::vec3 pos;
	float radius;
	glm::vec3 color;
} light_t;

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

float dofIntensity = 200.f, dofBlur = 20.0f, dofOffset = 0.55f, fogPower = 500.f;

light_t lights[NUM_LIGHTS];

ew::Camera camera;
ew::Camera shadowCamera;
ew::CameraController cameraController;

float float_rand(float min, float max)
{
	float norm = rand() / (float)RAND_MAX;
	return min + norm * (max - min);
}

int main() {
	GLFWwindow* window = initWindow("Assignment 3", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees

	shadowCamera.orthographic = true;
	shadowCamera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	shadowCamera.nearPlane = 0.5;
	shadowCamera.farPlane = 30;
	shadowCamera.orthoHeight = 80.0;
	shadowCamera.aspectRatio = 1.0;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	ew::Shader geomShader = ew::Shader("assets/geometry.vert", "assets/geometry.frag");
	ew::Shader shadowShader = ew::Shader("assets/shadow.vert", "assets/shadow.frag");
	ew::Shader deferredShader = ew::Shader("assets/deferred.vert", "assets/deferred.frag");
	ew::Shader orbShader = ew::Shader("assets/lightOrb.vert", "assets/lightOrb.frag");

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		lights[i].pos = glm::vec3((i % 8) * 4, 2.0f, i / 8 * 6);
		lights[i].radius = rand() % 10 + 5;
		lights[i].color = glm::vec3(float_rand(0.f, 1.f), float_rand(0.f, 1.f), float_rand(0.f, 1.f));
	}


	gg::FrameBuffer shadow_fb = gg::CreateFrameBufferDepthOnly(2048, 2048);
	gg::FrameBuffer gb = gg::CreateGBuffer(screenWidth, screenHeight);

	World* world = new World();

	ew::Mesh sphereMesh = ew::Mesh(ew::createSphere(1.0f, 8));

	unsigned int dummyVAO;
	glCreateVertexArrays(1, &dummyVAO);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &camera, deltaTime);
		world->update(deltaTime);

		{
			glBindFramebuffer(GL_FRAMEBUFFER, gb.fbo);
			glViewport(0, 0, gb.width, gb.height);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			world->draw(geomShader, camera);
		}

		{
			//RENDER
			glBindFramebuffer(GL_FRAMEBUFFER, shadow_fb.fbo);
			glViewport(0, 0, shadow_fb.width, shadow_fb.height);
			glClear(GL_DEPTH_BUFFER_BIT);

			//glCullFace(GL_FRONT);

			shadowCamera.position = shadowCamera.target - lightDirection * 5.0f;
			world->draw(shadowShader, shadowCamera);

			//glCullFace(GL_BACK);
		}

		{
			glBindFramebuffer(GL_FRAMEBUFFER, gb.fbo);
			glViewport(0, 0, gb.width, gb.height);

			orbShader.use();
			orbShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());

			for (int i = 0; i < NUM_LIGHTS; i++)
			{
				glm::mat4 m = glm::mat4(1.0f);
				m = glm::translate(m, lights[i].pos);
				m = glm::scale(m, glm::vec3(0.5f));

				orbShader.setMat4("_Model", m);
				orbShader.setVec3("_Color", lights[i].color);
				sphereMesh.draw();
			}
		}

		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, screenWidth, screenHeight);
			//glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			deferredShader.use();
			deferredShader.setMat4("_LightViewProj", shadowCamera.projectionMatrix() * shadowCamera.viewMatrix());
			deferredShader.setVec3("_EyePos", camera.position);
			deferredShader.setVec3("_LightDirection", glm::normalize(lightDirection));
			deferredShader.setVec3("_LightColor", glm::vec3(1.f));

			deferredShader.setFloat("_Material.Ka", material.Ka);
			deferredShader.setFloat("_Material.Kd", material.Kd);
			deferredShader.setFloat("_Material.Ks", material.Ks);
			deferredShader.setFloat("_Material.Shininess", material.Shininess);
			deferredShader.setFloat("_MinBias", 0.005);
			deferredShader.setFloat("_MaxBias", 0.015);

			for (int i = 0; i < NUM_LIGHTS; i++)
			{
				deferredShader.setVec3("_PointLights[" + std::to_string(i) + "].position", lights[i].pos);
				deferredShader.setFloat("_PointLights[" + std::to_string(i) + "].radius", lights[i].radius);
				deferredShader.setVec3("_PointLights[" + std::to_string(i) + "].color", lights[i].color);
			}

			glBindTextureUnit(0, gb.colorBuffer[0]);
			glBindTextureUnit(1, gb.colorBuffer[1]);
			glBindTextureUnit(2, gb.colorBuffer[2]);
			glBindTextureUnit(3, shadow_fb.depthBuffer);

			glBindVertexArray(dummyVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		{
			glBindFramebuffer(GL_READ_FRAMEBUFFER, gb.fbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

			glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

			orbShader.use();
			orbShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());

			for (int i = 0; i < NUM_LIGHTS; i++)
			{
				glm::mat4 m = glm::mat4(1.0f);
				m = glm::translate(m, lights[i].pos);
				m = glm::scale(m, glm::vec3(lights[i].radius / 5.0f));

				orbShader.setMat4("_Model", m);
				orbShader.setVec3("_Color", lights[i].color);
				sphereMesh.draw();
			}
		}

		/*{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glViewport(0, 0, screenWidth, screenHeight);
			glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			ppShader.use();
			ppShader.setInt("_ColorBuffer", 0);
			ppShader.setInt("_DepthBuffer", 1);
			ppShader.setVec2("_BufferSize", glm::vec2(fb.width, fb.height));
			ppShader.setFloat("_DofIntensity", dofIntensity);
			ppShader.setFloat("_DofOffset", dofOffset);
			ppShader.setFloat("_DofBlur", dofBlur);
			ppShader.setFloat("_FogPower", fogPower);

			glBindTextureUnit(0, fb.colorBuffer);
			glBindTextureUnit(1, fb.depthBuffer);
			glBindVertexArray(dummyVAO);
			glDrawArrays(GL_TRIANGLES, 0, 3);

		}*/

		drawUI(&camera, &cameraController, &gb, shadow_fb);

		glfwSwapBuffers(window);
	}

	delete world;

	printf("Shutting down...");
}

void drawUI(ew::Camera* cam, ew::CameraController* camCtrl, gg::FrameBuffer* buffers, const gg::FrameBuffer& shadowFb) {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	if (ImGui::Button("Reset Camera")) {
		resetCamera(cam, camCtrl);
	}
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}
	/*if (ImGui::CollapsingHeader("Effect")) {
		ImGui::SliderFloat("DOF Intensity", &dofIntensity, 0.0f, 500.0f);
		ImGui::SliderFloat("DOF Offset", &dofOffset, 0.0f, 1.0f);
		ImGui::SliderFloat("DOF Blur", &dofBlur, 0.0f, 50.0f);
		ImGui::SliderFloat("Fog Power", &fogPower, 1.0f, 1000.f);
	}*/

	//Add more camera settings here!
	ImGui::End();

	ImGui::Begin("ShadowMap");
	ImGui::BeginChild("ShadowMap");
	ImVec2 windowSize = ImGui::GetWindowSize();
	ImGui::Image((ImTextureID)shadowFb.depthBuffer, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndChild();
	ImGui::End();
	
	ImGui::Begin("GBuffers");
	ImGui::BeginChild("GBuffers");
	ImVec2 texSize = ImVec2(buffers->width / 4, buffers->height / 4);
	for (size_t i = 0; i < 3; i++)
	{
		ImGui::Image((ImTextureID)buffers->colorBuffer[i], texSize, ImVec2(0, 1), ImVec2(1, 0));
	}

	ImGui::EndChild();
	ImGui::End();


	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

