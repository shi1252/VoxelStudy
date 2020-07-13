#include "Voxel.h"
#include "MathHelper.h"
#include "PriorityQueue.h"
#include "PerlinNoise.h"
#include "Ray3D.h"
#include "TargaTextureClass.h"
#include "ThreadPool.h"
#include "Transvoxel.h"
#include "CameraClass.h"
#include <math.h>
#include <unordered_set>

using namespace MathHelper;

#pragma region Lookup Table
unsigned int edgeConnection[12][2] = {
		//	{0,1}, {1,2}, {2,3}, {3,0},
		//{4,5}, {5,6}, {6,7}, {7,4},
		//{0,4}, {1,5}, {2,6}, {3,7} };
		{0,1}, {1,2}, {3,2}, {0,3},
		{4,5}, {5,6}, {7,6}, {4,7},
		{0,4}, {1,5}, {2,6}, {3,7}};
int vertexOffset[8][3] = {
	{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0},
	{0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1} };// Transvoxel
	//{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1},
	//{0, 1, 0}, {1, 1, 0}, {1, 1, 1}, {0, 1, 1}};
XMFLOAT3 transVertexOffset[6][13] = {
	// Back
	{ XMFLOAT3(0.f,0.f,0.f), XMFLOAT3(0.5f,0.f,0.f), XMFLOAT3(1.f,0.f,0.f), 
	  XMFLOAT3(0.f,0.5f,0.f), XMFLOAT3(0.5f,0.5f,0.f), XMFLOAT3(1.f,0.5f,0.f), 
	  XMFLOAT3(0.f,1.f,0.f), XMFLOAT3(0.5f,1.f,0.f),XMFLOAT3(1.f,1.f,0.f), 
	  XMFLOAT3(0.f,0.f,0.f), XMFLOAT3(1.f,0.f,0.f),  XMFLOAT3(0.f,1.f,0.f), XMFLOAT3(1.f,1.f,0.f)},
	// Front
	{ XMFLOAT3(1.f,0.f,1.f),XMFLOAT3(0.5f,0.f,1.f) ,XMFLOAT3(0.f,0.f,1.f) ,
	  XMFLOAT3(1.f,0.5f,1.f) ,XMFLOAT3(0.5f,0.5f,1.f) ,XMFLOAT3(0.f,0.5f,1.f) ,
	  XMFLOAT3(1.f,1.f,1.f) ,XMFLOAT3(0.5f,1.f,1.f),XMFLOAT3(0.f,1.f,1.f),
	  XMFLOAT3(1.f,0.f,1.f),XMFLOAT3(0.f,0.f,1.f), XMFLOAT3(1.f,1.f,1.f), XMFLOAT3(0.f,1.f,1.f)},
	// Left
	{ XMFLOAT3(0.f,0.f,1.f),XMFLOAT3(0.f,0.f,0.5f) ,XMFLOAT3(0.f,0.f,0.f) ,
	  XMFLOAT3(0.f,0.5f,1.f) ,XMFLOAT3(0.f,0.5f,0.5f) ,XMFLOAT3(0.f,0.5f,0.f) ,
	  XMFLOAT3(0.f,1.f,1.f) ,XMFLOAT3(0.f,1.f,0.5f),XMFLOAT3(0.f,1.f,0.f),
	  XMFLOAT3(0.f,0.f,1.f), XMFLOAT3(0.f,0.f,0.f),  XMFLOAT3(0.f,1.f,1.f), XMFLOAT3(0.f,1.f,0.f) },
	// Right
	{ XMFLOAT3(1.f,0.f,0.f),XMFLOAT3(1.f,0.f,0.5f) ,XMFLOAT3(1.f,0.f,1.f) ,
	  XMFLOAT3(1.f,0.5f,0.f) ,XMFLOAT3(1.f,0.5f,0.5f) ,XMFLOAT3(1.f,0.5f,1.f) ,
	  XMFLOAT3(1.f,1.f,0.f) ,XMFLOAT3(1.f,1.f,0.5f),XMFLOAT3(1.f,1.f,1.f), 
	  XMFLOAT3(1.f,0.f,0.f), XMFLOAT3(1.f,0.f,1.f) , XMFLOAT3(1.f,1.f,0.f),XMFLOAT3(1.f,1.f,1.f) },
	// Down
	{ XMFLOAT3(0.f,0.f,1.f),XMFLOAT3(0.5f,0.f,1.f) ,XMFLOAT3(1.f,0.f,1.f) ,
	  XMFLOAT3(0.f,0.f,0.5f) ,XMFLOAT3(0.5f,0.f,0.5f) ,XMFLOAT3(1.f,0.f,0.5f) ,
	  XMFLOAT3(0.f,0.f,0.f) ,XMFLOAT3(0.5f,0.f,0.f),XMFLOAT3(1.f,0.f,0.f), 
	  XMFLOAT3(0.f,0.f,1.f), XMFLOAT3(1.f,0.f,1.f), XMFLOAT3(0.f,0.f,0.f), XMFLOAT3(1.f,0.f,0.f) },
	// Up
	{ XMFLOAT3(0.f,1.f,0.f),XMFLOAT3(0.5f,1.f,0.f) ,XMFLOAT3(1.f,1.f,0.f) ,
	  XMFLOAT3(0.f,1.f,0.5f) ,XMFLOAT3(0.5f,1.f,0.5f) ,XMFLOAT3(1.f,1.f,0.5f) ,
	  XMFLOAT3(0.f,1.f,1.f) ,XMFLOAT3(0.5f,1.f,1.f),XMFLOAT3(1.f,1.f,1.f), 
	  XMFLOAT3(0.f,1.f,0.f), XMFLOAT3(1.f,1.f,0.f),  XMFLOAT3(0.f,1.f,1.f), XMFLOAT3(1.f,1.f,1.f) }
};
int edgeDirection[12][3] = {
	{1, 0, 0}, {0, 0, 1}, {-1, 0, 0}, {0, 0, -1},
	{1, 0, 0}, {0, 0, 1}, {-1, 0, 0}, {0, 0, -1},
	{0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}
};
int edgeTable[256] = {
0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0 };
int triTable[256][16] = { {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
{3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
{4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
{9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
{10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
{5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
{8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
{2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
{11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
{5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
{11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
{11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
{9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
{6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
{6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
{8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
{7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
{3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
{9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
{8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
{0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
{6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
{10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
{10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
{0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
{3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
{9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
{8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
{3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
{10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
{10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
{7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
{1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
{11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
{8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
{0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
{7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
{7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
{10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
{0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
{7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
{9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
{6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
{4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
{10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
{8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
{1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
{10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
{10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
{9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
{7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
{3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
{7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
{3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
{6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
{9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
{1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
{4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
{7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
{6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
{0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
{6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
{0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
{11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
{6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
{5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
{9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
{1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
{10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
{0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
{5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
{11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
{9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
{7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
{2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
{9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
{1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
{10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
{2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
{0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
{0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
{9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
{5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
{5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
{9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
{1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
{3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
{4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
{9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
{11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
{2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
{9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
{3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
{1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
{4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
{0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
{1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1} };
VoxelType voxelTypeTable[256] = {
	NONE, LAND, LAND, LAND, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, LAND, LAND, LAND, LAND, NONE, NONE, NONE, LAND, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, LAND, LAND, LAND, LAND, NONE, NONE, NONE, NONE, NONE, NONE, NONE, LAND, NONE, NONE, NONE, NONE, LAND, LAND, LAND, LAND, NONE, NONE, NONE, LAND, NONE, NONE, NONE, LAND, NONE, NONE, NONE, LAND, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, LAND, NONE, LAND, NONE, NONE, NONE, LAND, NONE, NONE, NONE, LAND, NONE, NONE, NONE, LAND, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, LAND, LAND, NONE, NONE, NONE, LAND, NONE, NONE, NONE, LAND, NONE, NONE, NONE, LAND, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, LAND, NONE, NONE, NONE, LAND, NONE, NONE, NONE, LAND, NONE, NONE, NONE, NONE };
#pragma endregion

#include <fstream>
Voxel::Voxel(XMFLOAT3 pos, UINT chunkSize, float cellSize)
	: position(pos), chunkSize(chunkSize), cellSize(cellSize)
{
	dirtyFlag = true;
	chunkBounds[0] = position;
	chunkBounds[1] = XMFLOAT3(position.x + chunkSize * cellSize, 
		position.y + chunkSize * cellSize,
		position.z + chunkSize * cellSize);
	PerlinNoise perlin;
	voxelVertices.resize((chunkSize + 1) * (chunkSize + 1) * (chunkSize + 1));
	voxels.resize((chunkSize) * (chunkSize) * (chunkSize));
	for (int z = 0; z < chunkSize + 1; ++z)
	{
		for (int y = 0; y < chunkSize + 1; ++y)
		{
			for (int x = 0; x < chunkSize + 1; ++x)
			{
				voxelVertices[x + y * (chunkSize + 1) + z * (chunkSize + 1) * (chunkSize + 1)] = VoxelVertex();
				if (x < chunkSize && y < chunkSize && z < chunkSize)
					voxels[x + y * chunkSize + z * chunkSize * chunkSize] = VoxelData();
				float noise = perlin.Noise((double)x / (chunkSize + 1) * 0.8, (double)z / (chunkSize + 1) * 0.8, 0.1)* (chunkSize + 1);
				if (y < noise)
				{
					//if ((noise - y) >= 1.f)
					//	voxelVertices[VertexPositionToArrayKey(XMUINT3(x, y, z))].threshold = 1.f;
					//else
						voxelVertices[VertexPositionToArrayKey(XMUINT3(x, y, z))].threshold = (noise - y);
				}
			}
		}
	}

	//SetVoxelSphere(XMFLOAT3(-22.f, -60.f, 45.f), 1.3f, true);

	//SetVoxelSphere(XMFLOAT3(position) + XMFLOAT3(31.5, 31.5, 31.5), 1.f, true);

	//for (int x = 0; x < chunkSize+1; ++x)
	//{
	//	for (int y = 0; y < chunkSize+1; ++y)
	//	{
	//		for (int z = 0; z < chunkSize+1; ++z)
	//		{
	//			SetVoxelVertex(XMUINT3(x, y, z), (chunkSize / 2.f - (y + 0.f)));
	//		}
	//	}
	//}

	// For voxel normal data
	//for (int i = 0; i < 256; ++i)
	//{
	//	SetVoxel(XMUINT3(i % chunkSize, 70 + i / chunkSize * 3.f, 5), i);
	//}

	//std::ofstream of;
	//of.open("output.txt", std::ios::trunc);

	//std::vector<MeshClass::VertexType> vertices;
	//std::unordered_map<int, int> vertList;
	//for (int bitFlag = 0; bitFlag < 256; ++bitFlag)
	//{
	//std::vector<int> idxList;
	//	if (edgeTable[bitFlag] == 0)
	//	{
	//		of << 0 << ", ";
	//		continue;
	//	}

	//	RegularCellData rcd = regularCellData[regularCellClass[bitFlag]];
	//	int vc = rcd.GetVertexCount();
	//	int tc = rcd.GetTriangleCount();

	//	std::vector<unsigned char> vd;
	//	for (int i = 0; i < vc; ++i)
	//	{
	//		vd.push_back(regularVertexData[bitFlag][i]);
	//	}

	//	for (int i = 0; i < vc; ++i)
	//	{
	//		int d = (vd[i] & 0xFF);
	//		int d0 = d >> 4;
	//		int d1 = d & 0x0F;

	//		XMUINT3 p0 = XMUINT3(1 + vertexOffset[d0][0], 1 + vertexOffset[d0][1], 1 + vertexOffset[d0][2]);
	//		XMUINT3 p1 = XMUINT3(1 + vertexOffset[d1][0], 1 + vertexOffset[d1][1], 1 + vertexOffset[d1][2]);

	//		float t0 = 1; //GetVoxelVertex(p0);
	//		float t1 = -1;// GetVoxelVertex(p1);

	//		float interpt = (0.f - t0) / (t1 - t0);
	//		idxList.push_back(vertices.size());
	//		vertices.push_back(VertexInterp(p0, p1, interpt));
	//	}

	//	float max = 0.f;
	//	float min = 360.f;
	//	for (int i = 0; i < tc; ++i)
	//	{
	//		int t = i * 3;
	//		XMFLOAT3 normal = MathHelper::NormalVector(
	//			vertices[idxList[rcd.vertexIndex[t]]].position, 
	//			vertices[idxList[rcd.vertexIndex[t + 1]]].position, 
	//			vertices[idxList[rcd.vertexIndex[t + 2]]].position);
	//		float angle = XMVector3AngleBetweenVectors(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMLoadFloat3(&normal)).m128_f32[0];
	//		angle = Rad2Deg(angle);
	//		if (angle > max)
	//			max = angle;
	//		if (angle < min)
	//			min = angle;
	//		//of << "(" << normal.x << ", " << normal.y << ", " << normal.z << ") " << angle << "\n";
	//	}
	//	if (max < 60.f)
	//		of << 1 << ", ";//bitFlag << " " << max << ", " << min << "\n";
	//	else
	//		of << 0 << ", ";//bitFlag << " " << max << ", " << min << "\n";
	//}

	//of.close();
}

Voxel::~Voxel()
{
}

bool Voxel::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName)
{
	pool = new ThreadPool;
	if (pool == nullptr)
		return false;

	if (!this->device)
		this->device = device;

	for (int i = 0; i < 64; ++i)
	{
		if (!subChunks[i].InitializeMeshBuffer(device))
			return false;
		if (!subChunks[i].LoadMeshTexture(device, context, fileName))
			return false;
	}

	GetTransVoxelIAThreadPool();

	//// Initialize vertex index buffer
	//if (!InitializeBuffers(device))
	//	return false;

	init = false;

	// Load texture
	return LoadTexture(device, context, fileName);
}

void Voxel::Shutdown()
{
	if (pool)
	{
		delete pool;
		pool = nullptr;
	}

	voxelVertices.clear();

	ReleaseTexture();

	ShutdownBuffers();
}

void Voxel::Render(ID3D11DeviceContext* context, const int& index)
{
	RenderBuffers(context, index);
}

void Voxel::Update()
{
	GetTransVoxelIAThreadPool();
}

//void Voxel::Render(ID3D11DeviceContext* context)
//{
//	RenderBuffers(context);
//}

int Voxel::GetIndexCount()
{
	return iCount;
}

ID3D11ShaderResourceView* Voxel::GetTexture()
{
	return texture->GetTexture();
}

XMFLOAT3* Voxel::GetChunkBounds()
{
	return chunkBounds;
}

VoxelData& Voxel::GetVoxelData(XMUINT3 pos)
{
	if (CheckOutOfBoundary(pos))
		return voxels[0];
	return voxels[pos.x + pos.y * chunkSize + pos.z * chunkSize * chunkSize];
}

float Voxel::GetVoxelVertex(XMUINT3 pos)
{
	return voxelVertices[VertexPositionToArrayKey(pos)].threshold;
}

void Voxel::SetVoxel(XMUINT3 pos, UINT bitFlag, float threshold)
{
	UINT changed = GetVoxelBitFlag(pos) ^ bitFlag;
	for (int i = 0; i < 8; ++i)
	{
		if (changed & (1 << i))
			SetVoxelVertex(GetVoxelVertexPosition(pos, i), (bitFlag & (1 << i)) ? threshold : -threshold);
	}
}

void Voxel::SetVoxelSphere(XMFLOAT3 pos, float radius, bool draw)
{
	XMUINT3 index = WorldPositionToLocalIndex(pos);
	if (index.x == -1)
		return;

	for (int z = MathHelper::Clamp(index.z - (radius / cellSize), 0.f, (float)chunkSize);
		z < MathHelper::Clamp(index.z + (radius / cellSize + 2), 0.f, (float)chunkSize); ++z)
	{
		for (int y = MathHelper::Clamp(index.y - (radius / cellSize), 0.f, (float)chunkSize);
			y < MathHelper::Clamp(index.y + (radius / cellSize + 2), 0.f, (float)chunkSize); ++y)
		{
			for (int x = MathHelper::Clamp(index.x - (radius / cellSize), 0.f, (float)chunkSize);
				x < MathHelper::Clamp(index.x + (radius / cellSize + 2), 0.f, (float)chunkSize); ++x)
			{
				XMFLOAT3 targetPos = XMFLOAT3(this->position.x + x * cellSize, 
					this->position.y + y * cellSize,
					this->position.z + z * cellSize);
				//XMFLOAT3 targetPos = XMFLOAT3(pos.x - ((int)index.x - x) * cellSize, pos.y - ((int)index.y - y) * cellSize, pos.z - ((int)index.z - z) * cellSize);
				float distance = MathHelper::Distance(pos, targetPos);//XMVector3Length(XMVectorSubtract(XMLoadFloat3(&pos), XMLoadFloat3(&targetPos))).m128_f32[0];

				XMUINT3 newPos = XMUINT3(x, y, z);
				float value = radius - distance;//Clamp(radius - distance, -1.f, 1.f);
				float t = GetVoxelVertex(newPos);
				//if (t == value || MathHelper::Sign(t) == MathHelper::Sign(value) /*|| value > 1.f || value < -1.f*/)
				//	continue;
				if (draw)
				{
					if (value < 0.f)//t > value || value < -1.f)
						continue;
					if (t < value)
					{
						//if (Sign(t) == Sign(value))
							//SetVoxelVertex(newPos, t + value);
						//else
							//SetVoxelVertex(newPos, value);
					}
				}
				else
				{
					if (value < 0.f)//t <= value || value < -1.f)
						continue;
					//if (t > -value)
					//	SetVoxelVertex(newPos, -value);
				}
				SetVoxelVertex(newPos, (draw ? t + radius : t - radius));// -value));
				//if (!draw && GetVoxelVertex(newPos) <= 0.f)
				//{
				//	continue;
				//}
				//if (distance > radius)
				//{
				//	if ((distance - radius) <= cellSize)
				//		SetVoxelVertex(newPos, (distance - radius) / cellSize);
				//}
				//else
				//{
				//	SetVoxelVertex(newPos, (draw ? 1.f : 0.f));
				//}
			}
		}
	}
}

void Voxel::SetVoxelVertex(XMUINT3 pos, float threshold)
{
	voxelVertices[pos.x + pos.y * (chunkSize + 1) + 
		pos.z * (chunkSize + 1) * (chunkSize + 1)].threshold = MathHelper::Clamp(threshold, -1.f, 1.f);

	int chunkDelta = (chunkSize / 4);
	for (int i = -1; i < 1; ++i)
	{
		for (int j = -1; j < 1; ++j)
		{
			for (int k = -1; k < 1; ++k)
			{
				//changed.insert(i * 16 + j * 4 + k);
				changed.insert(Clamp(((pos.x + k) / chunkDelta), 0u, 3u) +
					Clamp(((pos.y + j) / chunkDelta), 0u, 3u) * 4u +
					Clamp(((pos.z + i) / chunkDelta), 0u, 3u) * 16u);
			}
		}
	}
	dirtyFlag = true;
}

bool IsEqual(const XMUINT3& lhs, const XMUINT3& rhs)
{
	return (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z);
}

bool Voxel::RayCast(Ray3D& ray, XMFLOAT3 &out)
{
	XMFLOAT3 minPos, maxPos;
	if (!ray.IntersectWithCube(chunkBounds, minPos, maxPos))
		return false;

	int stepX = (ray.direction.x >= 0) ? 1 : -1;
	int stepY = (ray.direction.y >= 0) ? 1 : -1;
	int stepZ = (ray.direction.z >= 0) ? 1 : -1;

	if (!IsInsideCube(chunkBounds, ray.origin))
	{
		minPos = XMFLOAT3(minPos.x + cellSize * stepX / 2.f, minPos.y + cellSize * stepY / 2.f, minPos.z + cellSize * stepZ / 2.f);
	}
	else
	{
		minPos = ray.origin;
	}
	maxPos = XMFLOAT3(maxPos.x - cellSize * stepX / 2.f, maxPos.y - cellSize * stepY / 2.f, maxPos.z - cellSize * stepZ / 2.f);

	XMUINT3 current = WorldPositionToLocalIndex(minPos);
	XMUINT3 last = WorldPositionToLocalIndex(maxPos);

	if (current.x == -1 || last.x == -1)
		return false;

	float next_voxel_boundry_x = (minPos.x + stepX * cellSize);
	float next_voxel_boundry_y = (minPos.y + stepY * cellSize);
	float next_voxel_boundry_z = (minPos.z + stepZ * cellSize);

	float tMaxX = (ray.direction.x != 0) ? (next_voxel_boundry_x - ray.origin.x) / ray.direction.x : FLT_MAX;
	float tMaxY = (ray.direction.y != 0) ? (next_voxel_boundry_y - ray.origin.y) / ray.direction.y : FLT_MAX;
	float tMaxZ = (ray.direction.z != 0) ? (next_voxel_boundry_z - ray.origin.z) / ray.direction.z : FLT_MAX;

	float tDeltaX = (ray.direction.x != 0) ? cellSize / ray.direction.x * stepX : FLT_MAX;
	float tDeltaY = (ray.direction.y != 0) ? cellSize / ray.direction.y * stepY : FLT_MAX;
	float tDeltaZ = (ray.direction.z != 0) ? cellSize / ray.direction.z * stepZ : FLT_MAX;

	while (!IsEqual(last, current))
	{
		if (tMaxX < tMaxY)
		{
			if (tMaxX < tMaxZ)
			{
				current.x += stepX;
				tMaxX += tDeltaX;
			}
			else
			{
				current.z += stepZ;
				tMaxZ += tDeltaZ;
			}
		}
		else
		{
			if (tMaxY < tMaxZ)
			{
				current.y += stepY;
				tMaxY += tDeltaY;
			}
			else
			{
				current.z += stepZ;
				tMaxZ += tDeltaZ;
			}
		}
		if (current.x >= chunkSize || current.y >= chunkSize || current.z >= chunkSize)
			break;
		if (Distance(ray.origin, GetCenterPositionFromIndex(current)) > (ray.length * 2))
			break;

		unsigned char bitFlag = GetVoxelBitFlag(current);
		if (bitFlag != 0 && bitFlag != 255)
		{
			out = XMFLOAT3(this->position.x + current.x * cellSize + cellSize / 2.f,
				this->position.y + current.y * cellSize + cellSize / 2.f,
				this->position.z + current.z * cellSize + cellSize / 2.f);
			return true;
		}
	}

	return false;
}

float Voxel::InterpThreshold(const float& t0, const float& t1)
{
	if ((t1 - t0) < FLT_EPSILON)
		return 0.5f;
	else
		return (- t0 / (t1 - t0));
	if (t0 < FLT_EPSILON)
		return (1.f - t1);
	else
		return (t0);
}

//XMUINT3 Voxel::GetVoxelVertexPosition(const XMUINT3& pos, UCHAR vertexNumber)
//{
//	return XMUINT3(pos.x + vertexOffset[vertexNumber][0],
//		pos.y + vertexOffset[vertexNumber][1], pos.z + vertexOffset[vertexNumber][2]);
//}

XMUINT3 Voxel::GetVoxelVertexPosition(const XMUINT3& pos, UCHAR vertexNumber, const int& lodLevel)
{
	return XMUINT3(pos.x + vertexOffset[vertexNumber][0] * pow(2, lodLevel),
		pos.y + vertexOffset[vertexNumber][1] * pow(2, lodLevel), pos.z + vertexOffset[vertexNumber][2] * pow(2, lodLevel));
}

XMUINT3 Voxel::GetTransVoxelVertexPosition(const XMUINT3& pos, UCHAR face, UCHAR vertexNumber, const int& lodLevel)
{
	return XMUINT3(pos.x + transVertexOffset[face][vertexNumber].x * pow(2, lodLevel),
		pos.y + transVertexOffset[face][vertexNumber].y * pow(2, lodLevel),
		pos.z + transVertexOffset[face][vertexNumber].z * pow(2, lodLevel));
}

UINT Voxel::VertexPositionToArrayKey(const XMUINT3& pos)
{
	return (pos.x + pos.y * (chunkSize + 1) + pos.z * (chunkSize + 1) * (chunkSize + 1));
}

XMFLOAT3 Voxel::GetCenterPositionFromIndex(const XMUINT3& index)
{
	return XMFLOAT3(position.x + index.x * cellSize + cellSize * 0.5f, position.y + index.y * cellSize + cellSize * 0.5f, position.z + index.z * cellSize + cellSize * 0.5f);
}

//Voxel::VertexType Voxel::VertexInterp(const XMUINT3& v0, const int edge, float threshold0)
//{
//		VertexType v;
//	
//		v.position = XMFLOAT3(position.x + (v0.x + edgeDirection[edge][0] * (1.f - threshold0)) * cellSize,
//			position.y + (v0.y + edgeDirection[edge][1] * (1.f - threshold0)) * cellSize,
//			position.z + (v0.z + edgeDirection[edge][2] * (1.f - threshold0)) * cellSize);
//	
//		return v;
//}

MeshClass::VertexType Voxel::VertexInterp(const XMUINT3& v0, const XMUINT3& v1, float threshold0)
{
	MeshClass::VertexType v;

	v.position = XMFLOAT3(position.x + (v1.x * threshold0 + v0.x * (1.f - threshold0)) * cellSize,
		position.y + (v1.y * threshold0 + v0.y * (1.f - threshold0)) * cellSize,
		position.z + (v1.z * threshold0 + v0.z * (1.f - threshold0)) * cellSize);

	//v.position = position + XMUINT3ToXMFLOAT3(v0) * cellSize + (XMUINT3ToXMFLOAT3(v0) - XMUINT3ToXMFLOAT3(v1)) * threshold0 * cellSize;

	return v;
}

unsigned char Voxel::GetVoxelBitFlag(XMUINT3 pos, const int& lodLevel)
{
	unsigned char bitFlag = 0;
	for (int i = 0; i < 8; ++i)
	{
		if (voxelVertices[VertexPositionToArrayKey(GetVoxelVertexPosition(pos, i, lodLevel))].threshold > 0.f)
			bitFlag |= (1 << i);
	}
	return bitFlag;
}

unsigned short Voxel::GetTransVoxelBitFlag(XMUINT3 pos, UCHAR face, const int& lodLevel)
{
	unsigned short bitFlag = 0;
	if (voxelVertices[VertexPositionToArrayKey(GetTransVoxelVertexPosition(pos, face, 0, lodLevel))].threshold > 0.f)
		bitFlag |= (1 << 0);
	if (voxelVertices[VertexPositionToArrayKey(GetTransVoxelVertexPosition(pos, face, 1, lodLevel))].threshold > 0.f)
		bitFlag |= (1 << 1);
	if (voxelVertices[VertexPositionToArrayKey(GetTransVoxelVertexPosition(pos, face, 2, lodLevel))].threshold > 0.f)
		bitFlag |= (1 << 2);
	if (voxelVertices[VertexPositionToArrayKey(GetTransVoxelVertexPosition(pos, face, 3, lodLevel))].threshold > 0.f)
		bitFlag |= (1 << 7);
	if (voxelVertices[VertexPositionToArrayKey(GetTransVoxelVertexPosition(pos, face, 4, lodLevel))].threshold > 0.f)
		bitFlag |= (1 << 8);
	if (voxelVertices[VertexPositionToArrayKey(GetTransVoxelVertexPosition(pos, face, 5, lodLevel))].threshold > 0.f)
		bitFlag |= (1 << 3);
	if (voxelVertices[VertexPositionToArrayKey(GetTransVoxelVertexPosition(pos, face, 6, lodLevel))].threshold > 0.f)
		bitFlag |= (1 << 6);
	if (voxelVertices[VertexPositionToArrayKey(GetTransVoxelVertexPosition(pos, face, 7, lodLevel))].threshold > 0.f)
		bitFlag |= (1 << 5);
	if (voxelVertices[VertexPositionToArrayKey(GetTransVoxelVertexPosition(pos, face, 8, lodLevel))].threshold > 0.f)
		bitFlag |= (1 << 4);
	return bitFlag;
}

XMINT3 facePositionData[6] = {
	XMINT3(0,0,-1),XMINT3(0,0,1),
	XMINT3(-1,0,0),XMINT3(1,0,0),
	XMINT3(0,-1,0),XMINT3(0,1,0)
};
UCHAR Voxel::GetTransVoxelFace(UINT index, bool inverse)
{
	UCHAR result = 0;
	//return result;
	UINT x = index & 0b11, y = (index >> 2) & 0b11, z = (index >> 4) & 0b11;

	if (!inverse)
	{
		// Back face
		if (z > 0)
		{
			UINT newX = x;
			UINT newY = y;
			UINT newZ = z - 1;
			if (subChunks[index].lodLevel > subChunks[newZ * 16 + newY * 4 + newX].lodLevel)
				result |= 1 << 0;
		}
		// Front face
		if (z < 3)
		{
			UINT newX = x;
			UINT newY = y;
			UINT newZ = z + 1;
			if (subChunks[index].lodLevel > subChunks[newZ * 16 + newY * 4 + newX].lodLevel)
				result |= 1 << 1;
		}
		// Left
		if (x > 0)
		{
			UINT newX = x - 1;
			UINT newY = y;
			UINT newZ = z;
			if (subChunks[index].lodLevel > subChunks[newZ * 16 + newY * 4 + newX].lodLevel)
				result |= 1 << 2;
		}
		// Right
		if (x < 3)
		{
			UINT newX = x + 1;
			UINT newY = y;
			UINT newZ = z;
			if (subChunks[index].lodLevel > subChunks[newZ * 16 + newY * 4 + newX].lodLevel)
				result |= 1 << 3;
		}
		// Down
		if (y > 0)
		{
			UINT newX = x;
			UINT newY = y - 1;
			UINT newZ = z;
			if (subChunks[index].lodLevel > subChunks[newZ * 16 + newY * 4 + newX].lodLevel)
				result |= 1 << 4;
		}
		// Up
		if (y < 3)
		{
			UINT newX = x;
			UINT newY = y + 1;
			UINT newZ = z;
			if (subChunks[index].lodLevel > subChunks[newZ * 16 + newY * 4 + newX].lodLevel)
				result |= 1 << 5;
		}

		return result;
	}
	else
	{
		// Back face
		if (z > 0)
		{
			UINT newX = x;
			UINT newY = y;
			UINT newZ = z - 1;
			if (subChunks[index].lodLevel < subChunks[newZ * 16 + newY * 4 + newX].lodLevel)
				result |= 1 << 0;
		}
		// Front face
		if (z < 3)
		{
			UINT newX = x;
			UINT newY = y;
			UINT newZ = z + 1;
			if (subChunks[index].lodLevel < subChunks[newZ * 16 + newY * 4 + newX].lodLevel)
				result |= 1 << 1;
		}
		// Left
		if (x > 0)
		{
			UINT newX = x - 1;
			UINT newY = y;
			UINT newZ = z;
			if (subChunks[index].lodLevel < subChunks[newZ * 16 + newY * 4 + newX].lodLevel)
				result |= 1 << 2;
		}
		// Right
		if (x < 3)
		{
			UINT newX = x + 1;
			UINT newY = y;
			UINT newZ = z;
			if (subChunks[index].lodLevel < subChunks[newZ * 16 + newY * 4 + newX].lodLevel)
				result |= 1 << 3;
		}
		// Down
		if (y > 0)
		{
			UINT newX = x;
			UINT newY = y - 1;
			UINT newZ = z;
			if (subChunks[index].lodLevel < subChunks[newZ * 16 + newY * 4 + newX].lodLevel)
				result |= 1 << 4;
		}
		// Up
		if (y < 3)
		{
			UINT newX = x;
			UINT newY = y + 1;
			UINT newZ = z;
			if (subChunks[index].lodLevel < subChunks[newZ * 16 + newY * 4 + newX].lodLevel)
				result |= 1 << 5;
		}

		return result;
	}
}

/// <summary>
/// Get vertex, index information of voxels to render
/// </summary>
/// <param name='vertices'>out</param>
/// <param name='indices'>out</param>
void Voxel::GetVoxelIA(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices)
{
	for (int z = 0; z < chunkSize; ++z)
	{
		for (int y = 0; y < chunkSize; ++y)
		{
			for (int x = 0; x < chunkSize; ++x)
			{
				unsigned char bitFlag = GetVoxelBitFlag(XMUINT3(x, y, z));
				if (edgeTable[bitFlag] == 0) continue;

				int prevVertexCount = vertices.size();
				std::unordered_map<int, int> vertList;

				for (int i = 0; i < 12; ++i)
				{
					if (edgeTable[bitFlag] & (1 << i))
					{
						vertList[i] = vertices.size();
						vertices.push_back(
							VertexInterp(
								GetVoxelVertexPosition(XMUINT3(x, y, z), edgeConnection[i][0]),
								GetVoxelVertexPosition(XMUINT3(x, y, z), edgeConnection[i][1]),
								InterpThreshold(GetVoxelVertex(GetVoxelVertexPosition(XMUINT3(x, y, z), edgeConnection[i][0])),
									GetVoxelVertex(GetVoxelVertexPosition(XMUINT3(x, y, z), edgeConnection[i][1])))));
							/*VertexInterp(
							GetVoxelVertexPosition(XMUINT3(x, y, z), edgeConnection[i][0]),
							i,
							InterpThreshold(GetVoxelVertex(GetVoxelVertexPosition(XMUINT3(x, y, z), edgeConnection[i][0])),
								GetVoxelVertex(GetVoxelVertexPosition(XMUINT3(x, y, z), edgeConnection[i][1]))) ));*/
					}
				}

				for (int i = 0; triTable[bitFlag][i] != -1; i += 3)
				{
					indices.push_back(vertList[triTable[bitFlag][i]]);
					indices.push_back(vertList[triTable[bitFlag][i + 1]]);
					indices.push_back(vertList[triTable[bitFlag][i + 2]]);

					vertices[vertList[triTable[bitFlag][i]]].uv = XMFLOAT2(0.f, 1.f);
					vertices[vertList[triTable[bitFlag][i + 1]]].uv = XMFLOAT2(0.5f, 0.f);
					vertices[vertList[triTable[bitFlag][i + 2]]].uv = XMFLOAT2(1.f, 1.f);
				}
			}
		}
	}
}

void Voxel::GetTransVoxelIA(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices)
{
	for (int z = 0; z < chunkSize; ++z)
	{
		for (int y = 0; y < chunkSize; ++y)
		{
			for (int x = 0; x < chunkSize; ++x)
			{
				unsigned char bitFlag = GetVoxelBitFlag(XMUINT3(x, y, z));
				if (edgeTable[bitFlag] == 0) continue;

				RegularCellData rcd = regularCellData[regularCellClass[bitFlag]];
				int vc = rcd.GetVertexCount();
				int tc = rcd.GetTriangleCount();

				std::vector<unsigned char> vd;
				for (int i = 0; i < vc; ++i)
				{
					vd.push_back(regularVertexData[bitFlag][i]);
				}

				std::unordered_map<int, int> vertList;
				std::vector<int> idxList;

				int mask = (x >= 0 ? 1 : 0) | (z >= 0 ? 2 : 0) | (y >= 0 ? 4 : 0);
				for (int i = 0; i < vc; ++i)
				{
					int d = (vd[i] & 0xFF);
					int d0 = d >> 4;
					int d1 = d & 0x0F;

					int r = vd[i] >> 8;
					int direction = r >> 4;
					int index = r & 0x0F;

					int nIndex = -1;
					bool p = (direction & mask) == direction;

					//XMUINT3 asdf = XMFLOAT3ToXMUINT3(XMFLOAT3(x, y, z));
					XMUINT3 p0 = XMUINT3(x + vertexOffset[d0][0], y + vertexOffset[d0][1], z + vertexOffset[d0][2]);
					XMUINT3 p1 = XMUINT3(x + vertexOffset[d1][0], y + vertexOffset[d1][1], z + vertexOffset[d1][2]);

					float t0 = GetVoxelVertex(p0);
					float t1 = GetVoxelVertex(p1);
					float interpt = (0.f - t0) / (t1 - t0);
					idxList.push_back(vertices.size());
					vertices.push_back(VertexInterp(p0, p1, interpt));
				}

				for (int i = 0; i < tc; ++i)
				{
					int t = i * 3;
					indices.push_back(idxList[rcd.vertexIndex[t]]);
					indices.push_back(idxList[rcd.vertexIndex[t + 1]]);
					indices.push_back(idxList[rcd.vertexIndex[t + 2]]);

					vertices[idxList[rcd.vertexIndex[t]]].uv = XMFLOAT2(0.f, 1.f);
					vertices[idxList[rcd.vertexIndex[t+1]]].uv = XMFLOAT2(0.5f, 0.f);
					vertices[idxList[rcd.vertexIndex[t+2]]].uv = XMFLOAT2(1.f, 1.f);

					vertices[idxList[rcd.vertexIndex[t]]].color = XMFLOAT4(1.f, 0.f, 0.f, 1.f);
					vertices[idxList[rcd.vertexIndex[t+1]]].color = XMFLOAT4(0.f, 1.f, 0.f, 1.f);
					vertices[idxList[rcd.vertexIndex[t+2]]].color = XMFLOAT4(0.f, 0.f, 1.f, 1.f);
				}
			}
		}
	}
}

void Voxel::GetTransVoxelIAThreadPool()//std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices)
{
	if (pool == nullptr)
	{
		//GetTransVoxelIA(vertices, indices);
		return;
	}
	size_t tc = 4 * 4 * 4;//pool->GetTotalThreadCount();

	std::vector<std::future<int>> results;
	//std::vector<std::vector<Voxel::VertexType>> threadVertices(tc);
	//std::vector<std::vector<int>> threadIndices(tc);

	//for (int i = 0; i < tc; ++i)
	//{
	//	std::vector<Voxel::VertexType> v;
	//	std::vector<int> idx;
	//	threadVertices.push_back(v);
	//	threadIndices.push_back(idx);
	//}

	int delta = chunkSize / 4;
	XMUINT3 start(0, 0, 0);
	XMUINT3 end(delta, delta, delta);

	bool lodChanged = false;

	for (int i = 0; i < 4; ++i)
	{
		start.y = 0;
		end.y = delta;
		for (int j = 0; j < 4; ++j)
		{
			start.x = 0;
			end.x = delta;
			for (int k = 0; k < 4; ++k)
			{
				XMUINT3 currIndex = (start + end) / 2;
				XMFLOAT3 currPosition = GetCenterPositionFromIndex(currIndex);
				float distance = Distance(CameraClass::mainCam->GetPosition(), currPosition);
				int lodLevel = Clamp((int)(distance / 70.f), (int)LOD0, (int)LOD3);
				if (init || subChunks[i * 16 + j * 4 + k].lodLevel != lodLevel)
				{
					subChunks[i * 16 + j * 4 + k].lodLevel = lodLevel;
					subChunks[i * 16 + j * 4 + k].min = start;
					subChunks[i * 16 + j * 4 + k].max = end;
					changed.insert(i * 16 + j * 4 + k);// .push_back(i * 16 + j * 4 + k);
					lodChanged = true;
				}
				start.x += delta;
				end.x += delta;
			}
			start.y += delta;
			end.y += delta;
		}
		start.z += delta;
		end.z += delta;
	}

	for (auto&& i : changed)
	{
		UCHAR face = GetTransVoxelFace(i, true);
		if (face > 0)
		{
			for (int q = 0; q < 6; ++q)
			{
				if (face & (1 << q))
				{
					UINT x = i & 0b11, y = (i >> 2) & 0b11, z = (i >> 4) & 0b11;
					XMUINT3 index(x, y, z);
					index += facePositionData[q];
					changed.insert(index.z * 16 + index.y * 4 + index.x);
				}
			}
		}
	}

	if (lodChanged || dirtyFlag)
	{
		if (lodChanged)
		{
			for (auto&& i : changed)
			{
				results.emplace_back(
					pool->Enqueue(
						[=]
						{
							GetTransVoxelIATask(subChunks[i].mesh->vertices, subChunks[i].mesh->indices, i);
							return (int)i;
						})
				);
			}
			//for (int i = 0; i < changed.size(); ++i)
			//{
			//	results.emplace_back(
			//		pool->Enqueue(
			//			[=]
			//			{
			//				GetTransVoxelIATask(subChunks[changed[i]].mesh->vertices, subChunks[changed[i]].mesh->indices, changed[i]);
			//				return (int)changed[i];
			//			})
			//	);
			//}
		}
		else
		{
			for (auto&& i : changed)//for (int i = 0; i < 64; ++i)
			{
				results.emplace_back(
					pool->Enqueue(
						[=]
						{
							GetTransVoxelIATask(subChunks[i].mesh->vertices, subChunks[i].mesh->indices, i);
							return i;
						})
				);
			}
		}
		lodChanged = false;
		changed.clear();

		for (int i = 0; i < results.size(); ++i)
		{
			auto&& result = results[i];
			result.wait();

			subChunks[result.get()].InitializeMeshBuffer(device);
		}
	}

	if (dirtyFlag)
	{
		dirtyFlag = false;
		for (int z = 0; z < chunkSize; ++z)
		{
			for (int y = 0; y < chunkSize; ++y)
			{
				for (int x = 0; x < chunkSize; ++x)
				{
					int bitFlag = GetVoxelBitFlag(XMUINT3(x, y, z), 0);
					voxels[x + y * chunkSize + z * chunkSize * chunkSize].type = voxelTypeTable[bitFlag];
				}
			}
		}
	}
}

void Voxel::GetTransVoxelIATask(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices, int startX, int endX)
{
	for (int z = 0; z < chunkSize; ++z)
	{
		for (int y = 0; y < chunkSize; ++y)
		{
			for (int x = startX; x < endX; ++x)
			{
				unsigned char bitFlag = GetVoxelBitFlag(XMUINT3(x, y, z));
				if (edgeTable[bitFlag] == 0) continue;

				RegularCellData rcd = regularCellData[regularCellClass[bitFlag]];
				int vc = rcd.GetVertexCount();
				int tc = rcd.GetTriangleCount();

				std::vector<unsigned char> vd;
				for (int i = 0; i < vc; ++i)
				{
					vd.push_back(regularVertexData[bitFlag][i]);
				}

				std::unordered_map<int, int> vertList;
				std::vector<int> idxList;

				int mask = (x >= 0 ? 1 : 0) | (z >= 0 ? 2 : 0) | (y >= 0 ? 4 : 0);
				for (int i = 0; i < vc; ++i)
				{
					int d = (vd[i] & 0xFF);
					int d0 = d >> 4;
					int d1 = d & 0x0F;

					int r = vd[i] >> 8;
					int direction = r >> 4;
					int index = r & 0x0F;

					int nIndex = -1;
					bool p = (direction & mask) == direction;

					XMUINT3 p0 = XMUINT3(x + vertexOffset[d0][0], y + vertexOffset[d0][1], z + vertexOffset[d0][2]);
					XMUINT3 p1 = XMUINT3(x + vertexOffset[d1][0], y + vertexOffset[d1][1], z + vertexOffset[d1][2]);

					float t0 = GetVoxelVertex(p0);
					float t1 = GetVoxelVertex(p1);
					if (t0 < 0.1f && t0 > 0.f)
						float a = 4;
					float interpt = (0.f - t0) / (t1 - t0);
					idxList.push_back(vertices.size());
					vertices.push_back(VertexInterp(p0, p1, interpt));
				}

				for (int i = 0; i < tc; ++i)
				{
					int t = i * 3;
					indices.push_back(idxList[rcd.vertexIndex[t]]);
					indices.push_back(idxList[rcd.vertexIndex[t + 1]]);
					indices.push_back(idxList[rcd.vertexIndex[t + 2]]);

					vertices[idxList[rcd.vertexIndex[t]]].uv = XMFLOAT2(0.f, 1.f);
					vertices[idxList[rcd.vertexIndex[t + 1]]].uv = XMFLOAT2(0.5f, 0.f);
					vertices[idxList[rcd.vertexIndex[t + 2]]].uv = XMFLOAT2(1.f, 1.f);

					//vertices[idxList[rcd.vertexIndex[t]]].color = XMFLOAT4(1.f, 0.f, 0.f, 1.f);
					//vertices[idxList[rcd.vertexIndex[t + 1]]].color = XMFLOAT4(0.f, 1.f, 0.f, 1.f);
					//vertices[idxList[rcd.vertexIndex[t + 2]]].color = XMFLOAT4(0.f, 0.f, 1.f, 1.f);

					XMFLOAT3 normal = MathHelper::NormalVector(vertices[idxList[rcd.vertexIndex[t]]].position, vertices[idxList[rcd.vertexIndex[t + 1]]].position, vertices[idxList[rcd.vertexIndex[t + 2]]].position);
					vertices[idxList[rcd.vertexIndex[t]]].normal += normal;
					vertices[idxList[rcd.vertexIndex[t + 1]]].normal += normal;
					vertices[idxList[rcd.vertexIndex[t + 2]]].normal += normal;
				}
			}
		}
	}
}

void Voxel::GetTransVoxelIATask(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices, XMUINT3 start, XMUINT3 end, int lodLevel)
{
	int delta = pow(2, lodLevel);
	if ((end - start).x < delta)
	{
		delta = (end - start).x;
		lodLevel = log2(delta);
	}
	for (int z = start.z; z < end.z; z += delta)
	{
		for (int y = start.y; y < end.y; y += delta)
		{
			for (int x = start.x; x < end.x; x += delta)
			{
				std::unordered_map<int, int> vertList;
				std::vector<int> idxList;

				//if (lodLevel == 0 || (x > 0 && x < (end.x - delta)) || (y > 0 && y < (end.y - delta)) || (z > 0 && z < (end.z - delta)))
				{
					unsigned char bitFlag = GetVoxelBitFlag(XMUINT3(x, y, z), lodLevel);
					if (edgeTable[bitFlag] == 0) continue;

					RegularCellData rcd = regularCellData[regularCellClass[bitFlag]];
					int vc = rcd.GetVertexCount();
					int tc = rcd.GetTriangleCount();

					std::vector<unsigned char> vd;
					for (int i = 0; i < vc; ++i)
					{
						vd.push_back(regularVertexData[bitFlag][i]);
					}

					//int mask = (x >= 0 ? 1 : 0) | (z >= 0 ? 2 : 0) | (y >= 0 ? 4 : 0);
					for (int i = 0; i < vc; ++i)
					{
						int d = (vd[i] & 0xFF);
						int d0 = d >> 4;
						int d1 = d & 0x0F;

						//int r = vd[i] >> 8;
						//int direction = r >> 4;
						//int index = r & 0x0F;

						//int nIndex = -1;
						//bool p = (direction & mask) == direction;

						XMUINT3 p0 = XMUINT3(x + vertexOffset[d0][0] * delta, y + vertexOffset[d0][1] * delta, z + vertexOffset[d0][2] * delta);
						XMUINT3 p1 = XMUINT3(x + vertexOffset[d1][0] * delta, y + vertexOffset[d1][1] * delta, z + vertexOffset[d1][2] * delta);

						float t0 = GetVoxelVertex(p0);
						float t1 = GetVoxelVertex(p1);
						//if (t0 < 0.1f && t0 > 0.f)
						//	float a = 4;
						float interpt = (0.f - t0) / (t1 - t0);
						idxList.push_back(vertices.size());
						vertices.push_back(VertexInterp(p0, p1, interpt));
					}

					for (int i = 0; i < tc; ++i)
					{
						int t = i * 3;
						indices.push_back(idxList[rcd.vertexIndex[t]]);
						indices.push_back(idxList[rcd.vertexIndex[t + 1]]);
						indices.push_back(idxList[rcd.vertexIndex[t + 2]]);

						vertices[idxList[rcd.vertexIndex[t]]].uv = XMFLOAT2(0.f, 1.f);
						vertices[idxList[rcd.vertexIndex[t + 1]]].uv = XMFLOAT2(0.5f, 0.f);
						vertices[idxList[rcd.vertexIndex[t + 2]]].uv = XMFLOAT2(1.f, 1.f);

						XMFLOAT3 normal = MathHelper::NormalVector(vertices[idxList[rcd.vertexIndex[t]]].position, vertices[idxList[rcd.vertexIndex[t + 1]]].position, vertices[idxList[rcd.vertexIndex[t + 2]]].position);
						vertices[idxList[rcd.vertexIndex[t]]].normal += normal;
						vertices[idxList[rcd.vertexIndex[t + 1]]].normal += normal;
						vertices[idxList[rcd.vertexIndex[t + 2]]].normal += normal;
					}
				}
				//else
				if (!(lodLevel == 0 || (x > 0 && x < (end.x - delta)) || (y > 0 && y < (end.y - delta)) || (z > 0 && z < (end.z - delta))))
				{
					unsigned char bitFlag = GetVoxelBitFlag(XMUINT3(x, y, z), lodLevel);
					if (transitionCellClass[bitFlag] == 0) continue;

					TransitionCellData tcd = transitionCellData[transitionCellClass[bitFlag] & 0x7F];
					int vc = tcd.GetVertexCount();
					int tc = tcd.GetTriangleCount();

					std::vector<unsigned char> vd;
					for (int i = 0; i < vc; ++i)
					{
						vd.push_back(transitionVertexData[bitFlag][i]);
					}

					//int mask = (x >= 0 ? 1 : 0) | (z >= 0 ? 2 : 0) | (y >= 0 ? 4 : 0);
					for (int i = 0; i < vc; ++i)
					{
						int d = (vd[i] & 0xFF);
						int d0 = d >> 4;
						int d1 = d & 0x0F;

						//int r = vd[i] >> 8;
						//int direction = r >> 4;
						//int index = r & 0x0F;

						//int nIndex = -1;
						//bool p = (direction & mask) == direction;

						XMUINT3 p0 = XMFLOAT3ToXMUINT3(XMFLOAT3(x, y, z) + transVertexOffset[0][d0] * delta);
						XMUINT3 p1 = XMUINT3(x + vertexOffset[d1][0] * delta, y + vertexOffset[d1][1] * delta, z + vertexOffset[d1][2] * delta);

						float t0 = GetVoxelVertex(p0);
						float t1 = GetVoxelVertex(p1);
						//if (t0 < 0.1f && t0 > 0.f)
						//	float a = 4;
						float interpt = (0.f - t0) / (t1 - t0);
						idxList.push_back(vertices.size());
						vertices.push_back(VertexInterp(p0, p1, interpt));
					}

					for (int i = 0; i < tc; ++i)
					{
						int t = i * 3;
						indices.push_back(idxList[tcd.vertexIndex[t]]);
						indices.push_back(idxList[tcd.vertexIndex[t + 1]]);
						indices.push_back(idxList[tcd.vertexIndex[t + 2]]);

						vertices[idxList[tcd.vertexIndex[t]]].uv = XMFLOAT2(0.f, 1.f);
						vertices[idxList[tcd.vertexIndex[t + 1]]].uv = XMFLOAT2(0.5f, 0.f);
						vertices[idxList[tcd.vertexIndex[t + 2]]].uv = XMFLOAT2(1.f, 1.f);

						XMFLOAT3 normal = MathHelper::NormalVector(vertices[idxList[tcd.vertexIndex[t]]].position, vertices[idxList[tcd.vertexIndex[t + 1]]].position, vertices[idxList[tcd.vertexIndex[t + 2]]].position);
						vertices[idxList[tcd.vertexIndex[t]]].normal += normal;
						vertices[idxList[tcd.vertexIndex[t + 1]]].normal += normal;
						vertices[idxList[tcd.vertexIndex[t + 2]]].normal += normal;
					}
				}
			}
		}
	}
}

void Voxel::GetTransVoxelIATask(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices, UINT subChunkIndex)
{
	XMUINT3 start = subChunks[subChunkIndex].min;
	XMUINT3 end = subChunks[subChunkIndex].max;
	int delta = pow(2, subChunks[subChunkIndex].lodLevel);
	if ((end - start).x < delta)
	{
		delta = (end - start).x;
		subChunks[subChunkIndex].lodLevel = log2(delta);
	}
	for (int z = start.z; z < end.z; z += delta)
	{
		for (int y = start.y; y < end.y; y += delta)
		{
			for (int x = start.x; x < end.x; x += delta)
			{
				std::vector<int> idxList;

				//if (subChunks[subChunkIndex].lodLevel == 0 || ((x > start.x && x < (end.x - delta)) && (y > start.y && y < (end.y - delta)) && (z > start.z && z < (end.z - delta))))
				{
					RegularCell(vertices, indices, subChunkIndex, XMUINT3(x, y, z));
				}
				//else
				if (!(subChunks[subChunkIndex].lodLevel == 0))// || (x > 0 && x < (end.x - delta)) || (y > 0 && y < (end.y - delta)) || (z > 0 && z < (end.z - delta))))
				{
					UCHAR face = GetTransVoxelFace(subChunkIndex);
					if (face > 0)
					{
						int drawCount = 0;
						for (int j = 0; j < 6; ++j)
						{
							if (face & (1 << j))
							{
								TransCell(vertices, indices, subChunkIndex, XMUINT3(x, y, z), j, drawCount);
							}
						}
						//if (drawCount == 0)
						//	RegularCell(vertices, indices, subChunkIndex, XMUINT3(x, y, z));
					}
					//else
					//{
					//	RegularCell(vertices, indices, subChunkIndex, XMUINT3(x, y, z));
					//}
				}
			}
		}
	}
}

void Voxel::RegularCell(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices, const UINT& subChunkIndex, const XMUINT3& indexPos)
{
	std::vector<int> idxList;
	int delta = pow(2, subChunks[subChunkIndex].lodLevel);

	unsigned char bitFlag = GetVoxelBitFlag(indexPos, subChunks[subChunkIndex].lodLevel);
	if (edgeTable[bitFlag] == 0)
	{
		voxels[indexPos.x + indexPos.y * chunkSize + indexPos.z * chunkSize * chunkSize].type = NONE;
		return;
	}

	RegularCellData rcd = regularCellData[regularCellClass[bitFlag]];
	int vc = rcd.GetVertexCount();
	int tc = rcd.GetTriangleCount();

	std::vector<unsigned char> vd;
	for (int i = 0; i < vc; ++i)
	{
		vd.push_back(regularVertexData[bitFlag][i]);
	}

	//int mask = (x >= 0 ? 1 : 0) | (z >= 0 ? 2 : 0) | (y >= 0 ? 4 : 0);
	for (int i = 0; i < vc; ++i)
	{
		int d = (vd[i] & 0xFF);
		int d0 = d >> 4;
		int d1 = d & 0x0F;

		//int r = vd[i] >> 8;
		//int direction = r >> 4;
		//int index = r & 0x0F;

		//int nIndex = -1;
		//bool p = (direction & mask) == direction;

		XMUINT3 p0 = XMUINT3(indexPos.x + vertexOffset[d0][0] * delta, indexPos.y + vertexOffset[d0][1] * delta, indexPos.z + vertexOffset[d0][2] * delta);
		XMUINT3 p1 = XMUINT3(indexPos.x + vertexOffset[d1][0] * delta, indexPos.y + vertexOffset[d1][1] * delta, indexPos.z + vertexOffset[d1][2] * delta);

		float t0 = GetVoxelVertex(p0);
		float t1 = GetVoxelVertex(p1);
		//if (t0 < 0.1f && t0 > 0.f)
		//	float a = 4;
		float interpt = (0.f - t0) / (t1 - t0);
		idxList.push_back(vertices.size());
		vertices.push_back(VertexInterp(p0, p1, interpt));
	}

	for (int i = 0; i < tc; ++i)
	{
		int t = i * 3;
		indices.push_back(idxList[rcd.vertexIndex[t]]);
		indices.push_back(idxList[rcd.vertexIndex[t + 1]]);
		indices.push_back(idxList[rcd.vertexIndex[t + 2]]);

		vertices[idxList[rcd.vertexIndex[t]]].uv = XMFLOAT2(0.f, 1.f);
		vertices[idxList[rcd.vertexIndex[t + 1]]].uv = XMFLOAT2(0.5f, 0.f);
		vertices[idxList[rcd.vertexIndex[t + 2]]].uv = XMFLOAT2(1.f, 1.f);

		XMFLOAT3 normal = MathHelper::NormalVector(vertices[idxList[rcd.vertexIndex[t]]].position, vertices[idxList[rcd.vertexIndex[t + 1]]].position, vertices[idxList[rcd.vertexIndex[t + 2]]].position);
		vertices[idxList[rcd.vertexIndex[t]]].normal += normal;
		vertices[idxList[rcd.vertexIndex[t + 1]]].normal += normal;
		vertices[idxList[rcd.vertexIndex[t + 2]]].normal += normal;
	}
}

void Voxel::TransCell(std::vector<MeshClass::VertexType>& vertices, std::vector<int>& indices, const UINT& subChunkIndex, const XMUINT3& indexPos, const int& face, int& drawCount)
{
	unsigned short bitFlag = GetTransVoxelBitFlag(indexPos, face, subChunks[subChunkIndex].lodLevel);
	if ((transitionCellClass[bitFlag] & 0x7F) == 0)
	{
		//RegularCell(vertices, indices, subChunkIndex, indexPos);
		return;
	}

	drawCount++;
	std::vector<int> idxList;
	int delta = pow(2, subChunks[subChunkIndex].lodLevel);

	int inv = (transitionCellClass[bitFlag] & 128) ? 1 : -1;
	TransitionCellData tcd = transitionCellData[transitionCellClass[bitFlag] & 0x7F];
	int vc = tcd.GetVertexCount();
	int tc = tcd.GetTriangleCount();

	std::vector<unsigned char> vd;
	for (int i = 0; i < vc; ++i)
	{
		vd.push_back(transitionVertexData[bitFlag][i]);
	}

	//int mask = (x >= 0 ? 1 : 0) | (z >= 0 ? 2 : 0) | (y >= 0 ? 4 : 0);
	for (int i = 0; i < vc; ++i)
	{
		int d = (vd[i] & 0xFF);
		int d0 = d >> 4;
		int d1 = d & 0x0F;

		//int r = vd[i] >> 8;
		//int direction = r >> 4;
		//int index = r & 0x0F;

		//int nIndex = -1;
		//bool p = (direction & mask) == direction;

		XMUINT3 p0 = XMFLOAT3ToXMUINT3(XMUINT3ToXMFLOAT3(indexPos) + transVertexOffset[face][d0] * delta);
		XMUINT3 p1 = XMFLOAT3ToXMUINT3(XMUINT3ToXMFLOAT3(indexPos) + transVertexOffset[face][d1] * delta);
		float t0 = GetVoxelVertex(p0);
		float t1 = GetVoxelVertex(p1);
		//if (t0 < 0.1f && t0 > 0.f)
		//	float a = 4;
		float interpt = (0.f - t0) / (t1 - t0);
		idxList.push_back(vertices.size());
		vertices.push_back(VertexInterp(p0, p1, interpt));
	}

	for (int i = 0; i < tc; ++i)
	{
		int t = i * 3;
		indices.push_back(idxList[tcd.vertexIndex[t + 1 + inv]]);
		indices.push_back(idxList[tcd.vertexIndex[t + 1]]);
		indices.push_back(idxList[tcd.vertexIndex[t + 1 - inv]]);

		vertices[idxList[tcd.vertexIndex[t + 1 + inv]]].uv = XMFLOAT2(0.f, 1.f);
		vertices[idxList[tcd.vertexIndex[t + 1]]].uv = XMFLOAT2(0.5f, 0.f);
		vertices[idxList[tcd.vertexIndex[t + 1 - inv]]].uv = XMFLOAT2(1.f, 1.f);

		XMFLOAT3 normal = MathHelper::NormalVector(vertices[idxList[tcd.vertexIndex[t + 1 + inv]]].position, vertices[idxList[tcd.vertexIndex[t + 1]]].position, vertices[idxList[tcd.vertexIndex[t + 1 - inv]]].position);
		//float dot = MathHelper::Dot(XMFLOAT3(0, -0.707, 0.707), normal);
		vertices[idxList[tcd.vertexIndex[t + 1 + inv]]].normal += normal;
		vertices[idxList[tcd.vertexIndex[t + 1]]].normal += normal;
		vertices[idxList[tcd.vertexIndex[t + 1 - inv]]].normal += normal;
	}
}

XMUINT3 Voxel::WorldPositionToLocalIndex(const XMFLOAT3& position)
{
	if ((position.x < this->position.x || (position.x > (this->position.x + chunkSize * cellSize)))
		|| (position.y < this->position.y || (position.y > (this->position.y + chunkSize * cellSize)))
		|| (position.z < this->position.z || (position.z > (this->position.z + chunkSize * cellSize))))
		return XMUINT3(-1, -1, -1);

	return XMUINT3((position.x - this->position.x) / cellSize, 
		(position.y - this->position.y) / cellSize,
		(position.z - this->position.z) / cellSize);
}

bool Voxel::CheckOutOfBoundary(const XMUINT3& pos)
{
	if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= chunkSize || pos.y >= chunkSize || pos.z >= chunkSize)
		return true;
	return false;
}

std::vector<VoxelNode*> Voxel::GetNeighbours(const XMUINT3& current, std::unordered_map<int, VoxelNode>& nodes)
{
	std::vector<VoxelNode*> result;

	for (int z = Clamp((int)current.z - 1, 0, (int)chunkSize - 1); z <= Clamp((int)current.z + 1, 0, (int)chunkSize - 1); ++z)
	{
		for (int y = Clamp((int)current.y - 1, 0, (int)chunkSize - 1); y <= Clamp((int)current.y + 1, 0, (int)chunkSize - 1); ++y)
		{
			for (int x = Clamp((int)current.x - 1, 0, (int)chunkSize - 1); x <= Clamp((int)current.x + 1, 0, (int)chunkSize - 1); ++x)
			{
				if (x == current.x && z == current.z)
					continue;

				if (nodes.find(x + y * chunkSize + z * chunkSize * chunkSize) == nodes.end())
				{
					nodes[x + y * chunkSize + z * chunkSize * chunkSize] = VoxelNode(&GetVoxelData(XMUINT3(x, y, z)), XMUINT3(x, y, z));
				}
				result.push_back(&nodes[x + y * chunkSize + z * chunkSize * chunkSize]);
				//result.push_back(VoxelNode(&GetVoxelData(XMUINT3(x, y, z)), XMUINT3(x, y, z)));
			}
		}
	}

	return result;
}

int Voxel::GetNeighbourDistance(const XMUINT3& first, const XMUINT3& second)
{
	XMUINT3 temp = first - second;
	int dx = fabsf(temp.x);
	int dy = fabsf(temp.y);
	int dz = fabsf(temp.z);

	return 0;
}

bool operator==(const VoxelNode& lhs, const VoxelNode& rhs)
{
	return (lhs.data == rhs.data);
}

bool Voxel::VoxelAStar(std::vector<XMFLOAT3>& path)
{
	path.clear();

	if (CheckOutOfBoundary(navStart) || CheckOutOfBoundary(navEnd))
		return false;

	VoxelNode startNode = {&GetVoxelData(navStart), navStart };
	VoxelNode endNode = { &GetVoxelData(navEnd), navEnd };

	PriorityQueue<VoxelNode> openSet;// chunkSize* chunkSize* chunkSize);
	std::unordered_set<VoxelNode, hash_fn> closedSet;
	std::vector<VoxelNode> closedList;
	std::unordered_map<int, VoxelNode> voxelNodes;
	openSet.Enqueue(startNode);

	while (openSet.Count() > 0)
	{
		VoxelNode currentNode = openSet.Dequeue();

		if (currentNode == endNode)
		{
			while (!(currentNode == startNode))
			{
				path.push_back(GetCenterPositionFromIndex(currentNode.index) + XMFLOAT3(0.f, 0.f, 0.f));
				currentNode = closedList[currentNode.parent];//*currentNode.parent;
			}
			path.push_back(GetCenterPositionFromIndex(startNode.index) + XMFLOAT3(0.f, 0.f, 0.f));
			//PathOptimization(path);
			return true;
		}

		closedSet.insert(currentNode);

		closedList.push_back(currentNode);

		for (auto&& n : GetNeighbours(currentNode.index, voxelNodes))
		{
			unsigned char upBitFlag = GetVoxelBitFlag(n->index + XMUINT3(0, 1, 0));
			unsigned char downBitFlag = GetVoxelBitFlag(n->index + XMUINT3(0, -2, 0));
			if (closedSet.find(*n) != closedSet.end() || n->data->type != LAND || 
				(upBitFlag != 0 && upBitFlag != 255) || (downBitFlag != 0 && downBitFlag != 255))
				continue;

			float gCost = currentNode.gCost + Distance(GetCenterPositionFromIndex(currentNode.index), GetCenterPositionFromIndex(n->index));
			float hCost = Distance(GetCenterPositionFromIndex(endNode.index), GetCenterPositionFromIndex(n->index));

			if (!openSet.Contains(*n))
			{
				n->gCost = gCost;
				n->hCost = hCost;
				n->parent = closedList.size() - 1;//&currentNode;
				openSet.Enqueue(*n);
			}
			else
			{
				if (n->fCost() > gCost + hCost || (n->fCost() == gCost + hCost && n->gCost > gCost))
				{
					n->gCost = gCost;
					n->hCost = hCost;
					n->parent = closedList.size() - 1;// &currentNode;
				}
			}
		}
	}

	path.clear();
	return false;
}

void Voxel::PathOptimization(std::vector<XMFLOAT3>& path)
{
	if (path.size() <= 2)
		return;

	std::reverse(path.begin(), path.end());
	XMFLOAT3 start = path[0];
	XMFLOAT3 end = path[path.size() - 1];

	std::vector<XMFLOAT3> newPath;
	newPath.push_back(start);

	for (auto i = path.begin() + 1; i != path.end() - 1; ++i)
	{
		XMFLOAT3 origin = start;
		XMFLOAT3 target = *(i + 0);

		Ray3D ray(origin + XMFLOAT3(0.f, cellSize * 2.f, 0.f), target + XMFLOAT3(0.f, cellSize * 2.f, 0.f));

		XMFLOAT3 out = XMFLOAT3(0, 0, 0);
		unsigned char downFlag = GetVoxelBitFlag(WorldPositionToLocalIndex(target) - XMUINT3(0, 1, 0));
		if (RayCast(ray, out) || (voxelTypeTable[downFlag] != LAND && (downFlag != 0 && downFlag != 255)))//downFlag != 0 && downFlag != 255))
		{
			newPath.push_back(*i);
			start = (*i);
		}
	}

	newPath.push_back(end);
	path.swap(newPath);

	if (path.size() < 3)
		return;

	newPath.clear();

	//newPath.push_back(path[0]);
	for (std::vector<XMFLOAT3>::iterator i = path.begin(); i != (path.end() - 1); i++)
	{
		XMVECTOR v[4];
		if (i == path.begin())
		{
			v[0] = XMLoadFloat3(&path[0]);
			v[3] = XMLoadFloat3(&(*(i + 2)));
		}
		else if (i == (path.end() - 2))
		{
			v[0] = XMLoadFloat3(&(*(i - 1)));
			v[3] = XMLoadFloat3(&(*(i + 1)));
		}
		else
		{
			v[0] = XMLoadFloat3(&(*(i - 1)));
			v[3] = XMLoadFloat3(&(*(i + 2)));
		}

		v[1] = XMLoadFloat3(&(*i));
		v[2] = XMLoadFloat3(&(*(i + 1)));
		for (int i = 0; i < 11; ++i)
		{
			float dt = 0.02f * i;
			XMFLOAT3 p;
			XMStoreFloat3(&p, XMVectorCatmullRom(v[0], v[1], v[2], v[3], dt));
			newPath.push_back(p);
		}

		for (int i = 0; i < 11; ++i)
		{
			float dt = 0.02f * i;
			XMFLOAT3 p;
			XMStoreFloat3(&p, XMVectorCatmullRom(v[0], v[1], v[2], v[3], 0.8f + dt));
			newPath.push_back(p);
		}
	}

	path.swap(newPath);
}

void Voxel::SetNavStart(XMFLOAT3 pos)
{
	navStart = WorldPositionToLocalIndex(pos);
}

void Voxel::SetNavStart(XMUINT3 index)
{
	navStart = index;
}

void Voxel::SetNavEnd(XMFLOAT3 pos)
{
	navEnd = WorldPositionToLocalIndex(pos);
}

void Voxel::SetNavEnd(XMUINT3 index)
{
	navEnd = index;
}

bool Voxel::InitializeBuffers(ID3D11Device* device)
{
	//if (!this->device)
	//	this->device = device;

	//// Maximum triangle count is 5 for each voxel
	//std::vector<Voxel::VertexType> vertices;
	//std::vector<int> indices;

	////GetTransVoxelIA(vertices, indices);
	////GetTransVoxelIAThreadPool(vertices, indices);
	////GetVoxelIA(vertices, indices);

	//if (vertices.empty() || indices.empty())
	//	return true;

	//// Initialize vertex buffer desc
	//D3D11_BUFFER_DESC vbd;
	//vbd.Usage = D3D11_USAGE_DEFAULT;
	////vbd.ByteWidth = sizeof(VertexType) * (12 * (chunkSize.x + 1) * (chunkSize.y + 1) * (chunkSize.z + 1));
	//vbd.ByteWidth = sizeof(VertexType) * vertices.size();
	//vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//vbd.CPUAccessFlags = 0;
	//vbd.MiscFlags = 0;
	//vbd.StructureByteStride = 0;

	//// Set ptr to subresource about vertext data
	//D3D11_SUBRESOURCE_DATA vertexData;
	//vertexData.pSysMem = &vertices[0];
	//vertexData.SysMemPitch = 0;
	//vertexData.SysMemSlicePitch = 0;

	//// Create vertex buffer
	//if (FAILED(device->CreateBuffer(&vbd, &vertexData, &vb)))
	//	return false;

	//// Initialize index buffer desc
	//D3D11_BUFFER_DESC ibd;
	//ibd.Usage = D3D11_USAGE_DEFAULT;
	////ibd.ByteWidth = sizeof(ULONG) * (15 * (chunkSize.x + 1) * (chunkSize.y + 1) * (chunkSize.z + 1));
	//ibd.ByteWidth = sizeof(ULONG) * indices.size();
	//ibd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//ibd.CPUAccessFlags = 0;
	//ibd.MiscFlags = 0;
	//ibd.StructureByteStride = 0;

	//// Set ptr to subresource about index data
	//D3D11_SUBRESOURCE_DATA indexData;
	//indexData.pSysMem = &indices[0];
	//indexData.SysMemPitch = 0;
	//indexData.SysMemSlicePitch = 0;

	//// Create index buffer
	//if (FAILED(device->CreateBuffer(&ibd, &indexData, &ib)))
	//	return false;

	////UCHAR f = GetVoxelBitFlag(XMUINT3(chunkSize / 2, 0, chunkSize / 2));
	////UCHAR f1 = GetVoxelBitFlag(XMUINT3(chunkSize / 2, 1, chunkSize / 2));
	//
	//vCount = vertices.size();
	//iCount = indices.size();

	//// Release vertices, indices
	//vertices.clear();
	//indices.clear();

	return true;
}

bool Voxel::UpdateBuffers(ID3D11DeviceContext* context)
{
	ShutdownBuffers();

	return InitializeBuffers(device);
}

void Voxel::ShutdownBuffers()
{
	// Release index buffer
	if (ib)
	{
		ib->Release();
		ib = nullptr;
	}

	// Release vertex buffer
	if (vb)
	{
		vb->Release();
		vb = nullptr;
	}
}

void Voxel::RenderBuffers(ID3D11DeviceContext* context, const int& index)
{
	//Update();
	subChunks[index].mesh->Render(context);
}

//void Voxel::RenderBuffers(ID3D11DeviceContext* context)
//{
//	GetTransVoxelIAThreadPool();
//	for (int i = 0; i < 64; ++i)
//	{
//		subChunks[i].mesh->Render(context);
//	}
//	//// if voxel data changed then update buffer before render
//	////if (dirtyFlag)
//	//{
//	//	dirtyFlag = false;
//	//	if (!UpdateBuffers(context)) return;
//	//}
//
//	//// Set stride and offset of vertex buffer
//	//UINT stride = sizeof(VertexType);
//	//UINT offset = 0;
//
//	//// Activate vertex buffer to render from IA
//	//context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
//
//	//// Activate index buffer to render from IA
//	//context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
//
//	//// Set primitive topology
//	//context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//}

bool Voxel::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* context, LPCSTR fileName)
{
	// Create texture object
	texture = new TargaTextureClass;
	if (!texture)
		return false;

	// Initialize texture object
	return texture->Initialize(device, context, fileName);
}

void Voxel::ReleaseTexture()
{
	// Release texture object
	if (texture)
	{
		texture->Shutdown();
		delete texture;
		texture = nullptr;
	}
}
