#pragma once

#include <glm/glm.hpp>

namespace PBRLookDev
{
	class PerspectiveCamera
	{
	public:
		PerspectiveCamera(unsigned int w, 
						  unsigned int h, 
						  float fovy = 45.f,
						  const glm::vec3& pos = glm::vec3(0, 0, -30),
						  const glm::vec3& ref = glm::vec3(0, 0, 0),
						  const glm::vec3& worldUp = glm::vec3(0, 1, 0));

		PerspectiveCamera(const PerspectiveCamera& c);

		void RecomputeAttributes();
		glm::mat4 GetViewProj();
	public:
		// perspective camera parameters
		unsigned int width, height;
		float fovy;

		float aspect;

		glm::vec3 position; // camera's position in world space
		glm::vec3 ref;

		glm::vec3 forward;
		glm::vec3 up;
		glm::vec3 right;
		glm::vec3 worldUp;
	};
}