cmake_minimum_required(VERSION 3.0)
enable_language(C)
project(elibgit2)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE CACHE BOOL "pic" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "shared" FORCE)
set(BUILD_CLAR OFF CACHE BOOL "clar" FORCE)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DEGIT_DEBUG")

if(USE_SYSTEM_LIBGIT2)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(git2 REQUIRED IMPORTED_TARGET libgit2)
else()
  add_subdirectory(libgit2)
  find_library(git2 libgit2.a)
endif()

add_subdirectory(src)

enable_testing()

# NOTE: These test names must be replicated in .appveyor-script.ps1
# for the Windows CI. Fixing this is still TODO.
set(EGIT_TESTS
  annotated-commit
  blame
  blob
  branch
  checkout
  cherrypick
  commit
  config
  describe
  diff
  graph
  ignore
  index
  merge
  message
  pathspec
  reference
  reflog
  remote
  repository
  reset
  revert
  revparse
  revwalk
  signature
  status
  submodule
  tag
  tree)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  list(APPEND EGIT_TESTS refcount)
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

foreach(test ${EGIT_TESTS})
  add_test(NAME libegit2_${test} COMMAND
    emacs --batch
          -L "${CMAKE_CURRENT_BINARY_DIR}"
          -L "${CMAKE_CURRENT_SOURCE_DIR}/test"
          -l libegit2
          -l test-helper
          -l ${test}-test
          -f ert-run-tests-batch-and-exit)
endforeach(test)
