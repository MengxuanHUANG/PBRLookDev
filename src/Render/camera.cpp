#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace PBRLookDev
{
	PerspectiveCamera::PerspectiveCamera(unsigned int w,
										 unsigned int h,
										 float fovy,
										 const glm::vec3& pos,
										 const glm::vec3& ref,
										 const glm::vec3& worldUp)
		:width(w),
		 height(h),
		 fovy(fovy),
		 position(pos),
		 ref(ref),
		 worldUp(worldUp)
	{
		RecomputeAttributes();
	}

	PerspectiveCamera::PerspectiveCamera(const PerspectiveCamera& c)
		:width(c.width),
		 height(c.height),
	 	 fovy(c.fovy),
		 position(c.position),
		 ref(c.ref),
		worldUp(c.worldUp)
	{
		RecomputeAttributes();
	}

	void PerspectiveCamera::RecomputeAttributes()
	{
		// recompute right, up
		forward = glm::normalize(ref - position);
		right = glm::normalize(glm::cross(forward, worldUp));
		up = glm::normalize(glm::cross(right, forward));
	}

	glm::mat4 PerspectiveCamera::GetViewProj() 
	{
		return glm::perspective(glm::radians(fovy), width / (float)height, 0.1f, 100.f) * glm::lookAt(position, ref, up);
	}
}