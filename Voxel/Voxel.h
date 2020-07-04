#pragma once
#include "Defined.h"
#include "MeshClass.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>

class Ray3D;
class ThreadPool;

struct VoxelVertex
{
	float threshold = -1.f;
};

enum VoxelType
{
	NONE,
	LAND
};

enum LOD
{
	LOD0 = 0,
	LOD1 = 1,
	LOD2 = 2,
	LOD3 = 4,
	LOD_COUNT,
	LOD_DISTANCE = 30
};

struct VoxelData
{
	VoxelType type = VoxelType::NONE;
};

struct VoxelNode
{
	VoxelNode() = default;
	VoxelNode(VoxelData* data, XMUINT3 index) : data(data), index(index) {}

	const VoxelData* data;
	XMUINT3 index;
	int parent = -1;
	//VoxelNode* parent = nullptr;
	bool visited = false;
	float gCost = 0;
	float hCost = 0;
	float fCost() const { return gCost + hCost; }

	int Compare(VoxelNode& node)
	{
		if (fCost() < node.fCost())
			return -1;
		else if (fabsf(fCost() - node.fCost()) < FLT_EPSILON)
		{
			if (gCost < node.gCost)
				return 0;
			return 1;
		}
		return 2;
	}

	friend bool operator==(const VoxelNode& lhs, const VoxelNode& rhs);

	bool operator==(const VoxelNode& rhs)
	{
		return (data == rhs.data);
	}
};

struct hash_fn
{
	std::size_t operator() (const VoxelNode& node) const
	{
		std::size_t h1 = std::hash<uint32_t>()(node.index.x);
		std::size_t h2 = std::hash<uint32_t>()(node.index.y);
		std::size_t h3 = std::hash<uint32_t>()(node.index.z);
		return h1 ^ h2 ^ h3;
	}

	std::size_t operator() (const XMUINT3& rhs) const
	{
		std::size_t h1 = std::hash<uint32_t>()(rhs.x);
		std::size_t h2 = std::hash<uint32_t>()(rhs.y);
		std::size_t h3 = std::hash<uint32_t>()(rhs.z);
		return h1 ^ h2 ^ h3;
	}
};

class TargaTextureClass;

class Voxel
{
public:
	//struct VertexType
	//{
	//	XMFLOAT3 position;
	//	XMFLOAT2 uv;
	//	XMFLOAT3 normal = XMFLOAT3(0.f, 0.f, 0.f);
	//	XMFLOAT4 color = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
	//};

	struct SubChunkInfo
	{
		XMUINT3 min = XMUINT3(0, 0, 0);
		XMUINT3 max = XMUINT3(0, 0, 0);
		int lodLevel = 0;

		MeshClass* mesh = nullptr;
		
		SubChunkInfo() {}
		SubChunkInfo(const SubChunkInfo& rhs)
		{
			min = rhs.min;
			max = rhs.max;
			lodLevel = rhs.lodLevel;
		}
		~SubChunkInfo()
		{
			if (mesh)
			{
				delete mesh;
				mesh = nullptr;
			}
		}

		bool InitializeMeshBuffer(ID3D11Device* device)
		{
			if (!mesh)
				mesh = new MeshClass;
			if (!mesh->InitializeBuffers(device))
				return false;
		}
		bool LoadMeshTexture(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName)
		{
			if (!mesh)
				mesh = new MeshClass;
			return mesh->LoadTexture(device, context, fileName);
		}
		//XMUINT3 GetCenter() const { return (max - min); }
	};

	//struct UCHAR3
	//{
	//	unsigned char x;
	//	unsigned char y;
	//	unsigned char z;

