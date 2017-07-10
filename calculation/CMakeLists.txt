cmake_minimum_required(VERSION 3.1)
project(metabolite)

set(CMAKE_CXX_STANDARD 14)
SET(METABOLITE_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

include_directories(${METABOLITE_SOURCE_DIR}/boost_extract/)
#set(CMAKE_VERBOSE_MAKEFILE ON)

if (EXISTS ${METABOLITE_SOURCE_DIR}/gurobi702)
  include_directories(${METABOLITE_SOURCE_DIR}/gurobi702/linux64/include/)
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DGUROBI=1")
  message("GUROBI" ${METABOLITE_SOURCE_DIR})
  set(GUROBI 1)
else()
  message("NO GUROBI")
  set(GUROBI 0)
endif()



set(SOURCE_FILES src/lib/Metabolism.cpp src/lib/Metabolism.hpp src/lib/LPSolverFacade.cpp
    src/lib/LPSolverFacade.h src/lib/utils.hpp)

find_library(GMP REQUIRED)
set(CMAKE_EXE_LINKER_FLAGS "-lgmp")

add_subdirectory(${METABOLITE_SOURCE_DIR}/glpk)
add_subdirectory(${METABOLITE_SOURCE_DIR}/lib)
add_subdirectory(${METABOLITE_SOURCE_DIR}/boost_extract)


add_executable(metabolite src/main/main.cpp ${SOURCE_FILES})
target_link_libraries(metabolite glpk)
target_link_libraries(metabolite MiniBoost )

add_executable(coverage src/main/coverage.cpp ${SOURCE_FILES})
target_link_libraries(coverage glpk)
target_link_libraries(coverage MiniBoost )

if (GUROBI)
  target_link_libraries()
endif()


if (MPI_BUILD)
  find_package(MPI REQUIRED)

  include_directories(${MPI_INCLUDE_PATH})

  target_link_libraries(metabolite ${MPI_LIBRARIES})
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DMPI_BUILD=1")
  set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -DMPI_BUILD=1")

  if(MPI_COMPILE_FLAGS)
    set_target_properties(metabolite PROPERTIES
        COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
  endif()

  if(MPI_LINK_FLAGS)
    set_target_properties(metabolite PROPERTIES
        LINK_FLAGS "${MPI_LINK_FLAGS}")
  endif()

endif ()

message("CMAKE_C_FLAGS_DEBUG is ${CMAKE_C_FLAGS_DEBUG}")
message("CMAKE_C_FLAGS_RELEASE is ${CMAKE_C_FLAGS_RELEASE}")
message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")