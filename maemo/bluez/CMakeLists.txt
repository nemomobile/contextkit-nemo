pkg_check_modules(DBUS dbus-1)
set(PROVIDER bluez)
set(INTERFACE bluetooth)

CKIT_GENERATE_CONTEXT(${INTERFACE} ${PROVIDER})
CKIT_GENERATE_TEST_MAIN(${INTERFACE} ${PROVIDER})

include_directories(
  ${DBUS_INCLUDE_DIRS}
)

link_directories(
  ${DBUS_LIBRARY_DIRS}
)

set(SRC
  bluezdevice.cpp
  bluezplugin.cpp
  )

set(HDRS
  bluezdevice.h
  bluezplugin.h
  )

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_definitions(-DQT_SHARED)
add_library(${PROVIDER} MODULE ${SRC} ${CSRC} ${MOC_SRC})
set_target_properties(${PROVIDER} PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(${PROVIDER}
  ${DBUS_LIBRARIES}
  ${QT_QTDBUS_LIBRARY}
  ${QT_PLUGIN_LIBRARIES}
  common)

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
