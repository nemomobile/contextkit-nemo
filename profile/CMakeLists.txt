pkg_check_modules(DBUS dbus-1)
set(PROVIDER profile)
set(CONTEXT profile.context)

include_directories(
  ${DBUS_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/common
)

link_directories(
  ${DBUS_LIBRARY_DIRS}
)

set(SRC
  profileplugin.cpp
  )

set(HDRS
  profileplugin.h
  )

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_definitions(-DQT_SHARED)
add_library(${PROVIDER} MODULE ${SRC} ${CSRC} ${MOC_SRC})
set_target_properties(${PROVIDER} PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(${PROVIDER} common)

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
install(FILES ${CONTEXT} DESTINATION share/contextkit/providers)
