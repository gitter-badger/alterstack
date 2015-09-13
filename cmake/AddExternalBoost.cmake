if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(EXT_BOOST_DOWNLOAD_URL "http://sourceforge.net/projects/boost/files/boost/1.59.0/boost_1_59_0.zip")
    set(EXT_BOOST_URL_HASH "08d29a2d85db3ebc8c6fdfa3a1f2b83c")
else()
    set(EXT_BOOST_DOWNLOAD_URL "http://sourceforge.net/projects/boost/files/boost/1.59.0/boost_1_59_0.tar.bz2")
    set(EXT_BOOST_URL_HASH "6aa9a5c6a4ca1016edd0ed1178e3cb87")
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
  URL_HASH MD5=${EXT_BOOST_URL_HASH}
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ./bootstrap.sh --with-libraries=context --prefix=${CMAKE_BINARY_DIR}/InstalledExternals
  BUILD_IN_SOURCE 1
  BUILD_COMMAND ""
  INSTALL_COMMAND ./b2 install
)
#ExternalProject_Get_Property(EXT_BOOST INSTALL_DIR)
set(BOOST_ROOT "${CMAKE_BINARY_DIR}/InstalledExternals")
