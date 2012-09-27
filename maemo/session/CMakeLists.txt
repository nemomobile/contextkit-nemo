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

add_ckit_plugin(${PROVIDER} MODULE ${SRC} ${MOC_SRC})
TARGET_LINK_LIBRARIES(${PROVIDER} ${X11_LIBRARIES} common)

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
