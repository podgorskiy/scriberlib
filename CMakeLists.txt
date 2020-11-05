cmake_minimum_required(VERSION 3.6)

project(scriber)

if (CMAKE_VERSION VERSION_LESS "3.1")
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    	set (CMAKE_CXX_FLAGS "--std=c++14 ${CMAKE_CXX_FLAGS}")
	endif ()
else ()
	set (CMAKE_CXX_STANDARD 14)
endif ()

set(SC_DIR sources/)
set(SC_LIBS_DIR 3rdparty/)
set(SC_INCLUDE include/)
include_directories(${SC_INCLUDE})
include_directories(${SC_LIBS_DIR}/xxhash)
include_directories(${SC_LIBS_DIR}/utf8)
include_directories(${SC_LIBS_DIR}/harfbuzz)
include_directories(${SC_LIBS_DIR}/harfbuzz/hb-ucdn)
include_directories(${SC_LIBS_DIR}/minibidi)
include_directories(${SC_LIBS_DIR}/freetype)
file(GLOB SC_SOURCES ${SC_DIR}/*.cpp ${SC_INCLUDE}/*.h)
file(GLOB SC_3DPARTY_HARFBUZZ ${SC_LIBS_DIR}/harfbuzz/*.cpp ${SC_LIBS_DIR}/harfbuzz/*.cc ${SC_LIBS_DIR}/harfbuzz/*.h ${SC_LIBS_DIR}/harfbuzz/*.hh)
file(GLOB SC_3DPARTY_UCDN ${SC_LIBS_DIR}/harfbuzz/hb-ucdn/*.cpp  ${SC_LIBS_DIR}/harfbuzz/hb-ucdn/*.h)
file(GLOB SC_3DPARTY_FREETYPE ${SC_LIBS_DIR}/freetype/*.c ${SC_LIBS_DIR}/freetype/*.h)

add_library(scribe ${SC_SOURCES} ${SC_3DPARTY_HARFBUZZ} ${SC_3DPARTY_UCDN} ${SC_3DPARTY_FREETYPE})

source_group("harfbuzz" FILES ${SC_3DPARTY_HARFBUZZ})
source_group("ucdn" FILES ${SC_3DPARTY_UCDN})
source_group("freetype" FILES ${SC_3DPARTY_FREETYPE})
source_group("scribe" FILES ${SC_SOURCES})

target_compile_options(scribe  PRIVATE -Wno-nonnull-compare)
target_compile_definitions(scribe PRIVATE HAVE_OT HB_NO_MT HAVE_FREETYPE HAVE_UCDN)

if(MSVC)
    target_compile_definitions(scribe PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
	target_compile_options(scribe PRIVATE /wd4267 /wd4244 /wd4800 )
elseif(APPLE)
	target_compile_options(scribe  PRIVATE -funsafe-math-optimizations -fno-strict-aliasing -fno-rtti -ffast-math -msse2 -msse3 -msse4 -fopenmp)
	target_link_libraries(scribe PRIVATE gomp)
else()
	target_compile_options(scribe PRIVATE -funsafe-math-optimizations -fno-strict-aliasing -fno-rtti -ffast-math -msse2 -msse3 -msse4 -fopenmp)
	target_link_libraries(scribe PRIVATE gomp)
endif()

target_compile_definitions(scribe PRIVATE FONT_SDF)

find_package(OpenGL)
if (OPENGL_FOUND)
	message("OpenGL Correctly Found")
	include_directories(${OPENGL_INCLUDE_DIR})
else (OPENGL_FOUND)
	message("OpenGL environment missing")
endif (OPENGL_FOUND)

file(GLOB_RECURSE SOURCES example/*.cpp example/*.h)

source_group("" FILES ${SOURCES})

add_executable(example ${SOURCES})

set(LIBRARIES scribe ${OPENGL_LIBRARIES} )

target_link_libraries(example ${LIBRARIES})
