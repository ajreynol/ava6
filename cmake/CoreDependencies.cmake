# Optional libraries and bindings outside the core solver are unavailable.
foreach(feature USE_POLY USE_COCOA USE_MPFR USE_NORMALIZ USE_CLN
                USE_CRYPTOMINISAT USE_KISSAT USE_GLPK
                BUILD_BINDINGS_PYTHON BUILD_BINDINGS_JAVA BUILD_DOCS
                ONLY_PYTHON_EXT_SRC)
  if(DEFINED ${feature} AND ${feature})
    message(FATAL_ERROR "${feature} is not supported in the core solver")
  endif()
  set(${feature} OFF CACHE BOOL "Unavailable in the core solver" FORCE)
endforeach()
