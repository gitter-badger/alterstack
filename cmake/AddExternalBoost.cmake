if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(EXT_BOOST_DOWNLOAD_URL "http://sourceforge.net/projects/boost/files/boost/1.59.0/boost_1_59_0.zip")
else()
    set(EXT_BOOST_DOWNLOAD_URL "http://sourceforge.net/projects/boost/files/boost/1.59.0/boost_1_59_0.tar.bz2")
endif()
#Bootstrap
#if((CMAKE_SYSTEM_NAME MATCHES "Windows") AND (NOT CMAKE_CROSSCOMPILING))
##        set(__BII_BOOST_BOOSTRAPER ${BII_BOOST_DIR}/bootstrap.bat ${SCOPE})
#        set(__BII_BOOST_B2         ${BII_BOOST_DIR}/b2.exe        ${SCOPE})
#    else()
#        set(__BII_BOOST_BOOSTRAPER ${BII_BOOST_DIR}/bootstrap.sh ${SCOPE})
#        set(__BII_BOOST_B2         ${BII_BOOST_DIR}/b2           ${SCOPE})
#    endif()
ExternalProject_Add(EXT_BOOST
  #GIT_REPOSITORY https://github.com/boostorg/boost.git
  #GIT_TAG boost-1.59.0 # use branch name, tag or commit hash
  URL "${EXT_BOOST_DOWNLOAD_URL}"
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ./bootstrap.sh --with-libraries=context --prefix=${CMAKE_BINARY_DIR}/InstalledExternals
  BUILD_IN_SOURCE 1
  BUILD_COMMAND ""
  INSTALL_COMMAND ./b2 install
)
ExternalProject_Get_Property(EXT_BOOST INSTALL_DIR)
set(BOOST_ROOT "${INSTALL_DIR}/installed")

find_package(Boost 1.58.0 COMPONENTS context)
