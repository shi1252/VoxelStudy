#pragma once
#include <vector>
class PerlinNoise
{
public:
	PerlinNoise();

	double Noise(double x, double y, double z);

private:
	double Fade(double t);
	double Lerp(double t, double a, double b);
	double Grad(int hash, double x, double y, double z);

	std::vector<int> perm;
};