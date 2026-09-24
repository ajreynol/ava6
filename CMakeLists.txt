###############################################################################
# This file is part of the cvc5 project.
#
# Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
# in the top-level source directory and their institutional affiliations.
# All rights reserved.  See the file COPYING in the top-level source
# directory for licensing information.
# #############################################################################
#
# The build system configuration.
##

cmake_minimum_required(VERSION 3.16)

#-----------------------------------------------------------------------------#
# Project configuration

project(ava6)

include(CheckIPOSupported)
include(GNUInstallDirs)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(version)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
if (WASM STREQUAL "OFF")
  set(CMAKE_C_EXTENSIONS OFF)
else()
  set(CMAKE_C_EXTENSIONS ON)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Generate compile_commands.json, which can be used for various code completion
# plugins.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default component used in install() commands
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME ava6)

#-----------------------------------------------------------------------------#
# Policies

# Required for FindGLPK since it sets CMAKE_REQUIRED_LIBRARIES
if(POLICY CMP0075)
  cmake_policy(SET CMP0075 NEW)
endif()

if(POLICY CMP0135)
  # For `ExternalProject_Add()`, set the timestamp of the files extracted from
  # the downloaded archive to the time of the extraction. This behavior was
  # introduced in CMake 3.23 and is the recommended to make sure that
  # dependencies get rebuilt when the URL changes.
  cmake_policy(SET CMP0135 NEW)
endif()

if(POLICY CMP0079)
  # Allows target_link_libraries() to use targets from other directories.
  # This will prevent issues with INTERFACE_LINK_LIBRARIES, which allows
  # ava6 to be properly linked as a static library on other CMake projects.
  cmake_policy(SET CMP0079 NEW)
endif()

#-----------------------------------------------------------------------------#
# Tell CMake where to find our dependencies

if(GLPK_DIR)
  list(APPEND CMAKE_PREFIX_PATH "${GLPK_DIR}")
endif()

# By default the contrib/get-* scripts install dependencies to deps/install.
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/deps/install")

#-----------------------------------------------------------------------------#

include(Helpers)
include(CoreDependencies)

#-----------------------------------------------------------------------------#
# User options

# License
option(ENABLE_GPL "Enable GPL dependencies")

# General build options
#
# >> 3-valued: IGNORE ON OFF
#    > allows to detect if set by user (default: IGNORE)
#    > only necessary for options set for build types
option(BUILD_SHARED_LIBS          "Build shared libraries and binary")
ava6_option(ENABLE_ASAN           "Enable ASAN build")
ava6_option(ENABLE_UBSAN          "Enable UBSan build")
ava6_option(ENABLE_TSAN           "Enable TSan build")
ava6_option(ENABLE_ASSERTIONS     "Enable assertions")
ava6_option(ENABLE_DEBUG_SYMBOLS  "Enable debug symbols")
ava6_option(ENABLE_MUZZLE         "Suppress ALL non-result output")
ava6_option(ENABLE_STATISTICS     "Enable statistics")
ava6_option(ENABLE_TRACING        "Enable tracing")
ava6_option(ENABLE_UNIT_TESTING   "Enable unit testing")
ava6_option(ENABLE_VALGRIND       "Enable valgrind instrumentation")
ava6_option(ENABLE_AUTO_DOWNLOAD  "Enable automatic download of dependencies")
ava6_option(USE_PYTHON_VENV       "Install Python dependencies in virtual environment")
ava6_option(ENABLE_IPO            "Enable interprocedural optimization")

ava6_set_option(USE_PYTHON_VENV OFF)

# >> 2-valued: ON OFF
#    > for options where we don't need to detect if set by user (default: OFF)
option(ENABLE_CLANG_TIDY       "Enable clang-tidy static analysis during build")
option(ENABLE_COVERAGE         "Enable support for gcov coverage testing")
option(ENABLE_DEBUG_CONTEXT_MM "Enable the debug context memory manager")
option(ENABLE_PROFILING        "Enable support for gprof profiling")

option(TREAT_WARNING_AS_ERROR  "Treat warnings on building as errors")

# Ensure ava6 does not use LibPoly's global context by stripping all related code

# Optional dependencies
#
# >> 2-valued: ON OFF
#    > for options where we don't need to detect if set by user (default: OFF)
option(USE_EDITLINE      "Use Editline for better interactive support")

