#ifndef SPATIALITEDB_INC
#define SPATIALITEDB_INC

#include "SQLiteDB.h"

#include <spatialite/gaiageo.h>
#include <spatialite.h>
#include <ostream>

class SpatiaLiteDB: public SQLiteDB {

public:
	class Point {
	public:
		Point(double x, double y);
		virtual ~Point();
		friend std::ostream& operator<<(std::ostream &lhs, Point& rhs);
		double _x;
		double _y;
	};


	class Linestring: public std::vector<Point>  {
	public:
		Linestring();
		virtual ~Linestring();
		friend std::ostream& operator<<(std::ostream &lhs, Linestring& rhs);
	};

	class Ring: public std::vector<Point>  {
	public:
		Ring();
		Ring(int clockwise);
		virtual ~Ring();
		friend std::ostream& operator<<(std::ostream &lhs, Ring& rhs);
		int _clockwise;
	};

	class Polygon {
	public:
		Polygon();
		virtual ~Polygon();
		void setExtRing(Ring r);
		void addIntRing(Ring r);
		Ring extRing();
		std::vector<Ring> intRings();
		friend std::ostream& operator<<(std::ostream &lhs, Polygon& rhs);
	protected:
		Ring _extRing;
		std::vector<Ring> _intRings;
	};

	class PointList: public std::vector<Point> {
	public:
		PointList();
		virtual ~PointList();
	};

	class LinestringList: public std::vector<Linestring> {
	public:
		LinestringList();
		virtual ~LinestringList();
	};

	class PolygonList: public std::vector<Polygon> {
	public:
		PolygonList();
		virtual ~PolygonList();
	};

public:
	SpatiaLiteDB(std::string dbPath) throw (std::string);
	virtual ~SpatiaLiteDB();
	/// @return Return a string with version information
	std::string version();
	///
	void queryGeometry(
			std::string table,
			std::string geometry_col,
			double left,
			double bottom,
			double right,
			double top);

	///
	PointList points();
	///
	LinestringList linestrings();
	///
	PolygonList polygons();

protected:
	/// @param col The column to be extracted.
	/// @return A pointer to geometry collection from the selected column of the current row.
	gaiaGeomCollPtr Geom(int col) throw (std::string);
	///
	PointList point_list(gaiaGeomCollPtr);
	///
	LinestringList linestring_list(gaiaGeomCollPtr);
	///
	PolygonList polygon_list(gaiaGeomCollPtr);


	PointList      _pointlist;
	LinestringList _linestringlist;
	PolygonList    _polygonlist;

};

std::ostream& operator<<(std::ostream& lhs, SpatiaLiteDB::Point &rhs);
std::ostream& operator<<(std::ostream& lhs, SpatiaLiteDB::Linestring &rhs);
std::ostream& operator<<(std::ostream& lhs, SpatiaLiteDB::Ring &rhs);
std::ostream& operator<<(std::ostream& lhs, SpatiaLiteDB::Polygon &rhs);

#endif
