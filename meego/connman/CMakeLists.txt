pkg_check_modules(CMAN_QT connman-qt4)
set(PROVIDER connman)
set(INTERFACE internet)

CKIT_GENERATE_CONTEXT(${INTERFACE} ${PROVIDER})
CKIT_GENERATE_TEST_MAIN(${INTERFACE} ${PROVIDER})

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

add_ckit_plugin(${PROVIDER} MODULE ${SRC} ${MOC_SRC})
target_link_libraries(${PROVIDER} ${CMAN_QT_LIBRARIES})

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
