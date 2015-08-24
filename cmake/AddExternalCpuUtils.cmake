ExternalProject_Add(EXT_CPU_UTILS
  GIT_REPOSITORY https://github.com/masterspline/cpu_utils.git
  GIT_TAG master # use branch name, tag or commit hash
  # URL "https://github.com/masterspline/cpu_utils/archive/master.zip"
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property(EXT_CPU_UTILS SOURCE_DIR)
include_directories(${SOURCE_DIR}/include)
