#Minimum version of CMAKE used
cmake_minimum_required(VERSION 2.8)

#list(APPEND CMAKE_CXX_FLAGS "-std=c++0x -g -ftest-coverage -fprofile-arcs -msse -msse2 -msse3 -march=native")
list(APPEND CMAKE_CXX_FLAGS "-std=c++11 -g -ftest-coverage -fprofile-arcs -msse -msse2 -msse3 -march=native")

project(UrbanTrackerLib)

find_package(OpenCV REQUIRED)
if(${OpenCV_VERSION} VERSION_LESS 2.3.1)
  message (FATAL_ERROR "OpenCV version is not compatible: ${OpenCV_VERSION}")
endif()

#BGS Lib
find_package(PkgConfig REQUIRED)
pkg_search_module(BGSLibrary REQUIRED bgslibrary)

#External dependancy
add_subdirectory(external)
include_directories(external/cvBlob/include)
include_directories(external/sqlite/include)
include_directories(external/brisk/include)


#BOOST
find_package(Boost REQUIRED program_options filesystem system)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

file(
    GLOB_RECURSE
	source_files
	src/*
	include/*
)

include_directories(include)
include_directories(${BGSLibrary_INCLUDE_DIRS})
add_library(UrbanTrackerLib STATIC ${source_files})
target_link_libraries(UrbanTrackerLib ${OpenCV_LIBS} ${Boost_LIBRARIES} ${BGSLibrary_LIBRARIES} ${CMAKE_DL_LIBS} external)

include_directories(/home/yzbx/git/jpjodoin-urbantrackingtools-1c05587240a7/src/TrackingLib/include)


ADD_EXECUTABLE( TrackingApp /home/yzbx/git/jpjodoin-urbantrackingtools-1c05587240a7/src/TrackingApp/main.cpp )
TARGET_LINK_LIBRARIES( TrackingApp ${OpenCV_LIBS} ${BGSLibrary_LIBRARIES} ${CMAKE_DL_LIBS} external UrbanTrackerLib ${Boost_LIBRARIES} )
