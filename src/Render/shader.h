#pragma

#include "Core/core.h"
#include <unordered_map>

#include "glm/glm.hpp"

#include "GL/glew.h"

namespace PBRLookDev
{
	class OpenglShader
	{
	public:
		OpenglShader(const std::string& filepath);
		OpenglShader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);
		~OpenglShader();

		void Bind() const;
		void Unbind() const;

		void SetUniformInt(const std::string& name, int value);
		void SetUniformMat4(const std::string& name, const glm::mat4& value);
		void SetUniformFloat(const std::string& name, float value);
		void SetUniformFloat3(const std::string& name, const glm::vec3& value);
		void SetUniformFloat4(const std::string& name, const glm::vec4& value);

		inline const std::string& GetName() const { return m_Name; }

		static uPtr<OpenglShader> Create(const std::string& filepath);
		static uPtr<OpenglShader> Create(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);

	private:
		static std::string ReadFile(const std::string& filepath);
		static std::unordered_map<GLenum, std::string> PreProcess(const std::string& program);
		void Compile(std::unordered_map<GLenum, std::string>&);

		inline void UseMe() { glUseProgram(m_RendererID); }

	public:
		uint32_t m_RendererID;
		std::string m_Name;
	};
}