#pragma once
#include <algorithm>
#include <chrono>

#include <Windows.h>
#include <wingdi.h>

#include "primitives.h"


const int NUM_TRIANGLES = 50000;
const int WORLDSPACE_EXTENT = 250;			// all world geometry pts should be < this or rendor window will clip

class TrianglePolyDemo {
public:
	CPolygon polygon;
	std::vector<Triangle> triangles;
	std::vector<bool> triangle_hit_results;
	std::chrono::duration<long, std::micro> duration;
	int nSelected;

	bool Calculate(void);
	void Recalculate(int index);
	void AddRandomTriangles(int cnt);
};