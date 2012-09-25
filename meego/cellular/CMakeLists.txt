pkg_check_modules(DBUS dbus-1)

set(PROVIDER cellular)
set(INTERFACE cellular)

CKIT_GENERATE_CONTEXT(${INTERFACE} ${PROVIDER})
CKIT_GENERATE_TEST_MAIN(${INTERFACE} ${PROVIDER})

include_directories(
  ${DBUS_INCLUDE_DIRS}
)

link_directories(
  ${DBUS_LIBRARY_DIRS}
)

set(SRC
  cellularprovider.cpp
  common.cpp
  )

set(HDRS
  cellularprovider.h
  )

set(MGR_IF org.ofono.manager.xml)
set(NET_IF org.ofono.networkregistration.xml) 
set(SIM_IF org.ofono.simmanager.xml)
set_source_files_properties(${MGR_IF} ${NET_IF} ${SIM_IF}
  PROPERTIES INCLUDE common.h
  )

qt4_add_dbus_interface_no_ns(SRC ${MGR_IF} manager Manager)
qt4_add_dbus_interface_no_ns(SRC ${NET_IF} ofono NetworkProperties)
qt4_add_dbus_interface_no_ns(SRC ${SIM_IF} sim SimProperties)

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_ckit_plugin(${PROVIDER} MODULE ${SRC} ${CSRC} ${MOC_SRC})
target_link_libraries(${PROVIDER} ${QT_QTDBUS_LIBRARY})

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
