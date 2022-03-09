cmake_minimum_required(VERSION 3.0)

project(libconf
	VERSION 0.3
	DESCRIPTION "libconf - small library to process config files"
	LANGUAGES C)

if(NOT UNIX AND NOT APPLE)
	message(FATAL_ERROR "Your system is not unix. Fail to build.")
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_C_FLAGS_DEBUG "-std=c11 -Wall -Wextra -Werror -Wpedantic -g -O0")
	set(CMAKE_C_FLAGS_RELEASE "-std=c11 -O1")
endif()

set(LIB_OUTPUT conf)
set(SOURCE src/libconf.c)
set(HEADERS include/libconf.h)

add_library(${LIB_OUTPUT} STATIC ${SOURCE})

install(TARGETS ${LIB_OUTPUT} DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)

