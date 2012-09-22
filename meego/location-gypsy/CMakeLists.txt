pkg_check_modules(DBUS dbus-1)
pkg_check_modules(MLITE mlite)

set(PROVIDER location_gypsy)
set(CONTEXT location_gypsy.context)

include_directories(
  ${DBUS_INCLUDE_DIRS}
  ${MLITE_INCLUDE_DIRS}
)

link_directories(
  ${DBUS_LIBRARY_DIRS}
  ${MLITE_LIBRARY_DIRS}
)

set(SRC
  location.cpp
  )

set(HDRS
  location.h
  )

set(GYP_IF org.freedesktop.Gypsy.xml)
set_source_files_properties(${GYP_IF}
  PROPERTIES NO_NAMESPACE TRUE INCLUDE gypsytypes.h
  )

qt4_add_dbus_interface_cls(SRC ${GYP_IF} gypsy)

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_definitions(-DQT_SHARED)
add_library(${PROVIDER} MODULE ${SRC} ${CSRC} ${MOC_SRC})
set_target_properties(${PROVIDER} PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(${PROVIDER}
  ${QT_PLUGIN_LIBRARIES}
  ${QT_QTDBUS_LIBRARY}
  ${MLITE_LIBRARIES}
  )

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
install(FILES ${CONTEXT} DESTINATION share/contextkit/providers)
