# spatialdb
A wrapper for a [SpatialLite](http://www.gaia-gis.it/gaia-sins/) specific database.
It is built upon [sqlitedb](https://github.com/ncareol/sqlitedb), and provides support
for the embeddable [qmicromap](https://github.com/ncareol/qmicromap) geomapping component.

Some of the SpatiaLite (i.e. PostGIS) abstractions are represented as internal classes. 
The database coordinates are not constrainted to a particular coordinate system.

The principal user members are queryGeometry(), which finds all artifacts
within a specified bounding box, and points(), linetstrings() and
polygons(), which return the results of the last queryGeometry() call.

SpatiaLiteDB only performs queries against an existing SpaitiaLite database;
it is not capable of modifying the database. See MicroMapOveriview.h
for a discussion of populating a database.

The user needs to have an awareness of the feature names that
the database contains. Feature names are strings such as *coastline*,
*rivers_lake_centerlines*, *populated_places*, and so on.

## Dependencies
 * [sqlitedb](https://github.com/ncareol/sqlitedb)
 
## Used By
 * [qmicromap](https://github.com/ncareol/qmicromap)
 * [aspen](https://github.com/ncareol/aspen)
