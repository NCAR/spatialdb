#ifndef SPATIALITEDB_INC
#define SPATIALITEDB_INC

#include <ostream>
#include <string>
#include <vector>
#include "SQLiteDB.h"
#include <spatialite/gaiageo.h>

/// A wrapper for the SpatiaLite Database. Some of the SpatiaLite (i.e. PostGIS)
/// abstractions are represented as internal classes. The database coordinates
/// are not constrainted to a particular coordinate system.
///
/// The principal user members are queryGeometry(), which finds all artifacts
/// within a specified bounding box, and points(), linetstrings() and
/// polygons(), which return the results of the last queryGeometry() call.
///
/// SpatiaLiteDB only performs queries against an existing SpaitiaLite database;
/// it is not capable of modifying the database. See MicroMapOveriview.h
/// for a discussion of populating a database.
///
/// The user needs to have an awareness of the feature names that
/// the database contains. Feature names are strings such as "coastline",
/// "rivers_lake_centerlines", "populated_places", and so on.
class SpatiaLiteDB: public SQLiteDB {

public:
	/// Represent a geographical point.
	class Point {
	public:
		/// Constructor
		/// @param x X (longitude) location).
		/// @param y Y (latitude) location.
		/// @param label An optional label for the point.
		Point(double x, double y, std::string label = "");
		/// Destructor
		virtual ~Point();
		/// Send a basic display of the characteristics of Point to a stream.
		/// The x and y coordinates are displayed. The label is not displayed.
		/// @param lhs The output stream.
		/// @param rhs The Point to be displayed.
		/// @returns The output stream.
		friend std::ostream& operator<<(std::ostream &lhs, Point& rhs);
		/// X coordinate.
		double _x;
		/// Y coordinate.
		double _y;
		/// Point label.
		std::string _label;
	};

	/// Represent a non-closed path between a collection of Point.
	class Linestring: public std::vector<Point>  {
	public:
		/// Constructor
		/// @param label Optional label.
		Linestring(std::string label = "");
		/// Destructor
		virtual ~Linestring();
		/// The label.
		std::string _label;
		/// Send a basic display of the characteristics of Linestring to a stream. The
		/// collection of Point is displayed.
		/// @param lhs The output stream.
		/// @param rhs The Linestring to be displayed.
		/// @returns The output stream.
		friend std::ostream& operator<<(std::ostream &lhs, Linestring& rhs);
	};

	/// A simple closed linestring which does not cross itself.
	class Ring: public std::vector<Point>  {
	public:
		/// Constructor
		Ring();
		/// Constructor
		/// @param clockwise True if the path is clockwise.
		Ring(int clockwise);
		/// Destructor
		virtual ~Ring();
		/// Send a basic display of the characteristics of Ring to a stream.
		/// Each Point is displayed.
		/// @param lhs The output stream.
		/// @param rhs The Ring to be displayed.
		/// @returns The output stream.
		friend std::ostream& operator<<(std::ostream &lhs, Ring& rhs);
		/// Set true if the graph direction is clockwise.
		int _clockwise;
	};

	/// A polygon is the representation of an area. It has a Ring containing zero
	/// or more internal Ring.
	class Polygon {
	public:
		/// Constructor
		/// @param label Optional label.
		Polygon(std::string label = "");
		/// Destructor
		virtual ~Polygon();
		/// Specify the external ring.
		/// @todo Is it legal for Polygon to not have an external ring?
		/// Why isn't the external ring required in the constructor?
		/// @param r The external ring.
		void setExtRing(Ring r);
		/// Add an internal ring.
		/// @param r The internal ring to be added.
		void addIntRing(Ring r);
		/// The label.
		std::string _label;
		/// Fetch the external ring.
		/// @returns The external ring.
		Ring extRing();
		/// Fetch the internal rings.
		/// @returns The internal rings.
		std::vector<Ring> intRings();
		/// Send a basic display of the characteristics of Polygon to a stream. The
		/// external ring is displayed first, followed by any internal rings.
		/// @param lhs The output stream.
		/// @param rhs The Polygon to be displayed.
		/// @returns The output stream.
		friend std::ostream& operator<<(std::ostream &lhs, Polygon& rhs);
	protected:
		/// The external ring.
		Ring _extRing;
		/// The internal rings.
		std::vector<Ring> _intRings;
	};

