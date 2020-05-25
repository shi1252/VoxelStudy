#pragma once
#include "Defined.h"
#include <unordered_map>
#include <typeinfo>

class LightClass;

class ShaderParameter
{
private:
	std::unordered_map<std::string, XMFLOAT4> pfloat4;
	std::unordered_map<std::string, XMFLOAT3> pfloat3;
	std::unordered_map<std::string, XMFLOAT2> pfloat2;
	std::unordered_map<std::string, float> pfloat;
	std::unordered_map<std::string, XMMATRIX> pmatrix;
	std::unordered_map<std::string, LightClass*> plight;

public:
	ShaderParameter() = default;
	~ShaderParameter() = default;

	void SetParam(std::string name, XMFLOAT4 val) {pfloat4[name] = val;}
	void SetParam(std::string name, XMFLOAT3 val) {pfloat3[name] = val;}
	void SetParam(std::string name, XMFLOAT2 val) {pfloat2[name] = val;}
	void SetParam(std::string name, float val) {pfloat[name] = val;}
	void SetParam(std::string name, XMMATRIX val) {pmatrix[name] = val;}
	void SetParam(std::string name, LightClass* val) { plight[name] = val; }

	template <typename T>
	T GetParam(std::string name) { return T(); }
	template <>
	XMFLOAT4 GetParam<XMFLOAT4>(std::string name)
	{
		if (pfloat4.find(name) != pfloat4.end())
			return pfloat4[name];
		else
			return XMFLOAT4();
	}
	template <>
	XMFLOAT3 GetParam<XMFLOAT3>(std::string name)
	{
		if (pfloat3.find(name) != pfloat3.end())
			return pfloat3[name];
		else
			return XMFLOAT3();
	}
	template <>
	XMFLOAT2 GetParam(std::string name)
	{
		if (pfloat2.find(name) != pfloat2.end())
			return pfloat2[name];
		else
			return XMFLOAT2();
	}
	template <>
	float GetParam(std::string name)
	{
		if (pfloat.find(name) != pfloat.end())
			return pfloat[name];
		else
			return 0.f;
	}
	template <>
	XMMATRIX GetParam(std::string name)
	{
		if (pmatrix.find(name) != pmatrix.end())
			return pmatrix[name];
		else
			return XMMATRIX();
	}
	template<>
	LightClass* GetParam(std::string name)
	{
		if (plight.find(name) != plight.end())
			return plight[name];
		else
			return nullptr;
	}
};