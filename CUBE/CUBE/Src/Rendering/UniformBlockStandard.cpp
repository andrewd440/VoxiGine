#include "Rendering\UniformBlockStandard.h"
#include "Rendering\GLUtils.h"


void FUniformBlock::GetBlockSize(const char* BlockName, GLint& SizeOut)
{
	// Get the current shader program and block index to use for retrieving block info
	GLint Program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &Program);

	GLuint BlockIndex = glGetUniformBlockIndex(Program, BlockName);

	glGetActiveUniformBlockiv(Program, BlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &SizeOut);
}

FUniformBlock::FUniformBlock(const char* BlockName, GLuint BindingIndexToSet, const uint32_t BlockSize)
	: mBufferID()
{
	GLint Program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &Program);

	GLuint BlockIndex = glGetUniformBlockIndex(Program, BlockName);
	glUniformBlockBinding(Program, BlockIndex, BindingIndexToSet);

	glGenBuffers(1, &mBufferID);
	glBindBuffer(GL_UNIFORM_BUFFER, mBufferID);
	glBufferData(GL_UNIFORM_BUFFER, BlockSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, BindingIndexToSet, mBufferID);
}

FUniformBlock::FUniformBlock(const uint32_t BindingIndex, const uint32_t BlockSize)
	: mBufferID()
{
	glGenBuffers(1, &mBufferID);
	glBindBuffer(GL_UNIFORM_BUFFER, mBufferID);
	glBufferData(GL_UNIFORM_BUFFER, BlockSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, BindingIndex, mBufferID);
}

FUniformBlock::~FUniformBlock()
{
	glDeleteBuffers(1, &mBufferID);
}

void FUniformBlock::SetData(const uint32_t DataOffset, const uint8_t* Data, const uint32_t DataSize)
{
	glBindBuffer(GL_UNIFORM_BUFFER, mBufferID);
	glBufferSubData(GL_UNIFORM_BUFFER, DataOffset, DataSize, Data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void* FUniformBlock::MapBuffer(GLenum Access) const
{
	glBindBuffer(GL_UNIFORM_BUFFER, mBufferID);
	return glMapBuffer(GL_UNIFORM_BUFFER, Access);
}
