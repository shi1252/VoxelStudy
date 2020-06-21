#include "VoxelOctree.h"
#include "MathHelper.h"
#include "CameraClass.h"
#include <algorithm>

using namespace MathHelper;

VoxelOctree::VoxelOctree(XMFLOAT3 position, UINT chunkSize, float cellSize)
	: position(position), chunkSize(chunkSize), cellSize(cellSize)
{
	root = this;
	data = new std::vector<VoxelVertex>(chunkSize * chunkSize * chunkSize);
	std::fill(data->begin(), data->end(), VoxelVertex());
	parent = nullptr;
	childs = nullptr;
}

VoxelOctree::VoxelOctree(std::vector<VoxelVertex>* data, XMFLOAT3 position, UINT chunkSize, float cellSize, VoxelOctree* root, VoxelOctree* parent)
	: data(data), position(position), chunkSize(chunkSize), cellSize(cellSize), root(root), parent(parent)
{
}

VoxelOctree::~VoxelOctree()
{
	ReleaseChilds();
}

void VoxelOctree::SetVoxelVertex(XMFLOAT3& pos, float threshold)
{
	XMUINT3 p = WorldPositionToLocalIndex(pos);
	SetVoxelVertex(p, threshold);
}

void VoxelOctree::SetVoxelVertex(XMUINT3& pos, float threshold)
{
	if (pos.x >= root->chunkSize || pos.y >= root->chunkSize || pos.z >= root->chunkSize)
		return;

	bool changed = (data->at(VertexPositionToArrayKey(pos)).threshold > 0.f) ^ (threshold > 0.f);
	data->at(VertexPositionToArrayKey(pos)).threshold = threshold;
	if (changed)
	{
		bool isInside = data->at(VertexPositionToArrayKey(pos)).threshold > 0.f;
		root->Insert(pos, isInside)->Merge();
	}
}

VoxelOctree* VoxelOctree::Insert(XMUINT3& pos, bool isInside)
{
	if (chunkSize == 1)
	{
		this->isInside = isInside;
		return this;
	}

	UINT half = chunkSize / 2;
	XMUINT3 curr = WorldPositionToLocalIndex(position);
	if (pos.y >= curr.y + half)
	{
		if (pos.z < curr.z + half)
		{
			if (pos.x < curr.x + half)
			{
				if (childs[NBW] == nullptr)
				{
					XMFLOAT3 p = XMFLOAT3(0.f, half, 0.f);
					childs[NBW] = new VoxelOctree(data, position + p, half, cellSize, root, this);
				}
				return childs[NBW]->Insert(pos, isInside);
			}
			else
			{
				if (childs[NBE] == nullptr)
				{
					XMFLOAT3 p = XMFLOAT3(half, half, 0.f);
					childs[NBE] = new VoxelOctree(data, position + p, half, cellSize, root, this);
				}
				return childs[NBE]->Insert(pos, isInside);
			}
		}
		else
		{
			if (pos.x < curr.x + half)
			{
				if (childs[NFW] == nullptr)
				{
					XMFLOAT3 p = XMFLOAT3(0.f, half, half);
					childs[NFW] = new VoxelOctree(data, position + p, half, cellSize, root, this);
				}
				return childs[NFW]->Insert(pos, isInside);
			}
			else
			{
				if (childs[NFE] == nullptr)
				{
					XMFLOAT3 p = XMFLOAT3(half, half, half);
					childs[NFE] = new VoxelOctree(data, position + p, half, cellSize, root, this);
				}
				return childs[NFE]->Insert(pos, isInside);
			}
		}
	}
	else
	{
		if (pos.z < curr.z + half)
		{
			if (pos.x < curr.x + half)
			{
				if (childs[SBW] == nullptr)
				{
					XMFLOAT3 p = XMFLOAT3(0.f, 0.f, 0.f);
					childs[SBW] = new VoxelOctree(data, position + p, half, cellSize, root, this);
				}
				return childs[SBW]->Insert(pos, isInside);
			}
			else
			{
				if (childs[SBE] == nullptr)
				{
					XMFLOAT3 p = XMFLOAT3(half, 0.f, 0.f);
					childs[SBE] = new VoxelOctree(data, position + p, half, cellSize, root, this);
				}
				return childs[SBE]->Insert(pos, isInside);
			}
		}
		else
		{
			if (pos.x < curr.x + half)
			{
				if (childs[SFW] == nullptr)
				{
					XMFLOAT3 p = XMFLOAT3(0.f, 0.f, half);
					childs[SFW] = new VoxelOctree(data, position + p, half, cellSize, root, this);
				}
				return childs[SFW]->Insert(pos, isInside);
			}
			else
			{
				if (childs[SFE] == nullptr)
				{
					XMFLOAT3 p = XMFLOAT3(half, 0.f, half);
					childs[SFE] = new VoxelOctree(data, position + p, half, cellSize, root, this);
				}
				return childs[SFE]->Insert(pos, isInside);
			}
		}
	}
}

