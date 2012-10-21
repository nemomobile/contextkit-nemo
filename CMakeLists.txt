PROJECT(CONTEXTKIT_PLUGINS)
cmake_minimum_required(VERSION 2.8)

set(CKIT_INCLUDES "${CMAKE_SOURCE_DIR}/include")

find_package(Qt4 4.7.0 REQUIRED)
find_package(PkgConfig)

pkg_check_modules(CONTEXTSUBSCRIBER contextsubscriber-1.0)
pkg_check_modules(CONTEXTPROVIDER contextprovider-1.0)

set(QT_PLUGIN_LIBRARIES
  ${QT_QTCORE_LIBRARY}
  ${CONTEXTSUBSCRIBER_LIBRARIES}
  ${CONTEXTPROVIDER_LIBRARIES}
  )

include(${QT_USE_FILE})

set(CKIT_CMAKE_DIR ${CMAKE_SOURCE_DIR}/cmake)
include(${CKIT_CMAKE_DIR}/ForVerLT288.cmake)
include(${CKIT_CMAKE_DIR}/CKit.cmake)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -W -Wall -Wextra -Werror -g -O2")

#-Wno-psabi is to remove next g++ warning/note:
#the mangling of 'va_list' has changed in GCC 4.4
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wno-psabi -std=c++0x")

include_directories(
  ${QT_INCLUDE_DIR}
  ${CONTEXTSUBSCRIBER_INCLUDE_DIRS}
  ${CONTEXTPROVIDER_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/include
)

link_directories(
  ${CONTEXTSUBSCRIBER_LIBRARY_DIRS}
  ${CONTEXTPROVIDER_LIBRARY_DIRS}
)

add_subdirectory(include/contextkit_props)

if(${PLATFORM} STREQUAL "N9_50")
  add_subdirectory(maemo)
elseif(${PLATFORM} STREQUAL "N900")
  add_subdirectory(maemo)
elseif(${PLATFORM} STREQUAL "NEMO_SHARED")
  add_subdirectory(maemo)
  add_subdirectory("meego/upower")
  add_subdirectory("meego/keyboard-generic")
  add_subdirectory("meego/cellular")
  add_subdirectory("meego/connman")
  add_subdirectory("meego/phone")
  add_subdirectory("meego/media")
  add_subdirectory("meego/location-gypsy")
  add_subdirectory("meego/location-skyhook")
endif(${PLATFORM} STREQUAL "N9_50")
