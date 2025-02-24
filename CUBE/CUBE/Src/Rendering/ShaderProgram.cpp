#include "Rendering\ShaderProgram.h"
#include "SystemResources\SystemFile.h"
#include "Debugging\ConsoleOutput.h"

#include <GL\glew.h>
#include <GL\GL.h>
#include "SFML\Window\Context.hpp"
#include <cstdint>
#include <vector>

/////////////////////////
//// FShader ////////////

FShader::FShader(const wchar_t* SourceFile, GLenum ShaderType)
	: mID()
	, mType(ShaderType)
{
	mID = glCreateShader(ShaderType);

	const std::string ShaderSource = ReadShader(SourceFile);
	const char* SourcePtr = ShaderSource.c_str();
	glShaderSource(mID, 1, &SourcePtr, nullptr);

	glCompileShader(mID);

#ifndef NDEBUG
	CheckShaderErrors(mID);
#endif
}

FShader::~FShader()
{
	glDeleteShader(mID);
}

GLuint FShader::GetID() const
{
	return mID;
}

GLenum FShader::GetType() const
{
	return mType;
}

std::string FShader::ReadShader(const wchar_t* SourceFile) const
{
	IFileSystem& FileSystem = IFileSystem::GetInstance();
	auto ShaderFile = FileSystem.OpenReadable(SourceFile);

	if (ShaderFile)
	{
		const uint32_t ShaderSize = ShaderFile->GetFileSize();

		std::string ShaderSource;
		ShaderSource.resize(ShaderSize);
		ShaderFile->Read((uint8_t*)ShaderSource.data(), ShaderSize);

		// Parse #includes and add source data
		size_t StringHead = 0;
		size_t FirstChar = ShaderSource.find("#include", StringHead);
		while (FirstChar != std::string::npos)
		{
			// Get the file to include, then delete that line
			std::size_t FileStart = ShaderSource.find('"', FirstChar);
			std::size_t FileEnd = ShaderSource.find('"', FileStart + 1);
			std::string IncludeFile = ShaderSource.substr(FileStart + 1, FileEnd - FileStart - 1);
			ShaderSource.erase(ShaderSource.begin() + FirstChar, ShaderSource.begin() + FileEnd + 1);

			// Read the included shader and insert it in the #include position
			std::wstring WIncludeFile{ IncludeFile.begin(), IncludeFile.end() };
			WIncludeFile.insert(0, L"Shaders/");
			std::string IncludeSource = ReadShader(WIncludeFile.data());
			ShaderSource.insert(FirstChar, IncludeSource, 0, std::string::npos);

			// Update string head to past the included file
			StringHead += FirstChar + IncludeSource.length();
			FirstChar = ShaderSource.find("#include", StringHead);
		}
		return ShaderSource;
	}

	return std::string();
}


#ifndef NDEBUG
void FShader::CheckShaderErrors(GLuint Shader) const
{
	// Retrieve the status of the shader
	GLint Success = 0;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &Success);

	if (Success == GL_TRUE)
		return;

	// If here, we have an error, so retrieves it's info
	GLint LogSize = 0;
	glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogSize);

	// Get the info log and pring to debug output
	std::vector<char> LogInfo(LogSize);
	glGetShaderInfoLog(Shader, LogSize, nullptr, &LogInfo[0]);
	FDebug::PrintF(&LogInfo[0]);
}
#endif

/////////////////////////
//// FShaderProgram /////

FShaderProgram::FShaderProgram()
	:mID(glCreateProgram())
	, mUniforms()
{

}

FShaderProgram::FShaderProgram(const std::initializer_list<const FShader*> Shaders)
	: mID(glCreateProgram())
{
	for (auto Itr = Shaders.begin(); Itr != Shaders.end(); Itr++)
	{
		AttachShader(*(*Itr));
	}

	LinkProgram();
}

FShaderProgram::~FShaderProgram()
{
	glDeleteProgram(mID);
}

void FShaderProgram::AttachShader(const FShader& Shader)
{
	glAttachShader(mID, Shader.GetID());
}

void FShaderProgram::LinkProgram()
{
	glLinkProgram(mID);

	// Detach all attached shaders
	GLsizei ShaderCount;
	glGetProgramiv(mID, GL_ATTACHED_SHADERS, &ShaderCount);

	std::vector<GLuint> AttachedShaders(ShaderCount);
	glGetAttachedShaders(mID, ShaderCount, nullptr, &AttachedShaders[0]);

	for (GLsizei i = 0; i < ShaderCount; i++)
	{
		glDetachShader(mID, AttachedShaders[i]);
	}

#ifndef NDEBUG
	CheckProgramErrors();
#endif
}

#ifndef NDEBUG
void FShaderProgram::CheckProgramErrors()
{
	// Retrieve the status of the program
	GLint IsLinked = 0;
	glGetProgramiv(mID, GL_LINK_STATUS, &IsLinked);

	if (IsLinked == GLU_TRUE)
		return;

	// If here, we have an error so retrieve and print it's info
	GLint LogSize = 0;
	glGetProgramiv(mID, GL_INFO_LOG_LENGTH, &LogSize);

	std::vector<char> LogInfo(LogSize);
	glGetProgramInfoLog(mID, LogSize, nullptr, &LogInfo[0]);
	FDebug::PrintF(&LogInfo[0]);
}
#endif

FUniform& FShaderProgram::GetUniform(const char* Name)
{
	auto Found = mUniforms.find(Name);
	if (Found != mUniforms.end())
	{
		return Found->second;
	}
	else
	{
		Use();
		mUniforms[Name].Bind(mID, Name);
		return mUniforms[Name];
	}
}