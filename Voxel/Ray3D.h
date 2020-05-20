#pragma once
#include "Defined.h"

struct Ray3D
{
	Ray3D(XMFLOAT3 start, XMFLOAT3 end);
	Ray3D(XMFLOAT3 origin, XMFLOAT3 direction, float length);

	bool IntersectWithCube(const XMFLOAT3& boundMin, const XMFLOAT3& boundMax, XMFLOAT3& hitPos);
	bool IntersectWithCube(const XMFLOAT3& boundMin, const XMFLOAT3& boundMax, XMFLOAT3& minPos, XMFLOAT3& maxPos);
	bool IntersectWithCube(_In_reads_(2) const XMFLOAT3* bounds, XMFLOAT3& hitPos);
	bool IntersectWithCube(_In_reads_(2) const XMFLOAT3* bounds, XMFLOAT3& minPos, XMFLOAT3& maxPos);

	XMFLOAT3 origin;
	XMFLOAT3 direction;
	XMFLOAT3 invDirection;
	float length;
	int sign[3];
};