# Custom install directories for dependencies
# If no directory is provided by the user, we first check if the dependency was
# installed via the corresponding contrib/get-* script and if not found, we
# check the intalled system version. If the user provides a directory we
# immediately fail if the dependency was not found at the specified location.
set(GLPK_DIR          "" CACHE STRING "Set GLPK install directory")

# Prepend binaries with prefix on make install
set(PROGRAM_PREFIX    "" CACHE STRING "Program prefix on make install")

# Supported language bindings based on new C++ API

# Api documentation

option(BUILD_GMP "Build GMP from source")
option(BUILD_CLN "Build CLN from source")

# Link against static system libraries
ava6_option(STATIC_BINARY "Link against static system libraries \
(enabled by default for static builds)")

option(SKIP_COMPRESS_DEBUG "Skip compression of debug symbols")
option(SKIP_SET_RPATH      "Skip setting of rpath")
option(USE_DEFAULT_LINKER  "Use default linker")

# Custom flags for WebAssembly compilation
set(WASM_FLAGS "" CACHE STRING "Link flags for the WebAssembly binary generation")
set(WASM "OFF" CACHE STRING "Use a specific extension for WebAssembly compilation")

# Make sure no other available value will be assigned to this variable
if(NOT(WASM STREQUAL "WASM" OR WASM STREQUAL "JS" OR WASM STREQUAL "HTML" OR WASM STREQUAL "OFF"))
  message(FATAL_ERROR "${WASM} is not a valid wasm flag value.")
endif()

if(WASM STREQUAL "OFF")
  set(WASM_FLAGS "")
  set(CONFIGURE_CMD_WRAPPER "")
# If it's a wasm compilation, but isn't using the right flags
elseif(BUILD_SHARED_LIBS OR NOT(STATIC_BINARY) OR NOT(ENABLE_AUTO_DOWNLOAD))
  message(FATAL_ERROR "Use --static, --static-binary and --auto-download flags for WebAssembly compilation. GMP will not compile with shared libraries.")
else()
  # Older SDKs miscompile ava6: their getrusage() overruns the caller's
  # buffer by 16 bytes, silently corrupting adjacent memory (a stack
  # smash in Minisat::cpuTime and, via CoCoALib, a nulled
  # GlobalManager pointer).
  if(EMSCRIPTEN_VERSION VERSION_LESS "6.0.8")
    message(FATAL_ERROR
      "Emscripten >= 6.0.8 is required for WebAssembly builds "
      "(found '${EMSCRIPTEN_VERSION}'). See INSTALL.rst.")
  endif()
  set(CONFIGURE_CMD_WRAPPER emconfigure)
  set(TOOLCHAIN_PREFIX wasm64-unknown-emscripten)
  string(TOLOWER ".${WASM}" WASM_EXTENSION)
  set(CMAKE_EXECUTABLE_SUFFIX ${WASM_EXTENSION})
  set(WASM_FLAGS_CONFIGURE "${WASM_FLAGS}")
  set(WASM_EXPORTED_FUNCTIONS_FLAG "")
  if(WASM_FLAGS MATCHES "(-s *EXPORTED_FUNCTIONS=[^ ]+)")
    set(WASM_EXPORTED_FUNCTIONS_FLAG "${CMAKE_MATCH_1}")
    string(REPLACE "${CMAKE_MATCH_1}" "" WASM_FLAGS_CONFIGURE "${WASM_FLAGS}")
  endif()
  string(APPEND CMAKE_EXE_LINKER_FLAGS " ${WASM_FLAGS_CONFIGURE}")
endif()

#-----------------------------------------------------------------------------#
# Internal cmake variables

set(OPTIMIZATION_LEVEL 3)
set(GPL_LIBS "")

if(ENABLE_CLANG_TIDY)
  if(NOT ENABLE_ASSERTIONS)
    message(FATAL_ERROR
      "Use of clang-tidy requires building with assertions enabled.")
  endif()
  set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
  if(TREAT_WARNING_AS_ERROR)
    set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY};-warnings-as-errors=*")
  endif()
endif()

#-----------------------------------------------------------------------------#
# Determine number of threads available, used to configure (default) parallel
# execution of custom test targets (can be overriden with ARGS=-jN).

include(ProcessorCount)
ProcessorCount(CTEST_NTHREADS)
if(CTEST_NTHREADS EQUAL 0)
  set(CTEST_NTHREADS 1)
