#pragma once
#include "Defined.h"
#include "Voxel.h"

struct VoxelOctree
{
	enum Section
	{
		NBW = 0,// North Back West
		NBE,
		NFW,
		NFE,
		SBW,// South Back West
		SBE,
		SFW,
		SFE,
		SECTION_COUNT
	};

	XMFLOAT3 position;
	UINT chunkSize;
	float cellSize;
	bool isInside = false;
	std::vector<VoxelVertex> *data = nullptr;
	VoxelOctree* root = nullptr;
	VoxelOctree* parent = nullptr;
	VoxelOctree** childs = nullptr;

	VoxelOctree(XMFLOAT3 position = XMFLOAT3(0.f, 0.f, 0.f), UINT chunkSize = 32, float cellSize = 1.f);
	VoxelOctree(std::vector<VoxelVertex>* data, XMFLOAT3 position, UINT chunkSize, float cellSize, VoxelOctree* root, VoxelOctree* parent);
	~VoxelOctree();

	void SetVoxelVertex(XMFLOAT3& pos, float threshold = 0.5f);
	void SetVoxelVertex(XMUINT3& pos, float threshold = 0.5f);
	
	VoxelOctree* Insert(XMUINT3& pos, bool isInside);
	void Merge();
	XMFLOAT3 GetCenter();
	UINT VertexPositionToArrayKey(const XMUINT3& pos);
	XMUINT3 WorldPositionToLocalIndex(const XMFLOAT3& position);
	bool IsRoot() const;

	//void GetVoxelIA(std::vector<Voxel::VertexType>& vertices, std::vector<int>& indices);

	void ReleaseChilds();
};