#pragma once

#include <ew/shader.h>
#include <ew/model.h>
#include <ew/transform.h>
#include <ew/camera.h>
#include <ew/texture.h>

class World {
private:
	float time;

public:
	World();

	void update(float deltaTime);
	void draw(ew::Shader& shader, ew::Camera& camera);

	uint32_t texture;

	ew::Model monkeyModel;
	ew::Transform monkeyTransform;

	ew::Mesh planeModel;
	ew::Transform planeTransform;
};