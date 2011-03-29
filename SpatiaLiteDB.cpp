#include "SpatiaLiteDB.h"
#include <iostream>

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::Point::Point(double x, double y) {
	_x = x;
	_y= y;
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::Point::~Point() {

}

////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& lhs, SpatiaLiteDB::Point& rhs) {
	lhs << "(" << rhs._x << "," << rhs._y << ")";
	return lhs;
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::Linestring::Linestring() {

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::Linestring::~Linestring() {

}

////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& lhs, SpatiaLiteDB::Linestring& rhs) {
	for (SpatiaLiteDB::Linestring::iterator i = rhs.begin();
			i != rhs.end();
			i++) {
		lhs << *i;
	}
	return lhs;
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::Ring::Ring():
	_clockwise(0)
{
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::Ring::Ring(int clockwise):
	_clockwise(clockwise)
{
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::Ring::~Ring() {

}

////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& lhs, SpatiaLiteDB::Ring& rhs) {
	for (SpatiaLiteDB::Ring::iterator i = rhs.begin();
			i != rhs.end();
			i++) {
		lhs << *i;
	}
	return lhs;
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::Polygon::Polygon() {

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::Polygon::~Polygon() {

}

////////////////////////////////////////////////////////////////////
void SpatiaLiteDB::Polygon::setExtRing(Ring r) {
	_extRing = r;
}

////////////////////////////////////////////////////////////////////
void SpatiaLiteDB::Polygon::addIntRing(Ring r) {
	_intRings.push_back(r);
}

////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& lhs, SpatiaLiteDB::Polygon& rhs) {
	lhs << rhs._extRing;

	for (int i = 0;
			i < rhs._intRings.size();
			i++) {
		lhs << rhs._intRings[i];
	}
	return lhs;
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::PointList::PointList() {

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::PointList::~PointList() {

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::LinestringList::LinestringList() {

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::LinestringList::~LinestringList() {

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::PolygonList::PolygonList() {

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::PolygonList::~PolygonList() {

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::SpatiaLiteDB(std::string dbPath) throw (std::string) :
SQLiteDB(dbPath, true)
{

	// initialize spatiaLite.
	/// @todo Does this need to be a singleton for the process?
	spatialite_init(0);

	SQLiteDB::init();

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::~SpatiaLiteDB() {

}

////////////////////////////////////////////////////////////////////
std::string SpatiaLiteDB::version() {
	std::stringstream s;
	s << SQLiteDB::version() << ", ";
	s << "SpatiaLite version: " << spatialite_version();

	return s.str();
}

////////////////////////////////////////////////////////////////////
void SpatiaLiteDB::queryGeometry(
		std::string table,
		std::string geometry_col,
		double left,
		double bottom,
		double right,
		double top) {

	_pointlist.clear();
	_linestringlist.clear();
	_polygonlist.clear();

	// Search for geometry and name

	// The query will be:
	// SELECT Geometry FROM  table WHERE MbrWithin(Column, BuildMbr(-180.0, -90.0, 180.0, 90.0));

	std::stringstream s;
	s <<
		 "SELECT "           <<
		 geometry_col        <<
		 " FROM "           <<
		 table               <<
		 " WHERE  MbrWithin(" <<
		 geometry_col        <<
		 ", BuildMbr("         <<
		 left                <<
		 ","                 <<
		 bottom              <<
		 ","                 <<
		 right               <<
		 ","                 <<
		 top                 <<
		 "));";

	// prepare the query
	prepare(s.str());

	// fetch the next result row
	while (step()) {
		// get the geometry
		gaiaGeomCollPtr geom = Geom(0);
		PointList points = point_list(geom);
		for (PointList::iterator i = points.begin(); i != points.end(); i++) {
			_pointlist.push_back(*i);
		}
		LinestringList lines = linestring_list(geom);
		for (LinestringList::iterator i = lines.begin(); i != lines.end(); i++) {
			_linestringlist.push_back(*i);
		}
		PolygonList polys = polygon_list(geom);
		for (PolygonList::iterator i = polys.begin(); i != polys.end(); i++) {
			_polygonlist.push_back(*i);
		}
	}

	// finish with this query
	finalize();
}

////////////////////////////////////////////////////////////////////
gaiaGeomCollPtr SpatiaLiteDB::Geom(int col) throw (std::string) {

	// throw an exception if the requested column doesn't exist
	checkColumn(SQLITE_BLOB, col);

	const void* blob;
	int blob_size;
	gaiaGeomCollPtr geom;

	blob = Blob(col, blob_size);
	/// @todoDoes geom need to be freed at some time?
	geom = gaiaFromSpatiaLiteBlobWkb((const unsigned char*) blob, blob_size);

	return geom;
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::PointList SpatiaLiteDB::point_list(gaiaGeomCollPtr geom) {

	SpatiaLiteDB::PointList pl;

	gaiaPointPtr point = geom->FirstPoint;

	while (point) {
		pl.push_back(SpatiaLiteDB::Point(point->X, point->Y));
		point = point->Next;
	}

	return pl;
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::LinestringList SpatiaLiteDB::linestring_list(gaiaGeomCollPtr geom) {

	SpatiaLiteDB::LinestringList ll;

	gaiaLinestringPtr linestring = geom->FirstLinestring;

	while (linestring) {
		Linestring ls;
		for (int i = 0; i < linestring->Points; i++) {
			double x;
			double y;
			gaiaGetPoint(linestring->Coords, i, &x, &y);
			ls.push_back(SpatiaLiteDB::Point(x, y));
		}
		ll.push_back(ls);
		linestring = linestring->Next;
	}

	return ll;
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::PolygonList SpatiaLiteDB::polygon_list(gaiaGeomCollPtr geom) {

	double x;
	double y;
	SpatiaLiteDB::PolygonList polylist;

	gaiaPolygonPtr polygon = geom->FirstPolygon;

	while (polygon) {
		Polygon p;

		gaiaRingPtr rExt = polygon->Exterior;
		Ring r(rExt->Clockwise);
		for (int i = 0; i < rExt->Points; i++) {
			gaiaGetPoint(rExt->Coords, i, &x, &y);
			r.push_back(SpatiaLiteDB::Point(x, y));
		}
		p.setExtRing(r);

		gaiaRingPtr rInt = polygon->Interiors;
		while (rInt) {
			Ring r(rInt->Clockwise);
			for (int i = 0; i < rInt->Points; i++) {
				gaiaGetPoint(rInt->Coords, i, &x, &y);
				r.push_back(SpatiaLiteDB::Point(x, y));
			}
			p.addIntRing(r);
			rInt = rInt->Next;
		}

		polylist.push_back(p);

		polygon = polygon->Next;
	}

	return polylist;

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::PointList SpatiaLiteDB::points() {
	return _pointlist;
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::LinestringList SpatiaLiteDB::linestrings() {
	return _linestringlist;
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::PolygonList SpatiaLiteDB::polygons() {
	return _polygonlist;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
