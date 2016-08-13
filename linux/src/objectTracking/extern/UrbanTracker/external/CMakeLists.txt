#Minimum version of CMAKE used
cmake_minimum_required(VERSION 2.8)
set(LIBOUTPUT bin/${CMAKE_BUILD_TYPE})

file(
    GLOB_RECURSE
	source_files
	cvBlob/include/*
	cvBlob/src/*
	sqlite/include/*
	sqlite/src/*
	brisk/include/*
	brisk/src/*
)

include_directories(cvBlob/include/ sqlite/include/ brisk/include/ brisk/include/agast/)

add_library(external STATIC ${source_files})
target_link_libraries(external ${OpenCV_LIBS}) 