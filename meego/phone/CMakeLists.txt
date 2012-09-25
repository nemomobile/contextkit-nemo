pkg_check_modules(DBUS dbus-1)

set(PROVIDER ofono)
set(INTERFACE phone)

CKIT_GENERATE_CONTEXT(${INTERFACE} ${PROVIDER})
CKIT_GENERATE_TEST_MAIN(${INTERFACE} ${PROVIDER})

include_directories(
  ${DBUS_INCLUDE_DIRS}
)

link_directories(
  ${DBUS_LIBRARY_DIRS}
)

set(SRC
  phoneprovider.cpp
  callmanager.cpp
  callitem.cpp
  callproxy.cpp
  common.cpp
  )

set(HDRS
  phoneprovider.h
  callmanager.h
  callitem.h
  callproxy.h
  )

set(VOL_IF org.ofono.callvolume.xml)
set(CMG_IF org.ofono.callmanager.xml)
set(MGR_IF org.ofono.manager.xml)
set(CAL_IF org.ofono.voicecall.xml)

set_source_files_properties(${VOL_IF} ${CMG_IF} ${MGR_IF} ${CAL_IF}
  PROPERTIES INCLUDE common.h
  )

qt4_add_dbus_interface_no_ns(SRC ${VOL_IF} callvolume CallVolume)
qt4_add_dbus_interface_no_ns(SRC ${CMG_IF} callmanager VoiceCallManager)
qt4_add_dbus_interface_no_ns(SRC ${MGR_IF} manager Manager)
qt4_add_dbus_interface_no_ns(SRC ${CAL_IF} voicecall VoiceCall)

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_ckit_plugin(${PROVIDER} MODULE ${SRC} ${CSRC} ${MOC_SRC})
target_link_libraries(${PROVIDER} ${QT_QTDBUS_LIBRARY})

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
