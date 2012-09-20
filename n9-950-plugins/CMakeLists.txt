include_directories(
  ${CMAKE_SOURCE_DIR}/n9-950-plugins/common
)

link_directories(
  ${CONTEXTKIT_PLUGINS_BINARY_DIR}/n9-950-plugins/common
)

set(QT_PLUGIN_LIBRARIES
  ${QT_QTCORE_LIBRARY}
  ${CONTEXTSUBSCRIBER_LIBRARIES}
  ${CONTEXTPROVIDER_LIBRARIES}
  )

add_subdirectory(common)
add_subdirectory(bluez)
add_subdirectory(kbslider)
add_subdirectory(session)
add_subdirectory(profile)
add_subdirectory(power)
