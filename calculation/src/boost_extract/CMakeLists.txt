project( MiniBoost CXX)
file( GLOB_RECURSE LIBSRCS *.c *.cpp *.cc *.h *.hpp )
add_library( MiniBoost STATIC ${LIBSRCS} )
set_target_properties(MiniBoost PROPERTIES LINKER_LANGUAGE CXX)