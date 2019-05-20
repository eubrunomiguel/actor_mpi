cmake_minimum_required(VERSION 3.8)

project(ActorMPI)

file(GLOB MPI_PINGPONG_EXAMPLE_SOURCES "src/examples/mpi_pingpong/*.cpp")
file(GLOB_RECURSE ACTORLIB_SOURCES "src/actorlib/*.cpp")

option(ENABLE_MEMORY_SANITATION "Enable GCC Address sanitation. Only supported with GCC toolchain." OFF)

find_package(MPI REQUIRED)
find_package(BOOST COMPONENTS mpi serialization REQUIRED)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

set(CMAKE_REQUIRED_DEFINITIONS ${MPI_COMPILE_FLAGS})
set(CMAKE_REQUIRED_INCLUDES ${MPI_INCLUDE_PATH})
set(CMAKE_REQUIRED_LIBRARIES ${MPI_LIBRARIES})

set(CMAKE_CXX_FLAGS "${MPI_COMPILE_FLAGS}")

add_executable(mpi_pingpong_example ${MPI_PINGPONG_EXAMPLE_SOURCES})
add_library(actorlib STATIC ${ACTORLIB_SOURCES})

target_include_directories(actorlib PUBLIC ${MPI_INCLUDE_PATH})
target_include_directories(mpi_pingpong_example PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(actorlib PUBLIC ${Boost_INCLUDE_DIRS})

if (ENABLE_MEMORY_SANITATION AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    message(STATUS "Memory sanitation enabled. Linking appropriate libraries.")
    target_compile_options(actorlib PUBLIC -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -fsanitize-address-use-after-scope -Wuninitialized -Wall -Wextra -pedantic)
    target_link_libraries(actorlib PUBLIC -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined)
    # For some old Ubuntu machines, you may need to use a different linker. Set the following variable: CMAKE_EXE_LINKER_FLAGS=qq-fuse-ld=gold
elseif (ENABLE_MEMORY_SANITATION)
    message(ERROR "Memory sanitation only supported on GNU Environment")
endif ()

target_include_directories(mpi_pingpong_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(actorlib ${MPI_LIBRARIES})

target_link_libraries(mpi_pingpong_example PUBLIC actorlib)

set_property(TARGET actorlib PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET actorlib PROPERTY CXX_STANDARD 14)

set_property(TARGET mpi_pingpong_example PROPERTY CXX_STANDARD 14)

# Export compiler commands for use in YouCompleteMe
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
