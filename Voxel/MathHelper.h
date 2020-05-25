#pragma once
#include <math.h>
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
};