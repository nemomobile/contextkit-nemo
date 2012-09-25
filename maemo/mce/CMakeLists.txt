pkg_check_modules(DBUS dbus-1)
pkg_check_modules(MCE mce)
set(PROVIDER mce)
set(INTERFACE mce)

CKIT_GENERATE_CONTEXT(${INTERFACE} ${PROVIDER})
CKIT_GENERATE_TEST_MAIN(${INTERFACE} ${PROVIDER})

include_directories(
  ${DBUS_INCLUDE_DIRS}
  ${MCE_INCLUDE_DIRS}
)

link_directories(
  ${DBUS_LIBRARY_DIRS}
  ${MCE_LIBRARY_DIRS}
)

set(SRC
  ${PROVIDER}plugin.cpp
  )

set(HDRS
  ${PROVIDER}plugin.h
  )

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_ckit_plugin(${PROVIDER} ${SRC} ${CSRC} ${MOC_SRC})
TARGET_LINK_LIBRARIES(${PROVIDER}
  ${DBUS_LIBRARIES}
  ${MCE_LIBRARIES}
  common)

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
