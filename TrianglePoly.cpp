
#include "TrianglePolyDemo.h"
#include "primitives.h"

using namespace std;

bool TrianglePolyDemo::Calculate(void)
{
	// manually encode a concave polygon.
	polygon = { { {60,180}, {30,25}, {160,20}, {156,100},{185,125}, {162,120}, {170,210}, {120,205}, {160,200}, {135,67}, {125,65} } };

	// manually generate 4 triangles
	triangles.push_back({ { {25,20}, {10, 40}, {39, 84} } });
	triangles.push_back({ { {40,210}, {60, 160}, {90, 170} } });
	triangles.push_back({ { {120,100}, {130, 81}, {138, 110} } });
	triangles.push_back({ { {80,40}, {100, 47}, {90, 60} } });

	// randomly generate 996 more
	AddRandomTriangles(NUM_TRIANGLES-4);

	// test all triangles
	auto time_start = chrono::high_resolution_clock::now();
	for (auto i = 0; i < triangles.size(); ++i)
		triangle_hit_results.push_back(polygon.Intersects(triangles[i]));

	// calculate total time
	auto time_end = chrono::high_resolution_clock::now();
	duration = chrono::duration_cast<chrono::microseconds>(time_end - time_start);

	// success
	nSelected = 0;
	return true;
}

// recalculate intersection for a specific triangle
void TrianglePolyDemo::Recalculate(int index) {
	triangle_hit_results[index] = polygon.Intersects(triangles[index]);
}


void TrianglePolyDemo::AddRandomTriangles(int cnt)
{
	const int VERTEX_MAX = 75;
	const int VERTEX_MIN = 0;
	const int VERTEX_RANGE = VERTEX_MAX - VERTEX_MIN;
	const int ORIGIN_RANGE = WORLDSPACE_EXTENT - VERTEX_MAX - 1;

	// note: not checking for degenerate case of 3 colinear points
	for (auto i = 0; i < cnt; ++i) {
		int offset = VERTEX_MIN + (rand() % ORIGIN_RANGE);
		triangles.push_back({ {
			{ offset + rand() % VERTEX_RANGE, offset + rand() % VERTEX_RANGE },
			{ offset + rand() % VERTEX_RANGE, offset + rand() % VERTEX_RANGE },
			{ offset + rand() % VERTEX_RANGE, offset + rand() % VERTEX_RANGE } } });
	}
}
