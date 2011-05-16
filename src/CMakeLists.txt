cmake_minimum_required (VERSION 2.6)

set (FDDL_Sources api.cpp cache.cpp mdd.cpp uniquetable.cpp operation.cpp unaryoperation.cpp binaryoperation.cpp)
add_library(FDDL ${FDDL_Sources})

install (TARGETS FDDL DESTINATION lib)
install (FILES cache.h operation.h unaryoperation.h binaryoperation.h dynarray.h mdd.h mddhandle.h uniquetable.h DESTINATION include/FDDL)
