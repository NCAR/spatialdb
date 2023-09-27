#include "SpatiaLiteDB.h"
#include <iostream>
#include <algorithm>

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::Point::Point(double x, double y, std::string label):
_x(x),
_y(y),
_label(label)
{
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
SpatiaLiteDB::Linestring::Linestring(std::string label):
_label(label)
{

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
SpatiaLiteDB::Polygon::Polygon(std::string label):
_label(label)
{

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
SpatiaLiteDB::Ring SpatiaLiteDB::Polygon::extRing() {
	return _extRing;
}

////////////////////////////////////////////////////////////////////
std::vector<SpatiaLiteDB::Ring> SpatiaLiteDB::Polygon::intRings() {
	return _intRings;
}

////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& lhs, SpatiaLiteDB::Polygon& rhs) {
	lhs << rhs._extRing;

	for (unsigned int i = 0;
			i < rhs._intRings.size();
			i++) {
		lhs << rhs._intRings[i];
	}
	return lhs;
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::PointList::PointList()
{

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::PointList::~PointList() {

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::LinestringList::LinestringList()
{

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::LinestringList::~LinestringList() {

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::PolygonList::PolygonList(std::string /*label*/)
{

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::PolygonList::~PolygonList() {

}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::SpatiaLiteDB(std::string dbPath) :
SQLiteDB(dbPath, true)
{
	// initialize spatiaLite.
	_connection = spatialite_alloc_connection();

	SQLiteDB::init();

	spatialite_init_ex(handle(), _connection, true);
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::~SpatiaLiteDB() {

	for (unsigned int i = 0; i < _geoms.size(); i++) {
		gaiaFreeGeomColl(_geoms[i]);
	}

	SQLiteDB::close();

	spatialite_cleanup_ex(_connection);
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
		double top,
		std::string label_col) {

	_pointlist.clear();
	_linestringlist.clear();
	_polygonlist.clear();
	bool getLabel = label_col.size() != 0;

	// Search for geometry and name

	// The query will be either
	//
	// SELECT Geometry FROM  table WHERE MbrWithin(Column, BuildMbr(left, bottom, right, top));
	//    or
	// SELECT Geometry,Label FROM  table WHERE MbrWithin(Column, BuildMbr(left, bottom, right, top));
	//
	// where Geometry is the geometry column name and Label is the column containing a name or label.
	std::string label;
	if (getLabel) {
		label = "," + label_col;
	}
	std::stringstream s;
	s <<
		 "SELECT "           <<
		 geometry_col        <<
		 label               <<
		 " FROM "            <<
		 table               <<
		 " WHERE  MbrIntersects(" <<
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
		// get the label, if we asked for it
		std::string label;
		if (getLabel) {
			label = Text(1);
		}
		// the following will create lists for points, lines and polygons found in
		// a single geometry. In practice it seems that only one of those types
		// exists for a given geometry, but it's not clear if this is a requirement
		// or just occurs in the sample databases we have been working with.
		PointList points = point_list(geom, label);
		for (PointList::iterator i = points.begin(); i != points.end(); i++) {
			_pointlist.push_back(*i);
		}
		LinestringList lines = linestring_list(geom, label);
		for (LinestringList::iterator i = lines.begin(); i != lines.end(); i++) {
			_linestringlist.push_back(*i);
		}
		PolygonList polys = polygon_list(geom, label);
		for (PolygonList::iterator i = polys.begin(); i != polys.end(); i++) {
			_polygonlist.push_back(*i);
		}
	}

	// finish with this query
	finalize();
}

////////////////////////////////////////////////////////////////////
gaiaGeomCollPtr SpatiaLiteDB::Geom(int col) {

	// throw an exception if the requested column doesn't exist
	checkColumn(SQLITE_BLOB, col);

	const void* blob;
	int blob_size;
	gaiaGeomCollPtr geom;

	blob = Blob(col, blob_size);

	geom = gaiaFromSpatiaLiteBlobWkb((const unsigned char*) blob, blob_size);

	// Save geom for later cleanup.
	_geoms.push_back(geom);

	return geom;
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::PointList SpatiaLiteDB::point_list(gaiaGeomCollPtr geom, std::string label) {

	SpatiaLiteDB::PointList pl;

	gaiaPointPtr point = geom->FirstPoint;

	while (point) {
		pl.push_back(SpatiaLiteDB::Point(point->X, point->Y, label));
		point = point->Next;
	}
	return pl;
}

////////////////////////////////////////////////////////////////////
SpatiaLiteDB::LinestringList SpatiaLiteDB::linestring_list(gaiaGeomCollPtr geom, std::string label) {

	SpatiaLiteDB::LinestringList ll;

	gaiaLinestringPtr linestring = geom->FirstLinestring;

	while (linestring) {
		Linestring ls(label);
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
SpatiaLiteDB::PolygonList SpatiaLiteDB::polygon_list(gaiaGeomCollPtr geom, std::string label) {

	double x;
	double y;
	SpatiaLiteDB::PolygonList polylist;

	gaiaPolygonPtr polygon = geom->FirstPolygon;

	while (polygon) {
		Polygon p(label);

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
std::vector<std::string> SpatiaLiteDB::geometryTables() {

	std::vector<std::string> all_tables = table_names();

	// find all tables containing a geometry column
	std::vector<std::string> geometry_tables;

	for (std::vector<std::string>::iterator table = all_tables.begin();
			table != all_tables.end(); table++) {

		std::vector<std::string> columns = column_names(*table);

		for (std::vector<std::string>::iterator column = columns.begin();
				column != columns.end(); column++) {
			std::transform(column->begin(), column->end(), column->begin(),
					(int(*)(int))std::tolower);

			if (!column->compare("geometry")) {
				geometry_tables.push_back(*table);
			}
		}
	}

	return geometry_tables;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
