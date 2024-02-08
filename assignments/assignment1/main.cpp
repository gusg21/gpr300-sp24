#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>

#include <gg/framebuffer.h>

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
void drawUI(ew::Camera* cam, ew::CameraController* camCtrl);
void resetCamera(ew::Camera* camera, ew::CameraController* controller);

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

float dofIntensity = 200.f, dofBlur = 20.0f, dofOffset = 0.55f, fogPower = 500.f;

ew::Transform monkeyTransform;

ew::Camera camera;
ew::CameraController cameraController;

int main() {
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader ppShader = ew::Shader("assets/blur.vert", "assets/blur.frag");
	GLuint brickTexture = ew::loadTexture("assets/roof_color.png");
	//GLuint brickNormalMapTexture = ew::loadTexture("assets/roof_normal.png");
	ew::Model monkeyModel = ew::Model("assets/Suzanne.fbx");
	gg::fb::framebuffer fb = gg::fb::Create(screenWidth, screenHeight, GL_RGBA8);
	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	printf("Framebuffer status: %d", fboStatus);

	shader.use();
	shader.setInt("_MainTex", 0);
	
	unsigned int dummyVAO;
	glCreateVertexArrays(1, &dummyVAO);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		gg::fb::Bind(&fb);
		glViewport(0, 0, fb.width, fb.height);
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST); //Depth testing

		glBindTextureUnit(0, brickTexture);

		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime * 0.5f, glm::vec3(0.0, 1.0, 0.0));

		shader.use();
		shader.setInt("_MainTex", 0);
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shader.setVec3("_EyePos", camera.position);
		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);

		monkeyModel.draw();

		cameraController.move(window, &camera, deltaTime);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

		drawUI(&camera, &cameraController);

		glfwSwapBuffers(window);
	}

	glDeleteFramebuffers(1, &fb.fbo);

	printf("Shutting down...");
}

void drawUI(ew::Camera *cam, ew::CameraController *camCtrl) {
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
	if (ImGui::CollapsingHeader("Effect")) {
		ImGui::SliderFloat("DOF Intensity", &dofIntensity, 0.0f, 500.0f);
		ImGui::SliderFloat("DOF Offset", &dofOffset, 0.0f, 1.0f);
		ImGui::SliderFloat("DOF Blur", &dofBlur, 0.0f, 50.0f);
		ImGui::SliderFloat("Fog Power", &fogPower, 1.0f, 1000.f);
	}

	//Add more camera settings here!
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

