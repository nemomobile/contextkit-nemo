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

add_ckit_plugin(${PROVIDER} MODULE ${SRC} ${MOC_SRC})
TARGET_LINK_LIBRARIES(${PROVIDER} common)

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
