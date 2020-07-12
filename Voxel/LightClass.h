#pragma once
#include "Defined.h"

class LightClass
{
public:
	LightClass() : rotation(XMFLOAT3(45.f, 45.f, 0.f)) {}
	LightClass(const LightClass&) = delete;
	~LightClass() = default;

	void SetAmbientColor(XMFLOAT4 color);
	void SetDiffuseColor(XMFLOAT4 color);
	void SetPosition(XMFLOAT3 pos);
	void SetRotation(XMFLOAT3 rot);

	XMFLOAT4 GetAmbientColor() { return ambient; }
	XMFLOAT4 GetDiffuseColor() { return diffuse; }
	XMFLOAT3 GetPosition() { return position; }
	XMFLOAT3 GetLightDirection();

private:
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
};