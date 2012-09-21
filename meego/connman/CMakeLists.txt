pkg_check_modules(CMAN_QT connman-qt4)
set(PROVIDER connman)
set(CONTEXT connman.context)

include_directories(
  ${CMAN_QT_INCLUDE_DIRS}
)

link_directories(
  ${CMAN_QT_LIBRARY_DIRS}
)

set(SRC
  connmanprovider.cpp
  )

set(HDRS
  connmanprovider.h
  )

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_definitions(-DQT_SHARED)
add_library(${PROVIDER} MODULE ${SRC} ${CSRC} ${MOC_SRC})
set_target_properties(${PROVIDER} PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(${PROVIDER}
  ${QT_PLUGIN_LIBRARIES}
  ${CMAN_QT_LIBRARIES}
  )

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
install(FILES ${CONTEXT} DESTINATION share/contextkit/providers)
