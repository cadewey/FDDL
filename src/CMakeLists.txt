cmake_minimum_required (VERSION 2.6)

set (FDDL_Sources api.cpp caches.cpp mdd.cpp uniquetable.cpp operation.cpp)
add_library(FDDL ${FDDL_Sources})

install (TARGETS FDDL DESTINATION lib)
install (FILES caches.h operation.h dynarray.h mdd.h mddhandle.h uniquetable.h DESTINATION include/FDDL)
