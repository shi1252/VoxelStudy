#pragma once
class MathHelper
{
public:
	template <typename T>
	static T Clamp(const T& val, const T& min, const T& max)
	{
		return (val < min ? min : (val > max ? max : val));
	}
};