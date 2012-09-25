include_directories(
  ${CMAKE_SOURCE_DIR}/maemo/common
)

link_directories(
  ${CONTEXTKIT_PLUGINS_BINARY_DIR}/maemo/common
)

add_subdirectory(common)

if(${PLATFORM} STREQUAL "N9_50")
  add_subdirectory(bluez)
  add_subdirectory(kbslider)
  add_subdirectory(power)
elseif(${PLATFORM} STREQUAL "N900")
  add_subdirectory(bluez)
  add_subdirectory(kbslider)
  add_subdirectory(power)
elseif(${PLATFORM} STREQUAL "I586")
  add_subdirectory(bluez)
elseif(${PLATFORM} STREQUAL "NEMO_SHARED")
  add_subdirectory(session)
  add_subdirectory(profile)
endif(${PLATFORM} STREQUAL "N9_50")


