#pragma once

#include <iterator>
#include <algorithm>
#include <vector>

const int COORDINATE_MIN = 0;
const int COORDINATE_MAX = (2<<(sizeof(int)*4))-1;  

class Point {
public:
	long x, y;
};

class Line {
public:
	Point p0, p1;
};

class Rect {
public:
	Point ll, tr; // lower-left, top-right corners
};


// aesthetically, one may prefer to derive these from an abstract shape class
class Triangle
{
public:
	Point pt[3];
};

// generic n-gon
class CPolygon
{
private:
	bool _normalized=false;
public:
	CPolygon() = default;
	CPolygon(std::vector<Point> vertices) : pts{ { vertices } }, _normalized(false) { Normalize(); }
	std::vector<Point> pts;
	Rect _bound_box;

	// shifts vertices, calculates bounding box; must be called before geometry calculations
	void Normalize(void);	

	const bool Contains(const Point& p);
	const bool Intersects(const Triangle& t);
};


// non-class primitives
int wind(const Point& pnt0, const Point& pnt1, const Point& pnt2);
bool segment_intersect(const Line& line1, const Line& line2);