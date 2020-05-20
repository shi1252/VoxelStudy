#pragma once
#include "Defined.h"
#include <vector>
#include <unordered_map>

class Ray3D;

struct VoxelVertex
{
	float threshold = 0.f;
};

enum VoxelType
{
	NONE,
	LAND
};

struct VoxelData
{
	VoxelType type = VoxelType::NONE;
};

struct hash_fn
{
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
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
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
	Voxel(XMFLOAT3 pos, XMUINT3 chunkSize = XMUINT3(32, 32, 32), float cellSize = 1.f);
	Voxel(const Voxel&) = delete;
	~Voxel();

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName);
	void Shutdown();
	void Render(ID3D11DeviceContext* context);

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTexture();

	XMFLOAT3* GetChunkBounds();
	VoxelData& GetVoxelData(XMFLOAT3 pos);
	float GetVoxelVertex(XMUINT3 pos);
	void SetVoxel(XMUINT3 pos, UINT bitFlag, float threshold = 0.5f);
	void SetVoxelSphere(XMFLOAT3 pos, float radius, bool draw);
	void SetVoxelVertex(XMUINT3 pos, float threshold = 0.5f);

	bool RayCast(Ray3D& ray, XMFLOAT3& out);

private:
	float InterpThreshold(const float& t0, const float& t1);
	XMUINT3 GetVoxelVertexPosition(const XMUINT3& pos, UCHAR vertexNumber);
	UINT VertexPositionToArrayKey(const XMUINT3& pos);
	VertexType VertexInterp(const XMUINT3& v0, const int edge, float threshold0);
	unsigned char GetVoxelBitFlag(XMUINT3 pos);
	void GetVoxelIA(std::vector<Voxel::VertexType>& vertices, std::vector<int>& indices);
	XMUINT3 WorldPositionToLocalIndex(const XMFLOAT3& position);
	//bool LoadVoxelToBuffers(VertexType *vertices, int *indices);

	bool InitializeBuffers(ID3D11Device* device);
	bool UpdateBuffers(ID3D11DeviceContext* context);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* context);

	bool LoadTexture(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName);
	void ReleaseTexture();

	std::vector<VoxelVertex> voxelVertices;
	std::unordered_map<XMUINT3, VoxelData, hash_fn> voxels;
	XMUINT3 chunkSize;
	XMFLOAT3 chunkBounds[2];
	float cellSize;
	XMFLOAT3 position;

	ID3D11Device* device = nullptr;
	ID3D11Buffer* vb = nullptr;
	ID3D11Buffer* ib = nullptr;
	int vCount = 0;
	int iCount = 0;
	bool dirtyFlag = false;

	TargaTextureClass* texture = nullptr;
};