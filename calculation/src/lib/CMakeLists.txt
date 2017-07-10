include_directories (${METABOLITE_SOURCE_DIR}/glpk)
link_directories (${METABOLITE_SOURCE_DIR}/glpk)

add_library(metabolism Metabolism.cpp LPSolverFacade.cpp LPSolverAbstract.h GurobiSolverFacade.cpp GurobiSolverFacade.h)
target_link_libraries(metabolism glpk)