endif()

#-----------------------------------------------------------------------------#
# Build types

# Note: Module CodeCoverage requires the name of the debug build to conform
#       to cmake standards (first letter uppercase).
set(BUILD_TYPES Production Debug Testing Competition)

if(ENABLE_ASAN OR ENABLE_UBSAN OR ENABLE_TSAN)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Set the default build type to Production
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      Production CACHE STRING "Options are: ${BUILD_TYPES}" FORCE)
  # Provide drop down menu options in cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${BUILD_TYPES})
endif()

# Check if specified build type is valid.
list(FIND BUILD_TYPES ${CMAKE_BUILD_TYPE} FOUND_BUILD_TYPE)
if(${FOUND_BUILD_TYPE} EQUAL -1)
  message(FATAL_ERROR
    "'${CMAKE_BUILD_TYPE}' is not a valid build type. "
    "Available builds are: ${BUILD_TYPES}")
endif()

message(STATUS "Building ${CMAKE_BUILD_TYPE} build")
include(Config${CMAKE_BUILD_TYPE})

#-----------------------------------------------------------------------------#
# Compiler flags

check_min_compiler_version("GNU" 10)
check_min_compiler_version("Clang" 12)

add_check_c_cxx_flag("-O${OPTIMIZATION_LEVEL}")
add_check_c_cxx_flag("-Wall")
add_check_c_cxx_flag("-Wextra")
add_check_c_flag("-fexceptions")
if(EMSCRIPTEN)
  # emcc compiles C++ with -fignore-exceptions by default, which discards
  # every catch block. Enabling catching at link time alone (the wasm
  # linker flags carry -fexceptions) is not enough: the flag must also be
  # present at compile time, or exceptions unwind straight past main().
  add_check_cxx_flag("-fexceptions")
endif()
add_check_cxx_flag("-Wsuggest-override")
add_check_cxx_flag("-Wnon-virtual-dtor")
add_check_c_cxx_flag("-Wimplicit-fallthrough")
add_check_c_cxx_flag("-Wshadow")
add_check_c_cxx_flag("-Wtype-limits")
add_check_cxx_flag("-fno-operator-names")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
   AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13
   AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
  # GCC 13 introduced -Wdangling-reference, but the
  # [[gnu::no_dangling]] attribute to suppress it was only
  # added in GCC 14, so disable the warning for GCC 13.
  add_cxx_flag("-Wno-dangling-reference")
endif()

# Assume no dynamic initialization of thread-local variables in non-defining
# translation units. This option should result in better performance and works
# around crashing issues with our Windows build.
if(NOT ENABLE_CLANG_TIDY)
  add_check_cxx_flag("-fno-extern-tls-init")
endif()

