ExternalProject_Add(EXT_CRASH_LOG
  #GIT_REPOSITORY /home/san/masterspline/Catch/
  GIT_REPOSITORY https://github.com/masterspline/crash_log.git
  GIT_TAG master # use branch name, tag or commit hash
  UPDATE_COMMAND ""
  #PATCH_COMMAND patch -p1 -t -N < ${CMAKE_SOURCE_DIR}/cmake/patch/Catch.patch
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/InstalledExternals
  #CONFIGURE_COMMAND ""
  #BUILD_COMMAND ""
  #INSTALL_COMMAND ""
)
