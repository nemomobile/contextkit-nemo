pkg_check_modules(X11 x11)
set(PROVIDER session)
set(CONTEXT session.context)

include_directories(
  ${X11_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/common
)

link_directories(
  ${X11_LIBRARY_DIRS}
)

set(SRC
  sessionplugin.cpp
  )

set(HDRS
  sessionplugin.h
  )

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_definitions(-DQT_SHARED)
add_library(${PROVIDER} MODULE ${SRC} ${CSRC} ${MOC_SRC})
set_target_properties(${PROVIDER} PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(${PROVIDER} common)

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
install(FILES ${CONTEXT} DESTINATION share/contextkit/providers)