	//	bool operator==(const UCHAR3& rhs) const
	//	{
	//		return ((x == rhs.x) && (y == rhs.y) && (z == rhs.z));
	//	}
	//};

public:
	Voxel(XMFLOAT3 pos, UINT chunkSize = 32, float cellSize = 1.f);
	Voxel(const Voxel&) = delete;
	~Voxel();

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName);
	void Shutdown();
	void Render(ID3D11DeviceContext* context, const int& index);
	void Update();

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTexture();

	XMFLOAT3* GetChunkBounds();
	VoxelData& GetVoxelData(XMUINT3 pos);
	float GetVoxelVertex(XMUINT3 pos);
	void SetVoxel(XMUINT3 pos, UINT bitFlag, float threshold = 0.5f);
	void SetVoxelSphere(XMFLOAT3 pos, float radius, bool draw);
	void SetVoxelVertex(XMUINT3 pos, float threshold = 0.5f);

	bool RayCast(Ray3D& ray, XMFLOAT3& out);

	bool VoxelAStar(std::vector<XMFLOAT3> &path);
	void PathOptimization(std::vector<XMFLOAT3>& path);

	SubChunkInfo subChunks[64];

	void SetNavStart(XMFLOAT3 pos);
	void SetNavStart(XMUINT3 index);
	void SetNavEnd(XMFLOAT3 pos);
	void SetNavEnd(XMUINT3 index);
private:
	float InterpThreshold(const float& t0, const float& t1);
	//XMUINT3 GetVoxelVertexPosition(const XMUINT3& pos, UCHAR vertexNumber);
	XMUINT3 GetVoxelVertexPosition(const XMUINT3& pos, UCHAR vertexNumber, const int& lodLevel = 0);
	XMUINT3 GetTransVoxelVertexPosition(const XMUINT3& pos, UCHAR face, UCHAR vertexNumber, const int& lodLevel = 0);
	UINT VertexPositionToArrayKey(const XMUINT3& pos);
	XMFLOAT3 GetCenterPositionFromIndex(const XMUINT3& index);
	//VertexType VertexInterp(const XMUINT3& v0, const int edge, float threshold0);
	MeshClass::VertexType VertexInterp(const XMUINT3& v0, const XMUINT3& v1, float threshold0);
	unsigned char GetVoxelBitFlag(XMUINT3 pos, const int& lodLevel = 0);
	unsigned short GetTransVoxelBitFlag(XMUINT3 pos, UCHAR face, const int& lodLevel);
	UCHAR GetTransVoxelFace(UINT index);
	void GetVoxelIA(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices);
	void GetTransVoxelIA(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices);
	void GetTransVoxelIAThreadPool();//std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices);
	void GetTransVoxelIATask(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices, int startX, int endX);
	void GetTransVoxelIATask(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices, XMUINT3 start, XMUINT3 end, int lodLevel);
	void GetTransVoxelIATask(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices, UINT subChunkIndex);
	void RegularCell(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices, const UINT& subChunkIndex, const XMUINT3& indexPos);
	void TransCell(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices, const UINT& subChunkIndex, const XMUINT3& indexPos, const int& face, int& drawCount);
	XMUINT3 WorldPositionToLocalIndex(const XMFLOAT3& position);
	//bool LoadVoxelToBuffers(VertexType *vertices, int *indices);

	bool CheckOutOfBoundary(const XMUINT3& pos);
	std::vector<VoxelNode*> GetNeighbours(const XMUINT3& current, std::unordered_map<int, VoxelNode>& nodes);
	int GetNeighbourDistance(const XMUINT3& first, const XMUINT3& second);

	bool InitializeBuffers(ID3D11Device* device);
	bool UpdateBuffers(ID3D11DeviceContext* context);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* context, const int& index);

	bool LoadTexture(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName);
	void ReleaseTexture();

	std::vector<VoxelVertex> voxelVertices;
	std::vector<VoxelData> voxels;
	UINT chunkSize;
	XMFLOAT3 chunkBounds[2];
	float cellSize;
	XMFLOAT3 position;

	ID3D11Device* device = nullptr;
	ID3D11Buffer* vb = nullptr;
	ID3D11Buffer* ib = nullptr;
	int vCount = 0;
	int iCount = 0;
	bool dirtyFlag = false;
	bool init = true;

	TargaTextureClass* texture = nullptr;

	ThreadPool* pool = nullptr;

	XMUINT3 navStart = XMUINT3(0, 0, 0);
	XMUINT3 navEnd = XMUINT3(0, 0, 0);
	std::unordered_set<int> changed;
};