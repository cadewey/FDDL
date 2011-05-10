cmake_minimum_required (VERSION 2.6)

set (FDDL_Sources api.cc caches.cc mdd.cc uniquetable.cc)
add_library(FDDL ${FDDL_Sources})

install (TARGETS FDDL DESTINATION lib)
install (FILES caches.h dynarray.h mdd.h mddhandle.h uniquetable.h DESTINATION include/FDDL)
