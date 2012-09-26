pkg_check_modules(DBUS dbus-1)

set(PROVIDER meego-music)
set(INTERFACE media)

CKIT_GENERATE_CONTEXT(${INTERFACE} ${PROVIDER})
CKIT_GENERATE_TEST_MAIN(${INTERFACE} ${PROVIDER})

include_directories(
  ${DBUS_INCLUDE_DIRS}
)

link_directories(
  ${DBUS_LIBRARY_DIRS}
)

set(SRC
  mediaprovider.cpp
  )

set(HDRS
  mediaprovider.h
  )

set(MUS_IF com.meego.app.Music.xml)
qt4_add_dbus_interface_no_ns(SRC ${MUS_IF} music Music)

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_ckit_plugin(${PROVIDER} MODULE ${SRC} ${MOC_SRC})
target_link_libraries(${PROVIDER} ${QT_QTDBUS_LIBRARY})

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
