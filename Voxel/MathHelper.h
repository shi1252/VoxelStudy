#pragma once
#include <math.h>
#define Rad2Deg(x) (x * 57.2957795131f)
namespace MathHelper
{
	template <typename T>
	static T Clamp(const T& val, const T& min, const T& max)
	{
		return (val < min ? min : (val > max ? max : val));
	}

	static float Distance(const XMFLOAT3& v0, const XMFLOAT3& v1)
	{
		float dx = v1.x - v0.x;
		float dy = v1.y - v0.y;
		float dz = v1.z - v0.z;
		return sqrtf(dx * dx + dy * dy + dz * dz);
	}

	template <typename T>
	static T Sign(const T& val)
	{
		if (val > (T)0)
			return (T)1;
		else if (val < (T)0)
			return (T)-1;
		else
			return (T)0;
	}

	static XMFLOAT3 NormalVector(const XMFLOAT3& v0, const XMFLOAT3& v1, XMFLOAT3& v2)
	{
		XMVECTOR dir10 = XMVectorSubtract(XMLoadFloat3(&v0), XMLoadFloat3(&v1));
		XMVECTOR dir12 = XMVectorSubtract(XMLoadFloat3(&v2), XMLoadFloat3(&v1));

		dir10 = XMVector3Normalize(dir10);
		dir12 = XMVector3Normalize(dir12);

		XMVECTOR normal = XMVector3Cross(dir12, dir10);
		normal = XMVector3Normalize(normal);

		XMFLOAT3 result;
		XMStoreFloat3(&result, normal);

		return result;
	}

	inline XMFLOAT3 operator+=(XMFLOAT3& lhs, const XMFLOAT3& rhs)
	{
		lhs = XMFLOAT3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
		return lhs;
	}

	inline XMFLOAT3 operator-=(XMFLOAT3& lhs, const XMFLOAT3& rhs)
	{
		lhs = XMFLOAT3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
		return lhs;
	}

	inline XMFLOAT3 operator+(const XMFLOAT3& lhs, const XMFLOAT3& rhs)
	{
		return XMFLOAT3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
	}

	inline XMFLOAT3 operator-(const XMFLOAT3& lhs, const XMFLOAT3& rhs)
	{
		return XMFLOAT3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
	}

	inline bool operator!=(const XMFLOAT3& lhs, const XMFLOAT3& rhs)
	{
		return (lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z);
	}

	inline XMUINT3 operator+(const XMUINT3& lhs, const XMUINT3& rhs)
	{
		return XMUINT3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
	}

	inline XMUINT3 operator/(const XMUINT3& lhs, const UINT& div)
	{
		return XMUINT3(lhs.x / div, lhs.y / div, lhs.z / div);
	}

	inline XMUINT3 operator-(const XMUINT3& lhs, const XMUINT3& rhs)
	{
		return XMUINT3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
	}

	inline bool operator==(const XMUINT3& lhs, const XMUINT3& rhs)
	{
		return (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z);
	}

	inline XMFLOAT3 operator*(const XMFLOAT3& lhs, const float& val)
	{
		return XMFLOAT3(lhs.x * val, lhs.y * val, lhs.z * val);
	}

	static XMUINT3 XMFLOAT3ToXMUINT3(const XMFLOAT3& lhs)
	{
		return XMUINT3(lhs.x, lhs.y, lhs.z);
	}

	static XMFLOAT3 XMUINT3ToXMFLOAT3(const XMUINT3& lhs)
	{
		return XMFLOAT3(lhs.x, lhs.y, lhs.z);
	}

	static bool IsInsideCube(XMFLOAT3* const & bounds, const XMFLOAT3 position)
	{
		if (position.x < bounds[0].x || position.x > bounds[1].x
			|| position.y < bounds[0].y || position.y > bounds[1].y
			|| position.z < bounds[0].z || position.z > bounds[1].z)
			return false;
		return true;
	}
};