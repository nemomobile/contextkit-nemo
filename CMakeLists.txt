PROJECT(CONTEXTKIT_PLUGINS)
cmake_minimum_required(VERSION 2.8)

set(CKIT_INCLUDES "${CMAKE_SOURCE_DIR}/include")

include(${CMAKE_SOURCE_DIR}/cmake/ForVerLT288.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/TestGen.cmake)

find_package(Qt4 4.7.0 REQUIRED)
find_package(PkgConfig)

include(${QT_USE_FILE})

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -W -Wall -Wextra -Werror -g -O2")

#-Wno-psabi is to remove next g++ warning/note:
#the mangling of 'va_list' has changed in GCC 4.4
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wno-psabi")

pkg_check_modules(CONTEXTSUBSCRIBER contextsubscriber-1.0)
pkg_check_modules(CONTEXTPROVIDER contextprovider-1.0)

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

set(QT_PLUGIN_LIBRARIES
  ${QT_QTCORE_LIBRARY}
  ${CONTEXTSUBSCRIBER_LIBRARIES}
  ${CONTEXTPROVIDER_LIBRARIES}
  )

if(${PLATFORM} STREQUAL "N9_50")
  add_subdirectory(maemo)
elseif(${PLATFORM} STREQUAL "N900")
  add_subdirectory(maemo)
elseif(${PLATFORM} STREQUAL "I586")
  add_subdirectory("meego/upower")
  add_subdirectory("meego/location-gypsy")
elseif(${PLATFORM} STREQUAL "I586_VM")
  add_subdirectory("meego/bluetooth")
  add_subdirectory("meego/upower")
  add_subdirectory("meego/location-gypsy")
elseif(${PLATFORM} STREQUAL "NEMO_SHARED")
  add_subdirectory("meego/cellular")
  add_subdirectory("meego/connman")
  add_subdirectory("meego/phone")
  add_subdirectory("meego/media")
endif(${PLATFORM} STREQUAL "N9_50")
