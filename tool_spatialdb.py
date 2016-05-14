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

toolname = 'spatialdb'

thisDir = Dir('.').abspath
upDir   = Dir('./../').abspath

options = None

# Define tool
def spatialdb(env):

    # get options and look for SPATIALITEDIR
    global options
    if not options:
        options = env.GlobalVariables()
        options.AddVariables(PathVariable('SPATIALITEDIR',
                                          'SpatiaLite installation root.', None))
    options.Update(env)

    # other tools we need
    env.Require(['qt4', 'prefixoptions', 'sqlitedb'])
    env.EnableQt4Modules(['QtCore','QtGui'])	

    env.AppendUnique(CPPPATH = upDir)
    env.AppendUnique(CPPPATH = thisDir)
    if env.has_key('SPATIALITEDIR'):
        env.AppendUnique(CPPPATH=[env['SPATIALITEDIR']+'/include',])
        env.AppendUnique(LIBPATH=[env['SPATIALITEDIR']+'/lib',])
    
    # Add libraries
    env.AppendLibrary(toolname)
    env.AppendLibrary('spatialite')
    env.AppendLibrary('geos')
    env.AppendLibrary('geos_c')
    env.AppendLibrary('proj')
    if (env['PLATFORM'] != 'posix'):
        env.AppendLibrary('iconv')
    if (env['PLATFORM'] == 'win32'):
        env.AppendLibrary('freexl')
    
Export(toolname)

# Build the library
env = Environment(tools = ['default', 'doxygen', toolname])

sources = Split("""
   SpatiaLiteDB.cpp
""")

headers = Split("""
   SpatiaLiteDB.h
""")

lib = env.Library(toolname, sources)
env.Default(lib)

# Create doxygen
doxref = env.Apidocs(sources + headers, DOXYFILE_DICT={'PROJECT_NAME':toolname, 'PROJECT_NUMBER':'1.0'})

