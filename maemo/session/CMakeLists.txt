pkg_check_modules(X11 x11)
set(PROVIDER session)
set(INTERFACE session)

CKIT_GENERATE_CONTEXT(${INTERFACE} ${PROVIDER})
CKIT_GENERATE_TEST_MAIN(${INTERFACE} ${PROVIDER})

include_directories(
  ${X11_INCLUDE_DIRS}
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
TARGET_LINK_LIBRARIES(${PROVIDER}
  ${QT_PLUGIN_LIBRARIES}
  ${X11_LIBRARIES}
  common)

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
