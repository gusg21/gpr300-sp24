#include "world.h"
#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <ew/procGen.h>

World::World() : monkeyModel(ew::Model("assets/Suzanne.fbx")) { // Hate that only this constructor has to go in the initializer list because the default constructor is deleted
	this->monkeyTransform = ew::Transform();

	this->planeModel = ew::Mesh(ew::createPlane(300, 300, 5));
	this->planeTransform = ew::Transform();
	this->planeTransform.position.y = -1.2f;

	this->time = 0.f;

	this->texture = ew::loadTexture("assets/roof_color.png");
}

void World::update(float deltaTime) {
	this->monkeyTransform.rotation = glm::rotate(this->monkeyTransform.rotation, deltaTime * 0.5f, glm::vec3(0.0, 1.0, 0.0));
	this->time += deltaTime;
}

void World::draw(ew::Shader& shader, ew::Camera& camera) {
	shader.use();
	shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());

	glBindTextureUnit(0, this->texture);

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