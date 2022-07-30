cmake_minimum_required(VERSION 3.0)

project(libconf
	VERSION 0.4
	DESCRIPTION "libconf - small library to process config files"
	LANGUAGES C)

if(NOT UNIX)
	message(FATAL_ERROR "Your system is not unix. Fail to build.")
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_C_FLAGS_RELEASE "-std=c99 -O2 -DNDEBUG")

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_C_FLAGS_DEBUG "-std=c99 -Wall -Wextra -Werror -g -O0 -Wpedantic -Wshadow -Wvla -Wpointer-arith -Wwrite-strings -Wfloat-equal -Wcast-align -Wcast-qual -Wbad-function-cast -Wstrict-overflow=4 -Wunreachable-code -Wformat=2 -Wundef -Wstrict-prototypes -Wold-style-definition -Wconversion -Wshift-overflow=2 -Wredundant-decls -Wnested-externs -Wmissing-include-dirs -Wlogical-op -Wcast-align=strict -Wduplicated-cond -Wjump-misses-init -Wstringop-overflow")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
	set(CMAKE_C_FLAGS_DEBUG "-std=c99 -Wall -Wextra -Werror -g -O0 -Wpedantic -Wshadow -Wvla -Wpointer-arith -Wwrite-strings -Wfloat-equal -Wcast-align -Wcast-qual -Wbad-function-cast -Wstrict-overflow=4 -Wunreachable-code -Wformat=2 -Wundef -Wstrict-prototypes -Wold-style-definition -Wconversion -Wshift-overflow -Wredundant-decls -Wnested-externs -Wmissing-include-dirs")
else()
	message(FALAT_ERROR "Clang or GCC is not installed on your system. Fail to build.")
endif()

set(LIB conf)
set(SOURCE src/libconf.c)
set(HEADER include/libconf.h)

add_library(${LIB} STATIC ${SOURCE} ${HEADER})
add_library(lib::conf ALIAS ${LIB})

target_include_directories(${LIB} PRIVATE "${CMAKE_SOURCE_DIR}/include/")

install(TARGETS ${LIB} DESTINATION lib)
install(FILES ${HEADER} DESTINATION include)
