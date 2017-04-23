cmake_minimum_required(VERSION 3.6)
project(metabolite)

set(CMAKE_CXX_STANDARD 14)
SET(METABOLITE_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)


message("CMAKE_C_FLAGS_DEBUG is ${CMAKE_C_FLAGS_DEBUG}")
message("CMAKE_C_FLAGS_RELEASE is ${CMAKE_C_FLAGS_RELEASE}")
message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")

set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

set(SOURCE_FILES src/main/main.cpp src/lib/Metabolism.cpp src/lib/Metabolism.hpp src/lib/LPSolverFacade.cpp
    src/lib/LPSolverFacade.h)

find_library(GMP REQUIRED)
set(CMAKE_EXE_LINKER_FLAGS "-lgmp")

add_subdirectory(${METABOLITE_SOURCE_DIR}/glpk)
add_subdirectory(${METABOLITE_SOURCE_DIR}/lib)


add_executable(metabolite ${SOURCE_FILES})
target_link_libraries(metabolite glpk)