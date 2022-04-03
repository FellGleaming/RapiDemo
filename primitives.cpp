#include "primitives.h"

const bool CPolygon::Contains(const Point& pt)
{
	if (!_normalized)
		this->Normalize();

	// cull point against bounding box
	if ( (pt.x < _bound_box.ll.x) || (pt.x > _bound_box.tr.x) ) return false;
	if ( (pt.y < _bound_box.ll.y) || (pt.y > _bound_box.tr.y) ) return false;

	Line testray, edge;
	int last_found = 0;
	int inter_count = 0;

	// horizontal test ray
	testray.p0 = pt; testray.p1.y = pt.y; testray.p1.x = COORDINATE_MAX;

	for (auto i = 1; i < pts.size(); ++i) {
		edge.p0 = pts[i]; edge.p1 = pts[i];
		if (!segment_intersect(edge, testray))
		{
			edge.p1 = pts[last_found]; last_found = i;
			if ( segment_intersect(edge,testray) ) ++inter_count;
		}
	}

	// return true if count is even
	return (inter_count & 1);
}

const bool CPolygon::Intersects(const Triangle& t)
{
	if ( !_normalized )
		this->Normalize();

	// on the assumption that the polygon is larger than triangle(s), the triangle
	// in outer loop should be marginally faster performance
	for (auto i = 0; i < 3; ++i) {
		int iprev = i ? i - 1 : 2;
		Line line1 = { { t.pt[iprev].x, t.pt[iprev].y}, { t.pt[i].x, t.pt[i].y } };

		for (auto j = 0; j < pts.size(); ++j) {
			int jprev = j ? j - 1 : (int)pts.size()-1;
			Line line2 = { { pts[jprev].x, pts[jprev].y}, { pts[j].x, pts[j].y } };
			if (segment_intersect(line1, line2) )
				return true;
		}
	}

	// check each vertex of the triangle
	/*for ( auto i=0 ; i<3 ; ++i )
		if ( this->Contains(t.pt[i]) )
			return true; */

	// we still have 2 corner cases of one wholly inside other: handle by testing whether (any) triangle
	// vertex is inside.  The 2nd corner case we ignore, based on assumption that our Polygon >> triangle
	if ( Contains(t.pt[0]) )
		return true;

	return false;
}

// add 'guard' vertex points, calculate bounding box
void CPolygon::Normalize(void)
{
	auto it = ::std::begin(pts);
	auto low = it;

	_bound_box.ll.x = _bound_box.ll.y = COORDINATE_MAX;
	_bound_box.tr.x = _bound_box.tr.y = COORDINATE_MIN;

	// find rotate point (lowest y/x vertex) & calculate bounding box
	for ( ; it != std::end(pts); ++it) {
		if ((it->y < low->y) || (it->y == low->y && it->x < low->x))
			low = it;
		if (_bound_box.ll.x > it->x) _bound_box.ll.x = it->x;
		if (_bound_box.ll.y > it->y) _bound_box.ll.y = it->y;
		if (_bound_box.tr.x < it->x) _bound_box.tr.x = it->x;
		if (_bound_box.tr.y < it->y) _bound_box.tr.y = it->y;
	}
	
	// rotate lowest-y point to begin of vertex list
	std::rotate(pts.begin(), low, pts.end());

	// add guard points - not strictly needed, but
	// simplifies the loop logic.  
	pts.insert(pts.begin(), pts[pts.size()-1]);
	pts.push_back(pts[1]);

	_normalized = true;
}

//
// classic clockwise/counter-clockwise winding primitive 
// return is 3-valued logic: 0 if colinear points & and p2 on p0-p1 segment 
//
int wind(const Point& pnt0, const Point& pnt1, const Point& pnt2) 
{
	Point d1, d2;

	// calculate delta x/y components
	d1.x = pnt1.x - pnt0.x; d1.y = pnt1.y - pnt0.y;
	d2.x = pnt2.x - pnt0.x; d2.y = pnt2.y - pnt0.y;
	if (d1.x * d2.y > d2.x * d1.y)
		return 1;
	if (d1.x * d2.y < d1.y * d2.x)
		return -1;
	if ( (d1.x * d2.x < 0) || (d1.y * d2.y) < 0 )
		return -1;
	if ( (d1.x * d1.x + d1.y * d1.y) < (d2.x * d2.x + d2.y * d2.y) )
		return 1;
	return  0;
}

//
// segment intersection primitive
//
bool segment_intersect(const Line& line1, const Line& line2)
{
	return ((wind(line1.p0, line1.p1, line2.p0) * wind(line1.p0, line1.p1, line2.p1)) <= 0)
		&& ((wind(line2.p0, line2.p1, line1.p0) * wind(line2.p0, line2.p1, line1.p1)) <= 0);
}
