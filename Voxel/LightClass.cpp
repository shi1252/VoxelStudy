#include "LightClass.h"

void LightClass::SetAmbientColor(XMFLOAT4 color)
{
	ambient = color;
}

void LightClass::SetDiffuseColor(XMFLOAT4 color)
{
	diffuse = color;
}

void LightClass::SetPosition(XMFLOAT3 pos)
{
	position = pos;
}

void LightClass::SetRotation(XMFLOAT3 rot)
{
	rotation = rot;
}

XMFLOAT3 LightClass::GetLightDirection()
{
	XMFLOAT3 forward;
	XMStoreFloat3(&forward, XMVector4Transform(XMVectorSet(0.f, 0.f, 1.f, 0.f), XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z))));
	return forward;
}
