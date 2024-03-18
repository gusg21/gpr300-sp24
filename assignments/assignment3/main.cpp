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
#include <gg/gbuffer.h>

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
void drawUI(ew::Camera* cam, ew::CameraController* camCtrl, gg::GBuffer* buffer);
void resetCamera(ew::Camera* camera, ew::CameraController* controller);

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

float dofIntensity = 200.f, dofBlur = 20.0f, dofOffset = 0.55f, fogPower = 500.f;

class World {
private:
	float time;

public:
	World();

	void update(float deltaTime);
	void draw(ew::Shader& shader);

	ew::Model monkeyModel;
	ew::Transform monkeyTransform;

	ew::Mesh planeModel;
	ew::Transform planeTransform;
};

World::World() : monkeyModel(ew::Model("assets/Suzanne.fbx")) { // Hate that only this constructor has to go in the initializer list because the default constructor is deleted
	this->monkeyTransform = ew::Transform();

	this->planeModel = ew::Mesh(ew::createPlane(300, 300, 5));
	this->planeTransform = ew::Transform();
	this->planeTransform.position.y = -1.2f;

	this->time = 0.f;
}

void World::update(float deltaTime) {
	this->monkeyTransform.rotation = glm::rotate(this->monkeyTransform.rotation, deltaTime * 0.5f, glm::vec3(0.0, 1.0, 0.0));
	this->time += deltaTime;
}

void World::draw(ew::Shader& shader) {
	ew::Transform newMonkeyTrans = ew::Transform(this->monkeyTransform);
	for (int32_t x = -10; x < 10; x++)
		for (int32_t y = -10; y < 10; y++)
		{
			newMonkeyTrans.position.x = x * 5;
			newMonkeyTrans.position.y = sinf((x * y) + this->time) + 1.f;
			newMonkeyTrans.position.z = y * 5;
			shader.setMat4("_Model", newMonkeyTrans.modelMatrix());
			this->monkeyModel.draw();
		}
	

	shader.setMat4("_Model", this->planeTransform.modelMatrix());
	this->planeModel.draw();
}

ew::Camera camera;
ew::CameraController cameraController;

int main() {
	GLFWwindow* window = initWindow("Assignment 2", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader ppShader = ew::Shader("assets/blur.vert", "assets/blur.frag");
	ew::Shader depthShader = ew::Shader("assets/depth.vert", "assets/depth.frag");
	ew::Shader geomShader = ew::Shader("assets/geometry.vert", "assets/geometry.frag");

	GLuint brickTexture = ew::loadTexture("assets/roof_color.png");

	World* world = new World();

	gg::FrameBuffer fb = gg::CreateFrameBuffer(screenWidth, screenHeight, GL_RGBA8);
	gg::FrameBuffer shadow_fb = gg::CreateFrameBufferDepthOnly(1000, 1000);

	gg::GBuffer* gb = new gg::GBuffer(screenWidth, screenHeight);

	ew::Camera light_cam;
	light_cam.position = glm::vec3(50.0f, 30.0f, 50.0f);
	light_cam.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	light_cam.aspectRatio = 1.0f;
	light_cam.farPlane = 30.f;
	light_cam.nearPlane = 0.f;
	light_cam.orthographic = true;

	litShader.use();
	litShader.setInt("_MainTex", 0);

	unsigned int dummyVAO;
	glCreateVertexArrays(1, &dummyVAO);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		world->update(deltaTime);
		cameraController.move(window, &camera, deltaTime);

		//{
		//	gg::BindFrameBuffer(&shadow_fb);
		//	glViewport(0, 0, shadow_fb.width, shadow_fb.height);
		//	glClear(GL_DEPTH_BUFFER_BIT);
		//	glEnable(GL_DEPTH_TEST); //Depth testing

		//	glBindTextureUnit(0, brickTexture);

		//	depthShader.use();
		//	depthShader.setMat4("_ViewProjection", light_cam.projectionMatrix() * light_cam.viewMatrix());

		//	world->draw(depthShader);
		//}

		gb->Begin();
		{
			glViewport(0, 0, fb.width, fb.height);
			glClearColor(0.f, 0.f, 0.f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST); //Depth testing

			glBindTextureUnit(0, brickTexture);
			glBindTextureUnit(1, shadow_fb.depthBuffer);

			litShader.use();
			litShader.setInt("_MainTex", 0);
			litShader.setInt("_ShadowMap", 1);
			litShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
			litShader.setMat4("_LightViewProj", light_cam.projectionMatrix() * light_cam.viewMatrix());

			litShader.setVec3("_EyePos", camera.position);
			litShader.setFloat("_Material.Ka", material.Ka);
			litShader.setFloat("_Material.Kd", material.Kd);
			litShader.setFloat("_Material.Ks", material.Ks);
			litShader.setFloat("_Material.Shininess", material.Shininess);

			world->draw(litShader);
		}
		gb->End();

		{
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

			drawUI(&camera, &cameraController, gb);
		}

		glfwSwapBuffers(window);
	}

	glDeleteFramebuffers(1, &fb.fbo);
	delete world;

	printf("Shutting down...");
}

void drawWorld(World* world, ew::Shader* shader) {
	shader->setMat4("_Model", world->monkeyTransform.modelMatrix());
	world->monkeyModel.draw();

	shader->setMat4("_Model", world->planeTransform.modelMatrix());
	world->planeModel.draw();
}

void drawUI(ew::Camera* cam, ew::CameraController* camCtrl, gg::GBuffer* buffer) {
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

	ImGui::Begin("Shadow Map");
	//Using a Child allow to fill all the space of the window.
	ImGui::BeginChild("Shadow Map");
	//Stretch image to be window size
	ImVec2 windowSize = ImGui::GetWindowSize();
	//Invert 0-1 V to flip vertically for ImGui display
	//shadowMap is the texture2D handle
	ImVec2 texSize = ImVec2(buffer->GetWidth() / 4, buffer->GetWidth() / 4);
	for (int i = 0; i < GBUFFER_COLOR_BUFFER_COUNT; i++) {
		ImGui::Image((ImTextureID)buffer->GetColorBuffers()[i], texSize, ImVec2(0, 1), ImVec2(1, 0));
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

