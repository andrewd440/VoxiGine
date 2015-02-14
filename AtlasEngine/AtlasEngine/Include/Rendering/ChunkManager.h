#pragma once

#include <vector>
#include <queue>

#include "Chunk.h"
#include "ShaderProgram.h"
#include "UniformBlockStandard.h"
#include "noise\noise.h"
#include "noise\noiseutils.h"
#include "Singleton.h"

class FChunkManager : public TSingleton<FChunkManager>
{
public:
	static const uint32_t WORLD_SIZE;
	static const uint32_t VISIBILITY_DISTANCE;
	static const uint32_t CHUNKS_TO_LOAD_PER_FRAME;

public:
	FChunkManager();
	~FChunkManager();

	void Setup();

	void Update();

	void Render();

	float GetNoiseHeight(uint32_t x, uint32_t z);

private:
	void UpdateUnloadList();
	void UpdateLoadList();
	void UpdateRebuildList();
	void UpdateVisibleList();
	void UpdateRenderList();

private:
	std::vector<FChunk> mChunks;        // All world chunks
	std::vector<uint32_t> mVisibleList; // Index list of potentially visible chunks
	std::vector<uint32_t> mRenderList;  // Index list of chunks to render
	std::vector<uint32_t> mIsLoadedList;  // Index list of all chunks that are currently loaded.
	std::vector<uint32_t> mLoadList;     // Index list of chunks to be loaded
	std::vector<uint32_t> mUnloadList;   // Index list of chunks to be unloaded
	std::vector<uint32_t> mRebuildList;  // Index list of chunks to be rebuilt

	// Rendering data
	FShaderProgram mShader;
	Vector3ui mLastCameraChunk;
	Vector3f mLastCameraPosition;
	Vector3f mLastCameraDirection;
	noise::utils::NoiseMap mNoiseMap;
};