	/// A simple list of Point.
	class PointList: public std::vector<Point> {
	public:
		/// Constructor
		PointList();
		/// Destructor
		virtual ~PointList();
	};

	/// A simple list of Linestring.
	class LinestringList: public std::vector<Linestring> {
	public:
		/// Constructor
		LinestringList();
		/// Destructor
		virtual ~LinestringList();
	};

	/// A simple list of Polygon.
	class PolygonList: public std::vector<Polygon> {
	public:
		/// Constructor
		/// @param label An optional label.
		PolygonList(std::string label = "");
		/// Destructor
		virtual ~PolygonList();
	};

public:
	/// Constructor
	/// @param dbPath Path to an existing valid SpatiaLite
	/// database.
	SpatiaLiteDB(std::string dbPath) throw (std::string);
	/// Destructor
	virtual ~SpatiaLiteDB();
	/// @return Return a string with version information
	static std::string version();
	/// Find out what geometry features are available. All of the tables
	/// in the database are checked to see if they have a "geometry" column.
	/// If so, the table name is returned.
	/// @returns The geometry table names.
	std::vector<std::string> geometryTables();
	/// Search the database for all Points, LinestringLists,
	/// and PolygonLists  within the specified table and bounding box. These
	/// geographical elements are stored in _linestringlist, _pointlist and
	/// _polygonlist (which are cleared on entry to the routine). A column containing a
	/// possible label for the element can be specified.
	/// @param table The table (i.e. feature) to be searched.
	/// @param geometry_col The name of the geometry column in the table. Typically it is "Geometry".
	/// @param left Minimum X limit.
	/// @param bottom Minimum Y limit.
	/// @param right Maximum X limit.
	/// @param top Maximum Y limit.
	/// @param label_col The name of the column which (might) contain a label for the items.
	void queryGeometry(
			std::string table,
			std::string geometry_col,
			double left,
			double bottom,
			double right,
			double top,
			std::string label_col = "");

	/// @return The points found in the last queryGeometry() call.
	PointList points();
	/// @return The Linestrings found in the last queryGeometry() call.
	LinestringList linestrings();
	/// @return The Polygons found in the last queryGeometry() call.
	PolygonList polygons();

protected:
	/// Get the geometry collection for the current row. Note that
	/// SQLiteDB::step() is used to advance to the next row.
	/// @param col The column to be extracted.
	/// @return A pointer to geometry collection from the selected column of the current row.
	gaiaGeomCollPtr Geom(int col) throw (std::string);
	/// Find all points for the specified geometry.
	/// @param geom The geometry (from one row).
	/// @param label A label to be included with the point.
	/// @return The collection of points.
	PointList point_list(gaiaGeomCollPtr geom, std::string label);
	/// Find all Linestrings for the specified geometry.
	/// @param geom The geometry (from one row).
	/// @param label A label to be included with the Linestrings.
	/// @return The collection of Linestrings.
	LinestringList linestring_list(gaiaGeomCollPtr geom, std::string label);
	/// Find all Polygons for the specified geometry.
	/// @param geom The geometry (from one row).
	/// @param label A label to be included with the Polygons.
	/// @return The collection of Polygons.
	PolygonList polygon_list(gaiaGeomCollPtr, std::string label);
	/// The points extracted by the last queryGeometry()
	PointList      _pointlist;
	/// The Linestrings extracted by the last queryGeometry()
	LinestringList _linestringlist;
	/// The Polygons extracted by the last queryGeometry()
	PolygonList    _polygonlist;
	/// The SQLite database connection.
	void* _connection;
	/// Save the geom for freeing
	std::vector<gaiaGeomCollPtr> _geoms;
};

std::ostream& operator<<(std::ostream& lhs, SpatiaLiteDB::Point &rhs);
std::ostream& operator<<(std::ostream& lhs, SpatiaLiteDB::Linestring &rhs);
std::ostream& operator<<(std::ostream& lhs, SpatiaLiteDB::Ring &rhs);
std::ostream& operator<<(std::ostream& lhs, SpatiaLiteDB::Polygon &rhs);

#endif
