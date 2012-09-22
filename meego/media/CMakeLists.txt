pkg_check_modules(DBUS dbus-1)

set(PROVIDER media)
set(CONTEXT media.context)

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

add_definitions(-DQT_SHARED)
add_library(${PROVIDER} MODULE ${SRC} ${CSRC} ${MOC_SRC})
set_target_properties(${PROVIDER} PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(${PROVIDER}
  ${QT_PLUGIN_LIBRARIES}
  ${QT_QTDBUS_LIBRARY}
  )

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
install(FILES ${CONTEXT} DESTINATION share/contextkit/providers)
