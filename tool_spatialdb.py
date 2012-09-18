# SpatialDB scons tool
#
# It builds the library for the SpatialDB C++ class library,
# and provides CPP and linker specifications for the header 
# and libraries.
#
# SpatialDB depends on SqliteDB, which provides the interface to 
# sqlite3. It also depends on SpatiaLite. Since SpatiaLite is 
# is also needed by SQLiteDB, we let the sqlitedb tool provide
# the required SpatiaLite plumbing.

import os
import sys
import eol_scons

tools = ['sqlitedb','doxygen','prefixoptions']
env = Environment(tools = ['default'] + tools)
platform = env['PLATFORM']
thisdir = env.Dir('.').srcnode().abspath

# define the tool
def spatialdb(env):
    env.AppendUnique(CPPPATH   =[thisdir,])
    env.AppendLibrary('spatialdb')
    env.AppendLibrary('geos')
    env.AppendLibrary('geos_c')
    env.AppendLibrary('proj')
    if (platform != 'posix'):
        env.AppendLibrary('iconv')
    if (platform == 'win32'):
        env.AppendLibrary('freexl')
    env.Require(tools)

Export('spatialdb')

# build the SpatialDB library
libsources = Split("""
SpatiaLiteDB.cpp
""")

headers = Split("""
SpatiaLiteDB.h
""")

libspatialdb = env.Library('spatialdb', libsources)
env.Default(libspatialdb)

html = env.Apidocs(libsources + headers,  DOXYFILE_DICT={'PROJECT_NAME':'SpatialDB', 'PROJECT_NUMBER':'1.0'})


