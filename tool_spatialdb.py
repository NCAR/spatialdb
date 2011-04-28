import os
import sys

import eol_scons
tools = ['sqlitedb','doxygen','prefixoptions']
env = Environment(tools = ['default'] + tools)
platform = env['PLATFORM']
env.AppendUnique(CPPDEFINES=['SPATIALITE_AMALGAMATION',])

libsources = Split("""
SpatiaLiteDB.cpp
""")

headers = Split("""
SpatiaLiteDB.h
""")

libspatialdb = env.Library('spatialdb', libsources)

html = env.Apidocs(libsources + headers,  DOXYFILE_DICT={'PROJECT_NAME':'SpatiaLiteDB', 'PROJECT_NUMBER':'1.0'})
env.Default(html)

env.Default(libspatialdb)

thisdir = env.Dir('.').srcnode().abspath

def spatialdb(env):
    env.AppendUnique(CPPPATH   =[thisdir,])
    env.AppendLibrary('spatialdb')
    env.AppendLibrary('geos')
    env.AppendLibrary('geos_c')
    env.AppendLibrary('proj')
    env.AppendLibrary('iconv')

    env.AppendDoxref('SpatialDB')
    
    env.AppendUnique(CPPDEFINES=['SPATIALITE_AMALGAMATION',])

    env.Require(tools)

Export('spatialdb')

