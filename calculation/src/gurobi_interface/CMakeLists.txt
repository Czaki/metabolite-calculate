find_package(GUROBI)

message("Gurobi " ${GUROBI_FOUND})

if (GUROBI_FOUND)
  include_directories(${GUROBI_INCLUDE_DIRS})
  set(GPATH ${METABOLITE_SOURCE_DIR}/gurobi_interface/src/cpp)
  set(GUROBI_INTERFACE_FILES ${GPATH}/attrprivate.h ${GPATH}/Callback.cpp ${GPATH}/Callback.h ${GPATH}/Column.cpp ${GPATH}/Column.h ${GPATH}/Common.h ${GPATH}/Constr.cpp ${GPATH}/Constr.h ${GPATH}/Env.cpp ${GPATH}/Env.h ${GPATH}/EnvISV.h ${GPATH}/Exception.cpp ${GPATH}/Exception.h ${GPATH}/Expr.h ${GPATH}/GenConstr.cpp ${GPATH}/GenConstr.h ${GPATH}/LinExpr.cpp ${GPATH}/LinExpr.h ${GPATH}/Model.cpp ${GPATH}/Model.h ${GPATH}/parprivate.h ${GPATH}/public.h ${GPATH}/QConstr.cpp ${GPATH}/QConstr.h ${GPATH}/QuadExpr.cpp ${GPATH}/QuadExpr.h ${GPATH}/SOS.cpp ${GPATH}/SOS.h ${GPATH}/TempConstr.cpp ${GPATH}/TempConstr.h ${GPATH}/Var.cpp ${GPATH}/Var.h)
  add_library(gurobi ${GUROBI_INTERFACE_FILES})
endif()