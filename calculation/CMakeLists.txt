cmake_minimum_required(VERSION 3.1)
project(metabolite)

# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_CXX_STANDARD 14)
SET(METABOLITE_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1")

#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
#set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")

find_library(GMP REQUIRED)
set(CMAKE_EXE_LINKER_FLAGS "-lgmp")

include_directories(${METABOLITE_SOURCE_DIR}/boost_extract/)
add_subdirectory(${METABOLITE_SOURCE_DIR}/gurobi_interface)
add_subdirectory(${METABOLITE_SOURCE_DIR}/glpk)
add_subdirectory(${METABOLITE_SOURCE_DIR}/lib)
add_subdirectory(${METABOLITE_SOURCE_DIR}/boost_extract)

set(SOURCE_FILES src/lib/Metabolism.cpp src/lib/Metabolism.hpp   src/lib/utils.hpp)
set(SOURCE_FILES_GLPK ${SOURCE_FILES} src/lib/LPSolverFacade.cpp src/lib/LPSolverFacade.h)

find_package(GUROBI)



add_executable(metabolite_glpk src/main/main.cpp ${SOURCE_FILES_GLPK})
target_link_libraries(metabolite_glpk MiniBoost )
target_link_libraries(metabolite_glpk glpk)

add_executable(coverage_glpk src/main/coverage.cpp ${SOURCE_FILES_GLPK})
target_link_libraries(coverage_glpk MiniBoost )
target_link_libraries(coverage_glpk glpk)

if (GUROBI_FOUND)
  include_directories(${GUROBI_INCLUDE_DIRS})
  message("GUROBI " ${GUROBI_LIBRARIES} " " ${GUROBI_INCLUDE_DIRS})
  link_directories(${GUROBI_LIBRARIES})
  set(SOURCE_FILES_GUROBI ${SOURCE_FILES} src/lib/GurobiSolverFacade.cpp src/lib/GurobiSolverFacade.h)
  add_executable(metabolite_gurobi src/main/main.cpp ${SOURCE_FILES_GUROBI})
  target_link_libraries(metabolite_gurobi MiniBoost )
  target_link_libraries(metabolite_gurobi gurobi -lgurobi75 -lm)
  target_compile_options(metabolite_gurobi PRIVATE "-DGUROBI=1")
  add_executable(coverage_gurobi src/main/coverage.cpp ${SOURCE_FILES_GUROBI})
  target_link_libraries(coverage_gurobi MiniBoost )
  target_link_libraries(coverage_gurobi gurobi -lgurobi75 -lm)
endif()

if (MPI_BUILD)
  find_package(MPI REQUIRED)

  include_directories(${MPI_INCLUDE_PATH})

  target_link_libraries(metabolite_glpk ${MPI_LIBRARIES})
  target_link_libraries(metabolite_gurobi ${MPI_LIBRARIES})
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