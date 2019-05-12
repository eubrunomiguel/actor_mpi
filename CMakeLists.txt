cmake_minimum_required(VERSION 3.8)

project(ActorUpcxx)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake-Aux/Modules/")

file(GLOB MPI_PINGPONG_EXAMPLE_SOURCES "src/examples/mpi_pingpong/*.cpp")
file(GLOB_RECURSE ACTORLIB_SOURCES "src/actorlib/*.cpp")

option(BUILD_USING_UPCXX_WRAPPER "Instead of querying for meta information, use the upcxx compiler wrapper directly." OFF)
option(ENABLE_O3_UPCXX_BACKEND "Builds with optimized UPC++ Library." ON)
option(ENABLE_PARALLEL_UPCXX_BACKEND "Links to parallel UPC++ Library backend." OFF)
option(ENABLE_MEMORY_SANITATION "Enable GCC Address sanitation. Only supported with GCC toolchain." OFF)

find_package(MPI REQUIRED)
set(CMAKE_REQUIRED_DEFINITIONS 	${MPI_COMPILE_FLAGS})
set(CMAKE_REQUIRED_INCLUDES 	${MPI_INCLUDE_PATH})
set(CMAKE_REQUIRED_LIBRARIES 	${MPI_LIBRARIES})

if (ENABLE_PARALLEL_UPCXX_BACKEND)
    SET(ENV{UPCXX_THREADMODE} par)
else()
    SET(ENV{UPCXX_THREADMODE} seq)
endif()

if (ENABLE_O3_UPCXX_BACKEND)
    SET(ENV{UPCXX_CODEMODE} O3)
else()
    SET(ENV{UPCXX_CODEMODE} debug)
endif()

if (NOT BUILD_USING_UPCXX_WRAPPER)
    execute_process(COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/CMake-Aux/get_libs.sh libs RESULT_VARIABLE RETVAL OUTPUT_VARIABLE UPCXX_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/CMake-Aux/get_libs.sh libpaths OUTPUT_VARIABLE UPCXX_LIBPATHS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/CMake-Aux/get_libs.sh defines OUTPUT_VARIABLE UPCXX_DEFINES OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/CMake-Aux/get_libs.sh includes OUTPUT_VARIABLE UPCXX_INCLUDES OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND upcxx-meta LDFLAGS OUTPUT_VARIABLE UPCXX_CXXFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if (BUILD_USING_UPCXX_WRAPPER)
    message(STATUS "UPC++ meta info")
    message(STATUS "Using UPC++ compiler wrapper.")
else()
    message(STATUS "UPC++ meta info")
    message(STATUS "LIBS:\t${UPCXX_LIBS}")
    message(STATUS "LIBPATHS:\t${UPCXX_LIBPATHS}")
    message(STATUS "DEFINES:\t${UPCXX_DEFINES}")
    message(STATUS "INCLUDES:\t${UPCXX_INCLUDES}")
    message(STATUS "CXXFLAGS:\t${UPCXX_CXXFLAGS}")
    
    foreach(LIBNAME ${UPCXX_LIBS})
        find_library(UPCXX_LIB_${LIBNAME} NAMES ${LIBNAME} HINTS ${UPCXX_LIBPATHS} NO_SYSTEM_ENVIRONMENT_PATH)
        message(STATUS " - ${LIBNAME}:\t${UPCXX_LIB_${LIBNAME}}")
    endforeach()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${UPCXX_CXXFLAGS} ${MPI_COMPILE_FLAGS}")

add_executable(mpi_pingpong_example ${MPI_PINGPONG_EXAMPLE_SOURCES})
add_library(actorlib STATIC ${ACTORLIB_SOURCES})

if(NOT BUILD_USING_UPCXX_WRAPPER)
    target_include_directories(actorlib PUBLIC ${UPCXX_INCLUDES})
    target_include_directories(actorlib PUBLIC ${MPI_INCLUDE_PATH})
    target_compile_definitions(actorlib PUBLIC ${UPCXX_DEFINES})
endif()

target_compile_definitions(actorlib PRIVATE ACTORLIB_UPCXX_CODEMODE=$ENV{UPCXX_CODEMODE})
target_compile_definitions(actorlib PRIVATE ACTORLIB_UPCXX_GASNET_CONDUIT=$ENV{UPCXX_GASNET_CONDUIT})
target_compile_definitions(actorlib PRIVATE ACTORLIB_UPCXX_GASNET_CODEMODE=$ENV{UPCXX_CODEMODE})
target_compile_definitions(actorlib PRIVATE ACTORLIB_UPCXX_INSTALLATION=$ENV{UPCXX_INSTALL})

if (ENABLE_MEMORY_SANITATION AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    message(STATUS "Memory sanitation enabled. Linking appropriate libraries.")
    target_compile_options(actorlib PUBLIC -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -fsanitize-address-use-after-scope -Wuninitialized -Wall -Wextra -pedantic)
    target_link_libraries(actorlib PUBLIC -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined)
    # For some old Ubuntu machines, you may need to use a different linker. Set the following variable: CMAKE_EXE_LINKER_FLAGS=qq-fuse-ld=gold
elseif(ENABLE_MEMORY_SANITATION)
    message(ERROR "Memory sanitation only supported on GNU Environment")
endif()

target_include_directories(mpi_pingpong_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (NOT BUILD_USING_UPCXX_WRAPPER)
    foreach(LIBNAME ${UPCXX_LIBS})
        target_link_libraries(actorlib ${UPCXX_LIB_${LIBNAME}})
    endforeach()
    target_link_libraries(actorlib ${MPI_LIBRARIES})
    target_link_libraries(actorlib pthread)
else()
    add_custom_command(TARGET actorlib PRE_BUILD COMMAND export UPCXX_CODEMODE=debug COMMAND export UPCXX_THREADMODE=par)
endif()

target_link_libraries(mpi_pingpong_example PUBLIC actorlib)

set_property(TARGET actorlib PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET actorlib PROPERTY CXX_STANDARD 14)

set_property(TARGET mpi_pingpong_example PROPERTY CXX_STANDARD 14)

# Export compiler commands for use in YouCompleteMe
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
