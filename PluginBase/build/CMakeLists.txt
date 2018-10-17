cmake_minimum_required(VERSION 3.5)

if (NOT TARGET PluginBase)
project(PluginBase LANGUAGES CXX)
include (../../vqmt_plugin.cmake)

set ( sys_files )

set ( common_files_source
	   ../ICustomPlugin.h
	   ../pluginadapter.h
	   ../pluginadapter.cpp)
	
set ( common_files
	${common_files_source} )

add_library(PluginBase
	${sys_files}
	${common_files}
)

add_vqmt_plugin(PluginBase)

target_link_libraries (PluginBase)
ADD_VQMT_TARGET(PluginBase Plugins/Dependencies)
endif()