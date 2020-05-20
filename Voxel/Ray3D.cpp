#include "Ray3D.h"

Ray3D::Ray3D(XMFLOAT3 start, XMFLOAT3 end)
{
	origin = start;
	XMVECTOR sub = XMVectorSubtract(XMLoadFloat3(&end), XMLoadFloat3(&start));
	XMStoreFloat3(&direction, XMVector3Normalize(sub));
	XMStoreFloat(&length, XMVector3Length(sub));

	invDirection = XMFLOAT3(1.f / direction.x, 1.f / direction.y, 1.f / direction.z);
	sign[0] = (invDirection.x < 0);
	sign[1] = (invDirection.y < 0);
	sign[2] = (invDirection.z < 0);
}

Ray3D::Ray3D(XMFLOAT3 origin, XMFLOAT3 direction, float length)
	: origin(origin), direction(direction), length(length)
{
	invDirection = XMFLOAT3(1.f / direction.x, 1.f / direction.y, 1.f / direction.z);
	sign[0] = (invDirection.x < 0);
	sign[1] = (invDirection.y < 0);
	sign[2] = (invDirection.z < 0);
}

bool Ray3D::IntersectWithCube(const XMFLOAT3& boundMin, const XMFLOAT3& boundMax, XMFLOAT3& hitPos)
{
	XMFLOAT3 bounds[2] = { boundMin, boundMax };
	return IntersectWithCube(bounds, hitPos);
}

bool Ray3D::IntersectWithCube(const XMFLOAT3& boundMin, const XMFLOAT3& boundMax, XMFLOAT3& minPos, XMFLOAT3& maxPos)
{
	XMFLOAT3 bounds[2] = { boundMin, boundMax };
	return IntersectWithCube(bounds, minPos, maxPos);
}

bool Ray3D::IntersectWithCube(const XMFLOAT3* bounds, XMFLOAT3& hitPos)
{
	float tmin = (bounds[sign[0]].x - origin.x) * invDirection.x;
	float tmax = (bounds[1 - sign[0]].x - origin.x) * invDirection.x;
	float tymin = (bounds[sign[1]].y - origin.y) * invDirection.y;
	float tymax = (bounds[1 - sign[1]].y - origin.y) * invDirection.y;

	if ((tmin > tymax) || (tymin > tmax))
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (bounds[sign[2]].z - origin.z) * invDirection.z;
	float tzmax = (bounds[1 - sign[2]].z - origin.z) * invDirection.z;

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	float t = tmin;

	if (t < 0)
	{
		t = tmax;
		if (t < 0)
			return false;
	}

	hitPos = XMFLOAT3(origin.x + direction.x * t, 
		origin.y + direction.y * t, 
		origin.z + direction.z * t);

	return true;
}

bool Ray3D::IntersectWithCube(const XMFLOAT3* bounds, XMFLOAT3& minPos, XMFLOAT3& maxPos)
{
	float tmin = (bounds[sign[0]].x - origin.x) * invDirection.x;
	float tmax = (bounds[1 - sign[0]].x - origin.x) * invDirection.x;
	float tymin = (bounds[sign[1]].y - origin.y) * invDirection.y;
	float tymax = (bounds[1 - sign[1]].y - origin.y) * invDirection.y;

	if ((tmin > tymax) || (tymin > tmax))
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (bounds[sign[2]].z - origin.z) * invDirection.z;
	float tzmax = (bounds[1 - sign[2]].z - origin.z) * invDirection.z;

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	float t = tmin;

	if (t < 0)
	{
		t = tmax;
		if (t < 0)
			return false;
	}

	minPos = XMFLOAT3(origin.x + direction.x * tmin,
		origin.y + direction.y * tmin,
		origin.z + direction.z * tmin);

	maxPos = XMFLOAT3(origin.x + direction.x * tmax,
		origin.y + direction.y * tmax,
		origin.z + direction.z * tmax);

	return true;
}
