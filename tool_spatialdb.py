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

from SCons.Script import Environment, Export


def build_spatialdb(env):
    "All the requirements to build the spatialdb library."
    # get options and look for SPATIALITEDIR
    options.Update(env)
    if 'SPATIALITEDIR' in env:
        env.AppendUnique(CPPPATH=[env['SPATIALITEDIR']+'/include'])
        env.AppendUnique(LIBPATH=[env['SPATIALITEDIR']+'/lib'])

    # other tools we need
    env.Require(['qtcore', 'qtgui', 'sqlitedb'])
    env.AppendLibrary('spatialite')
    env.AppendLibrary('geos')
    env.AppendLibrary('geos_c')
    env.AppendLibrary('proj')
    if (env['PLATFORM'] != 'posix'):
        env.AppendLibrary('iconv')
    if (env['PLATFORM'] == 'win32'):
        env.AppendLibrary('freexl')


env = Environment(tools=['default', 'doxygen'])

options = env.GlobalVariables()
options.AddVariables(
    PathVariable('SPATIALITEDIR', 'SpatiaLite installation root.', None))

env.Require(build_spatialdb)

sources = env.Split("""
   SpatiaLiteDB.cpp
""")

headers = env.Split("""
   SpatiaLiteDB.h
""")

toolname = 'spatialdb'

lib = env.Library(toolname, sources)
env.Default(lib)

doxref = env.Apidocs(sources + headers,
                     DOXYFILE_DICT={'PROJECT_NAME': toolname})

tooldir = env.Dir('.')


def spatialdb(env):
    "Setup to build against the spatialdb library."
    # Everything required to build it, plus the library itself and headers.
    build_spatialdb(env)
    env.AppendUnique(CPPPATH=tooldir.Dir('..'))
    env.AppendLibrary(toolname)


Export(toolname)
