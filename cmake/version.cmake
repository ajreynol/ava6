###############################################################################
# This file is part of the cvc5 project.
#
# Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
# in the top-level source directory and their institutional affiliations.
# All rights reserved.  See the file COPYING in the top-level source
# directory for licensing information.
# #############################################################################
#
# Responsible to identify the version of this source tree, expose this version
# information to the rest of cmake and properly update the versioninfo.cpp. 
#
# Note that the above responsibilities are split among configure and build
# time. To achieve this, this file is both executed as a part of the regular
# cmake setup during configure time, but also adds a special target to call
# itself during build time to always keep versioninfo.cpp updated.
##

if(CMAKE_SCRIPT_MODE_FILE)
  # was run as standalone script
  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
else()
  # was run within the overall cmake project
  # add target to update versioninfo.cpp at build time
  add_custom_target(gen-versioninfo
    COMMAND ${CMAKE_COMMAND}
      -DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
      -DPROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}
      -P ${PROJECT_SOURCE_DIR}/cmake/version.cmake
  )
endif()

# include basic version information
include(version-base)

if(AVA6_IS_RELEASE STREQUAL "false")
  # increment patch part of version (AVA6_LAST_RELEASE + 0.0.1)
  set(NEXT_AVA6_VERSION ${AVA6_LAST_RELEASE})
  string(REGEX MATCHALL "[0-9]+" VERSION_LIST "${NEXT_AVA6_VERSION}")
  list(LENGTH VERSION_LIST VERSION_LIST_LENGTH)
  # append .0 until we have a patch part
  while(VERSION_LIST_LENGTH LESS "3")
    list(APPEND VERSION_LIST "0")
    list(LENGTH VERSION_LIST VERSION_LIST_LENGTH)
  endwhile()
  # increment patch part
  list(GET VERSION_LIST 2 VERSION_LAST_NUMBER)
  list(REMOVE_AT VERSION_LIST 2)
  math(EXPR VERSION_LAST_NUMBER "${VERSION_LAST_NUMBER} + 1")
  list(APPEND VERSION_LIST ${VERSION_LAST_NUMBER})
  # join version string into NEXT_AVA6_VERSION
  list(GET VERSION_LIST 0 NEXT_AVA6_VERSION)
  while(VERSION_LIST_LENGTH GREATER "1")
    list(REMOVE_AT VERSION_LIST 0)
    list(GET VERSION_LIST 0 TMP)
    set(NEXT_AVA6_VERSION "${NEXT_AVA6_VERSION}.${TMP}")
    list(LENGTH VERSION_LIST VERSION_LIST_LENGTH)
  endwhile()

  set(AVA6_VERSION "${NEXT_AVA6_VERSION}.dev")
  set(AVA6_VERSION_NUMBER "${NEXT_AVA6_VERSION}")
  # Set SOVERSION to ava6 major version number
  string(REGEX MATCH "^[0-9]+" AVA6_SOVERSION "${NEXT_AVA6_VERSION}")
  set(AVA6_FULL_VERSION "${NEXT_AVA6_VERSION}.dev")
  # Python: Development segment MUST follow the format devN, where N is a sequence of digits.
  set(AVA6_WHEEL_VERSION "${NEXT_AVA6_VERSION}.dev0")
  # Java: Development version is distinguished by the suffix "-SNAPSHOT"
  set(AVA6_MAVEN_VERSION "${NEXT_AVA6_VERSION}-SNAPSHOT")
endif()

# now use git to retrieve additional version information
find_package(Git)
if(GIT_FOUND)
  # git is available

  # Call git. If result is 0 and prints "true", it is a git repository
  execute_process(
      COMMAND ${GIT_EXECUTABLE} -C ${PROJECT_SOURCE_DIR} rev-parse --verify HEAD
      RESULT_VARIABLE GIT_RESULT
      ERROR_QUIET
      OUTPUT_VARIABLE GIT_INSIDE_WORK_TREE
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(GIT_RESULT EQUAL 0)
    # it is a git working copy

    set(GIT_BUILD "true")
    # get current git branch
    execute_process(
        COMMAND ${GIT_EXECUTABLE} -C ${PROJECT_SOURCE_DIR} rev-parse --abbrev-ref HEAD
        RESULT_VARIABLE GIT_RESULT
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # get current git commit
    execute_process(
        COMMAND ${GIT_EXECUTABLE} -C ${PROJECT_SOURCE_DIR} rev-parse --short HEAD
        RESULT_VARIABLE GIT_RESULT
        OUTPUT_VARIABLE GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(AVA6_IS_RELEASE STREQUAL "false")
      set(AVA6_FULL_VERSION "${AVA6_FULL_VERSION}+${GIT_BRANCH}@${GIT_COMMIT}")
      # A local version id MUST start with + and contain only ASCII letters, digits, and periods.
      set(AVA6_WHEEL_VERSION "${AVA6_WHEEL_VERSION}+${GIT_COMMIT}")
    endif()

    # result is != 0 if worktree is dirty
    # note: git diff HEAD shows both staged and unstaged changes.
    execute_process(
      COMMAND ${GIT_EXECUTABLE} -C ${PROJECT_SOURCE_DIR} diff-index --quiet HEAD
      RESULT_VARIABLE GIT_RESULT
    )
    if(GIT_RESULT EQUAL 0)
      set(GIT_DIRTY_MSG "")
    else()
      set(GIT_DIRTY_MSG " with local modifications")
      set(AVA6_VERSION "${AVA6_VERSION}-modified")
      set(AVA6_FULL_VERSION "${AVA6_FULL_VERSION}-modified")
    endif()

    set(AVA6_GIT_INFO "git ${GIT_COMMIT} on branch ${GIT_BRANCH}${GIT_DIRTY_MSG}")
  endif()
endif()

# actually configure versioninfo.cpp
configure_file(
    ${PROJECT_SOURCE_DIR}/src/base/versioninfo.cpp.in ${PROJECT_BINARY_DIR}/src/base/versioninfo.cpp)