void VoxelOctree::Merge()
{
	if (!childs[0])
		return;

	bool state = childs[0]->isInside;
	for (int i = 1; i < 8; ++i)
	{
		if (!childs[i] || childs[i]->isInside != state)
			return;
	}

	isInside = state;
	ReleaseChilds();
	parent->Merge();
}

XMFLOAT3 VoxelOctree::GetCenter()
{
	return position + XMFLOAT3(chunkSize * cellSize * 0.5f, chunkSize * cellSize * 0.5f, chunkSize * cellSize * 0.5f);
}

UINT VoxelOctree::VertexPositionToArrayKey(const XMUINT3& pos)
{
	return (pos.x + pos.y * chunkSize + pos.z * chunkSize * chunkSize);
}

XMUINT3 VoxelOctree::WorldPositionToLocalIndex(const XMFLOAT3& position)
{
	if (IsRoot())
	{
		if ((position.x < this->position.x || (position.x >= (this->position.x + chunkSize * cellSize)))
			|| (position.y < this->position.y || (position.y >= (this->position.y + chunkSize * cellSize)))
			|| (position.z < this->position.z || (position.z >= (this->position.z + chunkSize * cellSize))))
			return XMUINT3(-1, -1, -1);

		return XMUINT3((position.x - this->position.x) / cellSize,
			(position.y - this->position.y) / cellSize,
			(position.z - this->position.z) / cellSize);
	}
	else
	{
		if ((position.x < root->position.x || (position.x >= (root->position.x + chunkSize * cellSize)))
			|| (position.y < root->position.y || (position.y >= (root->position.y + chunkSize * cellSize)))
			|| (position.z < root->position.z || (position.z >= (root->position.z + chunkSize * cellSize))))
			return XMUINT3(-1, -1, -1);

		return XMUINT3((position.x - root->position.x) / cellSize,
			(position.y - root->position.y) / cellSize,
			(position.z - root->position.z) / cellSize);
	}
}

bool VoxelOctree::IsRoot() const
{
	if (root == this)
		return true;
	else
		return false;
}

//void VoxelOctree::GetVoxelIA(std::vector<Voxel::VertexType>& vertices, std::vector<int>& indices)
//{
//	float distance = Distance(CameraClass::mainCam->GetPosition(), GetCenter());
//	int lodLevel = distance / LOD_DISTANCE;
//	if (chunkSize == lodLevel)
//	{
//		XMUINT3 index = WorldPositionToLocalIndex(position);
//	}
//
//	if (childs)
//	{
//
//	}
//}

void VoxelOctree::ReleaseChilds()
{
	if (childs)
	{
		for (int i = 0; i < 8; ++i)
		{
			if (childs[i])
			{
				delete[] childs[i];
				childs[i] = nullptr;
			}
		}
		delete[] childs;
		childs = nullptr;
	}
}