if (WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,100000000")
endif ()

if (NOT USE_DEFAULT_LINKER)
  #-----------------------------------------------------------------------------#
  # Use ld.mold if available, otherwise use ld.lld if available, otherwise
  # use ld.gold if available (unless ASan is enabled).

  set(USE_EXPLICIT_LINKER_FLAG FALSE)
  execute_process(COMMAND ${CMAKE_C_COMPILER}
                  -fuse-ld=mold
                  -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_MOLD_VERSION)
  if ("${LD_MOLD_VERSION}" MATCHES "mold")
    set(USE_EXPLICIT_LINKER_FLAG TRUE)
    set(EXPLICIT_LINKER_FLAG " -fuse-ld=mold")
    message(STATUS "Using mold linker.")
  else ()
    execute_process(COMMAND ${CMAKE_C_COMPILER}
                    -fuse-ld=lld
                    -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_LLD_VERSION)
    if ("${LD_LLD_VERSION}" MATCHES "LLD")
      set(USE_EXPLICIT_LINKER_FLAG TRUE)
      set(EXPLICIT_LINKER_FLAG " -fuse-ld=lld")
      message(STATUS "Using LLD linker.")
    elseif (NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" AND NOT ENABLE_ASAN)
      # On Linux ARM64, the gold linker has a bug that causes build failures. Additionally,
      # using the gold linker with ASan triggers "cannot export local symbol" warnings.
      execute_process(COMMAND ${CMAKE_C_COMPILER}
                      -fuse-ld=gold
                      -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_GOLD_VERSION)
      if ("${LD_GOLD_VERSION}" MATCHES "GNU gold")
        set(USE_EXPLICIT_LINKER_FLAG TRUE)
        set(EXPLICIT_LINKER_FLAG " -fuse-ld=gold")
        message(STATUS "Using GNU gold linker.")
      endif ()
    endif ()
  endif ()
  if (USE_EXPLICIT_LINKER_FLAG)
    string(APPEND CMAKE_EXE_LINKER_FLAGS ${EXPLICIT_LINKER_FLAG})
    string(APPEND CMAKE_SHARED_LINKER_FLAGS ${EXPLICIT_LINKER_FLAG})
    string(APPEND CMAKE_MODULE_LINKER_FLAGS ${EXPLICIT_LINKER_FLAG})
  endif ()
endif()

#-----------------------------------------------------------------------------#
# Use interprocedural optimization if requested

if(ENABLE_IPO)
  check_ipo_supported(RESULT have_ipo_support)
  if(have_ipo_support)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(STATUS "Interprocedural optimization not supported.")
    set(ENABLE_IPO OFF)
  endif()
endif()

#-----------------------------------------------------------------------------#

# Only enable unit testing if assertions are enabled. Otherwise, unit tests
# that expect AssertionException to be thrown will fail.
if(NOT ENABLE_ASSERTIONS)
  message(STATUS "Disabling unit tests since assertions are disabled.")
  set(ENABLE_UNIT_TESTING OFF)
endif()

#-----------------------------------------------------------------------------#
# Shared/static libraries

# Embed the installation prefix as an RPATH in the executable such that the
# linker can find our libraries (such as libava6parser) when executing the
# ava6 binary. This is for example useful when installing ava6 with a custom
# prefix on macOS (e.g. when using homebrew in a non-standard directory). If
# we do not set this option, then the linker will not be able to find the
# required libraries when trying to run ava6.
#
# Also embed the installation prefix of the installed contrib libraries as an
# RPATH. This allows to install a dynamically linked binary that depends on
# dynamically linked libraries. This is dangerous, as the installed binary
# breaks if the contrib library is removed or changes in other ways, we thus
# print a big warning and only allow if installing to a custom installation
# prefix.
#
# More information on RPATH in CMake:
# https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling
if(NOT SKIP_SET_RPATH)
  if (APPLE)
    SET(CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
  else()
    SET(CMAKE_INSTALL_RPATH "\${ORIGIN}/../${CMAKE_INSTALL_LIBDIR}")
  endif()
endif()

# Set visibility to default if unit tests are enabled. If unit tests are
# enabled, we also check if we can execute white box unit tests (some versions
# of Clang have issues with the required flag).
set(ENABLE_WHITEBOX_UNIT_TESTING OFF)
if(ENABLE_UNIT_TESTING)
  set(CMAKE_CXX_VISIBILITY_PRESET default)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN 0)

  # Check if Clang version has the bug that was fixed in
  # https://reviews.llvm.org/D93104
  set(ENABLE_WHITEBOX_UNIT_TESTING ON)
  check_cxx_compiler_flag("-faccess-control" HAVE_CXX_FLAGfaccess_control)
  if(NOT HAVE_CXX_FLAGfaccess_control)
    set(ENABLE_WHITEBOX_UNIT_TESTING OFF)
    message(STATUS "Disabling white box unit tests")
  endif()
endif()

#-----------------------------------------------------------------------------#
# Enable the ctest testing framework

# This needs to be enabled here rather than in subdirectory test in order to
# allow calling ctest from the root build directory.
enable_testing()

#-----------------------------------------------------------------------------#
# Check options, find packages and configure build.

if(BUILD_SHARED_LIBS)
  if (WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .dll .lib .a)
  else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .so .dylib .a)
  endif()
else()
  if (WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)
  else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
  endif()
endif()

# Find a Python interpreter on the system. It serves two exclusive purposes:
# either to be used when the '--auto-download' option is not specified, or
# to create a virtual environment when the '--auto-download' option is given.
# In the latter case, it is also used to install the Python bindings (if built).
if(DEFINED BUILD_BINDINGS_PYTHON_VERSION)
  find_package(Python ${BUILD_BINDINGS_PYTHON_VERSION}
    EXACT COMPONENTS Interpreter REQUIRED
  )
else()
  find_package(Python COMPONENTS Interpreter REQUIRED)
endif()

# Keep the PATH to the Python binary before setting venv.
# Use a CACHE variable to remember the original value.
# We use this path in the installation step of the Python bindings.
set(Python_BASE_EXECUTABLE "${Python_EXECUTABLE}"
    CACHE STRING "Base Python executable")

macro(create_venv)
  message(STATUS "Creating Python virtual environment")
  # Use packages installed in the base installation if they're already present...

    set(VENV_SYSTEM_PACKAGES_FLAG "--system-site-packages")

  execute_process (
    COMMAND
      "${Python_EXECUTABLE}" -m venv ${VENV_SYSTEM_PACKAGES_FLAG} "${VENV_PATH}"
    RESULT_VARIABLE VENV_CMD_EXIT_CODE
  )
  if(VENV_CMD_EXIT_CODE)
    message(FATAL_ERROR "Could not create Python virtual environment")
  endif()
endmacro()

if(ENABLE_AUTO_DOWNLOAD)
  if(USE_PYTHON_VENV)
    # Set up Python venv if one doesn't exist already
    set(VENV_PATH "${PROJECT_BINARY_DIR}/venv-${Python_VERSION}")
    if(NOT EXISTS ${VENV_PATH})
      create_venv()
    endif()

    # Set up variables to find Python within the virtual environment
    set(ENV{VIRTUAL_ENV} "${VENV_PATH}")
    unset(Python_EXECUTABLE CACHE)
    if(EXISTS ${VENV_PATH}/bin/python3)
      set(Python_EXECUTABLE ${VENV_PATH}/bin/python3)
    elseif(EXISTS ${VENV_PATH}/Scripts/python.exe)
      set(Python_EXECUTABLE ${VENV_PATH}/Scripts/python.exe)
    else()
      message(FATAL_ERROR "Could not find Python interpreter in venv")
    endif()

    # Find the Python interpreter within the virtual environment
    find_package(Python COMPONENTS Interpreter REQUIRED)
  endif()
  find_package(Pip REQUIRED)
endif()

find_package(GMP 6.3 REQUIRED)

if(ENABLE_ASAN)
  # -fsanitize=address requires CMAKE_REQUIRED_FLAGS to be explicitely set,
  # otherwise the -fsanitize=address check will fail while linking.
  set(CMAKE_REQUIRED_FLAGS -fsanitize=address)
  add_required_c_cxx_flag("-fsanitize=address")
  unset(CMAKE_REQUIRED_FLAGS)
  add_required_c_cxx_flag("-fno-omit-frame-pointer")
  add_check_c_cxx_flag("-fsanitize-recover=address")
endif()

if(ENABLE_UBSAN)
  add_required_c_cxx_flag("-fsanitize=undefined")
  add_definitions(-DAVA6_USE_UBSAN)
endif()

if(ENABLE_TSAN)
  # -fsanitize=thread requires CMAKE_REQUIRED_FLAGS to be explicitely set,
  # otherwise the -fsanitize=thread check will fail while linking.
  set(CMAKE_REQUIRED_FLAGS -fsanitize=thread)
  add_required_c_cxx_flag("-fsanitize=thread")
  unset(CMAKE_REQUIRED_FLAGS)
endif()

if(ENABLE_ASSERTIONS)
  add_definitions(-DAVA6_ASSERTIONS)
else()
  add_definitions(-DNDEBUG)
endif()

if(ENABLE_COVERAGE)
  include(CodeCoverage)
  append_coverage_compiler_flags()
  add_definitions(-DAVA6_COVERAGE)
  setup_code_coverage_fastcov(
    NAME coverage
    PATH "${PROJECT_SOURCE_DIR}"
    EXCLUDE "${PROJECT_BINARY_DIR}/deps/*"
    DEPENDENCIES ava6-bin
  )
endif()

if(ENABLE_DEBUG_CONTEXT_MM)
  add_definitions(-DAVA6_DEBUG_CONTEXT_MEMORY_MANAGER)
endif()

if(ENABLE_DEBUG_SYMBOLS)
  add_check_c_cxx_flag("-ggdb3")
  if(NOT SKIP_COMPRESS_DEBUG)
    add_check_c_cxx_flag("-gz")
  endif()
endif()

if(ENABLE_MUZZLE)
  add_definitions(-DAVA6_MUZZLE)
endif()

if(ENABLE_PROFILING)
  add_definitions(-DAVA6_PROFILING)
  add_check_c_cxx_flag("-pg")
endif()

if(ENABLE_TRACING)
  add_definitions(-DAVA6_TRACING)
endif()



if(ENABLE_STATISTICS)
  add_definitions(-DAVA6_STATISTICS_ON)
endif()

if(ENABLE_VALGRIND)
  find_package(Valgrind REQUIRED)
  add_definitions(-DAVA6_VALGRIND)
endif()

find_package(CaDiCaL REQUIRED)

  set(AVA6_USE_CLN_IMP 0)
  set(AVA6_USE_GMP_IMP 1)






  set(AVA6_USE_POLY_IMP 0)




if(USE_EDITLINE)
  find_package(Editline REQUIRED)
  set(HAVE_LIBEDITLINE 1)
endif()

# SymFPU is always needed for the symbolic FP word blaster in the theory solver.


  set(AVA6_USE_MPFR FALSE)


if(GPL_LIBS)
  if(NOT ENABLE_GPL)
    message(FATAL_ERROR
      "Bad configuration detected: BSD-licensed code only, but also requested "
      "GPLed libraries: ${GPL_LIBS}. (Configure with --gpl to allow GPLed "
      "libraries.)")
  endif()
  set(AVA6_GPL_DEPS 1)
endif()

#-----------------------------------------------------------------------------#
# Provide targets to inspect iwyu suggestions

include(IWYU)
include(fuzzing-murxla)

#-----------------------------------------------------------------------------#

include(ConfigureAva6)
if(BUILD_SHARED_LIBS)
  set(AVA6_STATIC_BUILD OFF)
else()
  set(AVA6_STATIC_BUILD ON)
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    ava6_set_option(STATIC_BINARY ON)
  endif()
endif()

# Re-apply EXPORTED_FUNCTIONS after configure checks complete
if(NOT "${WASM_EXPORTED_FUNCTIONS_FLAG}" STREQUAL "")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " ${WASM_EXPORTED_FUNCTIONS_FLAG}")
endif()

