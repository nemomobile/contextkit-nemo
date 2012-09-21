pkg_check_modules(DBUS dbus-1)
pkg_check_modules(MLITE mlite)
pkg_check_modules(GCONF gconf-2.0)

set(PROVIDER upower)
set(CONTEXT upower.context)

include_directories(
  ${DBUS_INCLUDE_DIRS}
  ${MLITE_INCLUDE_DIRS}
  ${GCONF_INCLUDE_DIRS}
)

link_directories(
  ${DBUS_LIBRARY_DIRS}
  ${MLITE_LIBRARY_DIRS}
  ${GCONF_LIBRARY_DIRS}
)

set(SRC
  powerplugin.cpp
  )

set(HDRS
  powerplugin.h
  )

qt4_add_dbus_interface_no_ns(SRC DeviceKit.Power.xml power)
qt4_add_dbus_interface_no_ns(SRC DeviceKit.Power.Device.xml device)

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_definitions(-DQT_SHARED)
add_library(${PROVIDER} MODULE ${SRC} ${CSRC} ${MOC_SRC})
set_target_properties(${PROVIDER} PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(${PROVIDER}
  ${QT_PLUGIN_LIBRARIES}
  ${QT_QTDBUS_LIBRARY}
  ${MLITE_LIBRARIES}
  ${GCONF_LIBRARIES}
  )

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
install(FILES ${CONTEXT} DESTINATION share/contextkit/providers)
