#include "shader.h"

#include <fstream>
#include <array>

namespace PBRLookDev
{
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex") return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel") return GL_FRAGMENT_SHADER;

		ASSERT(false);
		return 0;
	}
	
	std::string OpenglShader::ReadFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream fin;
		fin.open(filepath, std::ios::in | std::ios::binary);

		if (fin)
		{
			fin.seekg(0, std::ios::end);
			size_t size = fin.tellg();
			if (size != -1)
			{
				result.resize(size);
				fin.seekg(0, std::ios::beg);
				fin.read(&result[0], size);
				fin.close();
			}
			else
			{
				printf("Could not read from file %s\n", filepath.c_str());
				ASSERT(false);
			}
		}
		else
		{
			printf("Could not read from file %s\n", filepath.c_str());
			ASSERT(false);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenglShader::PreProcess(const std::string& program)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = program.find(typeToken);
		while (pos != std::string::npos)
		{
			size_t eol = program.find_first_of("\r\n", pos);
			ASSERT(eol != std::string::npos); // syntax error
			size_t begin = pos + typeTokenLength + 1;
			std::string type = program.substr(begin, eol - begin);
			ASSERT(ShaderTypeFromString(type) != 0);// Invaild shader type specified!;

			size_t nextLinePos = program.find_first_not_of("\r\n", eol);
			pos = program.find(typeToken, nextLinePos);
			shaderSources[ShaderTypeFromString(type)] =
				program.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? program.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void OpenglShader::Compile(std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();
		ASSERT(shaderSources.size() <= 2);// "Over Max. Number of Shaders!");
		std::array<GLenum, 2> glShaderIDs;

		int glShadreIDIndex = 0;

		for (auto& key : shaderSources)
		{
			GLenum shaderType = key.first;
			const std::string& source = key.second;

			GLuint shader = glCreateShader(shaderType);

			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			// Compile the vertex shader
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				// We don't need the shader anymore.
				glDeleteShader(shader);

				printf("Shader compilation failure! Error: %s\n", infoLog.data());
				break;
			}

			glAttachShader(program, shader);
			glShaderIDs[glShadreIDIndex++] = shader;
		}
		// Link program
		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);
			// Don't leak shaders either.
			for (auto id : glShaderIDs) glDeleteShader(id);

			printf("Shader Link failure! Error: %s\n", infoLog.data());
			return;
		}

		// Always detach shaders after a successful link.
		for (auto id : glShaderIDs) glDetachShader(program, id);

		m_RendererID = program;
	}

	OpenglShader::OpenglShader(const std::string& filepath)
	{
		std::string program = ReadFile(filepath);
		auto shaderSources = PreProcess(program);
		Compile(shaderSources);

		// find name
		size_t lastSlash = filepath.find_last_of("/\\");
		lastSlash = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
		size_t lastDot = filepath.rfind('.');
		size_t count = (lastDot == std::string::npos) ? filepath.size() - lastSlash : lastDot - lastSlash;

		m_Name = filepath.substr(lastSlash, count);
	}

	OpenglShader::OpenglShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath)
		:m_Name(name)
	{
		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = ReadFile(vertexPath);
		sources[GL_FRAGMENT_SHADER] = ReadFile(fragmentPath);
		Compile(sources);
	}

	OpenglShader::~OpenglShader()
	{
		glDeleteProgram(m_RendererID);
	}

	void OpenglShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}
	void OpenglShader::Unbind() const
	{
		glUseProgram(0);
	}
	void OpenglShader::SetUniformInt(const std::string& name, int value)
	{
		UseMe();
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		ASSERT(location != -1);
		glUniform1i(location, value);
	}
	void OpenglShader::SetUniformMat4(const std::string& name, const glm::mat4& value)
	{
		UseMe();
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		ASSERT(location != -1);
		glUniformMatrix4fv(location, 1, GL_FALSE, &(value[0][0]));
	}
	void OpenglShader::SetUniformFloat(const std::string& name, float value)
	{
		UseMe();
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		ASSERT(location != -1);
		glUniform1f(location, value);
	}
	void OpenglShader::SetUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		UseMe();
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		ASSERT(location != -1);
		glUniform2f(location, value.x, value.y);
	}
	void OpenglShader::SetUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		UseMe();
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		ASSERT(location != -1);
		glUniform3f(location, value.x, value.y, value.z);
	}
	void OpenglShader::SetUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		UseMe();
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		ASSERT(location != -1);
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	uPtr<OpenglShader> OpenglShader::Create(const std::string& filepath)
	{
		return mkU<OpenglShader>(filepath);
	}
	uPtr<OpenglShader> OpenglShader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		return mkU<OpenglShader>(name, vertexSrc, fragmentSrc);;
	}
}
