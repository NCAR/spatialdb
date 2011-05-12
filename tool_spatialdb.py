import os
import sys

import eol_scons
tools = ['sqlitedb','doxygen','prefixoptions']
env = Environment(tools = ['default'] + tools)
platform = env['PLATFORM']
thisdir = env.Dir('.').srcnode().abspath

libsources = Split("""
SpatiaLiteDB.cpp
""")

headers = Split("""
SpatiaLiteDB.h
""")

env.AppendUnique(CPPDEFINES=['SPATIALITE_AMALGAMATION',])
libspatialdb = env.Library('spatialdb', libsources)
env.Default(libspatialdb)

html = env.Apidocs(libsources + headers,  DOXYFILE_DICT={'PROJECT_NAME':'SpatiaLiteDB', 'PROJECT_NUMBER':'1.0'})
env.Default(html)

def spatialdb(env):
    env.AppendUnique(CPPPATH   =[thisdir,])
    env.AppendLibrary('spatialdb')
    env.AppendLibrary('geos')
    env.AppendLibrary('geos_c')
    env.AppendLibrary('proj')
    if (platform != 'posix'):
        env.AppendLibrary('iconv')
    env.AppendDoxref('SpatialDB')  
    env.AppendUnique(CPPDEFINES=['SPATIALITE_AMALGAMATION',])
    env.Replace(CCFLAGS=['-g','-O2'])
    env.Require(tools)

Export('spatialdb')

