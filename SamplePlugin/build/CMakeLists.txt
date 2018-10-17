cmake_minimum_required(VERSION 3.5)

project(PluginSample LANGUAGES CXX)

set ( plugin_files
	../vqmt_sample_plugin.h
	../vqmt_sample_plugin.cpp
)

set ( support_files
	../../PluginBase/json.h
	../../PluginBase/PluginAdapter.h
	../../PluginBase/ICustomPlugin.h
)

add_library(PluginSample SHARED
	${plugin_files}
	${support_files}
	../../README.md
)

if(VQMT_FULL_BUILD)
	include_directories(../../../include)
else()
	include_directories(../../include)
endif(VQMT_FULL_BUILD)

source_group("Plugin files" FILES ${plugin_files})
source_group("Support files" FILES ${support_files})

if(MSVC)
	set_target_properties(PluginSample
		PROPERTIES PREFIX ""
				   SUFFIX ".vmp"
		)
endif(MSVC)

if(MSVC)
	set(linkLibs)
else()
	set(linkLibs -lpthread -lstdc++fs )
endif()

target_link_libraries (PluginSample ${linkLibs})

