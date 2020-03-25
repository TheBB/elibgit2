file(GLOB ELIBGIT2_SRCS *.c)

add_library(egit2 SHARED ${ELIBGIT2_SRCS})
set_target_properties(egit2 PROPERTIES C_STANDARD 99)

# Emacs looks for .so on linux and OSX.
# By default CMake builds .dylib on OSX, so override that.
if(APPLE)
  set_target_properties(egit2 PROPERTIES SUFFIX .so)
endif(APPLE)

if(WIN32)
  set_target_properties(egit2 PROPERTIES PREFIX lib)
endif(WIN32)

if(USE_SYSTEM_LIBGIT2)
  target_link_libraries(egit2 PRIVATE PkgConfig::git2)
else()
  target_link_libraries(egit2 git2)
  target_include_directories(
    egit2 SYSTEM PRIVATE "${libgit2_SOURCE_DIR}/include")
endif()

if(CMAKE_COMPILER_IS_GNUCC)
  target_compile_options(egit2 PRIVATE -Wall -Wextra)
endif(CMAKE_COMPILER_IS_GNUCC)

add_custom_command(TARGET egit2 POST_BUILD COMMAND
  ${CMAKE_COMMAND} -E copy $<TARGET_FILE:egit2> "${CMAKE_CURRENT_BINARY_DIR}/..")
