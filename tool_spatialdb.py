import os
import sys

import eol_scons
tools = ['sqlitedb','doxygen']
env = Environment(tools = ['default'] + tools)

libsources = Split("""
SpatiaLiteDB.cpp
""")

headers = Split("""
SpatiaLiteDB.h
""")

env.AppendUnique(CPPPATH=['/opt/local/include',]) 
libspatialdb = env.Library('spatialdb', libsources)

html = env.Apidocs(libsources + headers,  DOXYFILE_DICT={'PROJECT_NAME':'SpatiaLiteDB', 'PROJECT_NUMBER':'1.0'})
env.Default(html)

env.Default(libspatialdb)

thisdir = env.Dir('.').srcnode().abspath

def spatialdb(env):
    env.AppendUnique(CPPPATH=['/opt/local/include',]) 
    env.AppendUnique(LIBPATH=['/opt/local/lib',])
    env.AppendUnique(LIBS=['spatialite',])
    env.AppendUnique(CPPPATH   =[thisdir,])
    env.AppendLibrary('spatialdb')
    #env.AppendDoxref('spatialdb')
    env.Require(tools)

Export('spatialdb')