#-----------------------------------------------------------------------------#
# Add subdirectories

# Include public headers
include_directories(include)
include_directories(${PROJECT_BINARY_DIR}/include)

# Install public headers
install(DIRECTORY include/ava6 DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

add_subdirectory(src)





add_subdirectory(test)

include(target-graphs)

#-----------------------------------------------------------------------------#
# Package configuration
#
# Export ava6 targets to support find_package(ava6) in other cmake projects.

include(CMakePackageConfigHelpers)

# If we install a dynamically linked binary that also uses dynamically used
# libraries from deps/install/lib, we need to be cautious. Changing these
# shared libraries from deps/install/lib most probably breaks the binary.
# We only allow such an installation for custom installation prefixes
# (in the assumption that only reasonably experienced users use this and
# also that custom installation prefixes are not used for longer periods of
# time anyway). Also, we print a big warning with further instructions.
if(BUILD_SHARED_LIBS)
  # Get the libraries that ava6 links against
  get_target_property(libs ava6 INTERFACE_LINK_LIBRARIES)
  set(LIBS_SHARED_FROM_DEPS "")
  foreach(lib ${libs})
    # Filter out those that are linked dynamically and come from deps/install
    if(lib MATCHES ".*/deps/install/lib/.*\.so")
      list(APPEND LIBS_SHARED_FROM_DEPS ${lib})
    endif()
  endforeach()
  list(LENGTH LIBS_SHARED_FROM_DEPS list_len)
  # Check if we actually use such "dangerous" libraries
  if(list_len GREATER 0)
    # Print a generic warning
    install(CODE "message(WARNING \"You are installing a dynamically linked \
    binary of ava6 which may be a problem if you are using any dynamically \
    linked third-party library that you obtained through one of the \
    contrib/get-xxx scripts. The binary uses the rpath mechanism to find these \
    locally, hence executing such a contrib script removing the \
    \\\"deps/install\\\" folder most probably breaks the installed binary! \
    Consider installing the dynamically linked dependencies on your system \
    manually or link ava6 statically.\")")
    # Print the libraries in question
    foreach(lib ${LIBS_SHARED_FROM_DEPS})
      install(CODE "message(WARNING \"The following library is used by the ava6 binary: ${lib}\")")
    endforeach()
    # Check if we use a custom installation prefix
    if(CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
      install(CODE "message(FATAL_ERROR \"To avoid installing a \
      soon-to-be-broken binary, system-wide installation is disabled if the \
      binary depends on locally-built shared libraries.\")")
    else()
      install(CODE "message(WARNING \"You have selected a custom install \
      directory ${CMAKE_INSTALL_PREFIX}, so we expect you understood the \
      previous warning and know what you are doing.\")")
    endif()
  endif()
endif()

install(EXPORT ava6-targets
  FILE ava6Targets.cmake
  NAMESPACE ava6::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ava6)

configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/ava6Config.cmake.in
  ${PROJECT_BINARY_DIR}/cmake/ava6Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ava6
  PATH_VARS CMAKE_INSTALL_LIBDIR
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/ava6ConfigVersion.cmake
  VERSION ${AVA6_VERSION}
  COMPATIBILITY ExactVersion
)

install(FILES
  ${PROJECT_BINARY_DIR}/cmake/ava6Config.cmake
  ${PROJECT_BINARY_DIR}/ava6ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ava6
)


#-----------------------------------------------------------------------------#
# Print build configuration

# Set colors.
if(NOT WIN32)
  string(ASCII 27 Esc)
  set(Green "${Esc}[32m")
  set(Blue "${Esc}[1;34m")
  set(ResetColor "${Esc}[m")
endif()

# Convert build type to lower case.
string(TOLOWER ${CMAKE_BUILD_TYPE} AVA6_BUILD_PROFILE_STRING)

# Get all definitions added via add_definitions.
get_directory_property(AVA6_DEFINITIONS COMPILE_DEFINITIONS)
string(REPLACE ";" " " AVA6_DEFINITIONS "${AVA6_DEFINITIONS}")

message("")
print_info("ava6 ${AVA6_VERSION}")
message("")
print_config("Build profile             " "${AVA6_BUILD_PROFILE_STRING}")
message("")
print_config("Assertions                " ${ENABLE_ASSERTIONS})
print_config("Debug symbols             " ${ENABLE_DEBUG_SYMBOLS})
print_config("Debug context mem mgr     " ${ENABLE_DEBUG_CONTEXT_MM})
message("")
print_config("Muzzle                    " ${ENABLE_MUZZLE})
print_config("Statistics                " ${ENABLE_STATISTICS})
print_config("Tracing                   " ${ENABLE_TRACING})
message("")
print_config("ASan                      " ${ENABLE_ASAN})
print_config("UBSan                     " ${ENABLE_UBSAN})
print_config("TSan                      " ${ENABLE_TSAN})
print_config("Coverage (gcov)           " ${ENABLE_COVERAGE})
print_config("Profiling (gprof)         " ${ENABLE_PROFILING})
print_config("Unit tests                " ${ENABLE_UNIT_TESTING})
print_config("Valgrind                  " ${ENABLE_VALGRIND})
message("")
print_config("Shared build              " ${BUILD_SHARED_LIBS})
print_config("Interprocedural opt.      " ${ENABLE_IPO})
message("")

  print_config("MP library                " "gmp" FOUND_SYSTEM ${GMP_FOUND_SYSTEM})



print_config("Editline                  " ${USE_EDITLINE})
message("")
message("")
if(GLPK_DIR)
endif()
if(PYTHONIC_PATH)
  print_config("Pythonic API path         " ${PYTHONIC_PATH})
endif()
message("")
print_config("CPPLAGS (-D...)" "${AVA6_DEFINITIONS}")
print_config("CXXFLAGS       " "${CMAKE_CXX_FLAGS}")
print_config("CFLAGS         " "${CMAKE_C_FLAGS}")
print_config("Linker flags   " "${CMAKE_EXE_LINKER_FLAGS}")
message("")
print_config("Install prefix " "${CMAKE_INSTALL_PREFIX}")
message("")

message("${Blue}ava6 license:${ResetColor} modified BSD (see COPYING)")

if("${CMAKE_GENERATOR}" STREQUAL "Ninja")
  set(BUILD_COMMAND_NAME "ninja")
else()
  set(BUILD_COMMAND_NAME "make")
endif()

message("")
message("Now just type '${BUILD_COMMAND_NAME}', "
        "followed by '${BUILD_COMMAND_NAME} check' "
        "or '${BUILD_COMMAND_NAME} install'.")
message("")
