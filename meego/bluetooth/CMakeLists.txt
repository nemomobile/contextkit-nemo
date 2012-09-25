pkg_check_modules(BTQT bluetooth-qt)
set(PROVIDER btmobility)
set(INTERFACE bluetooth)

CKIT_GENERATE_CONTEXT(${INTERFACE} ${PROVIDER})
CKIT_GENERATE_TEST_MAIN(${INTERFACE} ${PROVIDER})

include_directories(
  ${BTQT_INCLUDE_DIRS}
)

link_directories(
  ${BTQT_LIBRARY_DIRS}
)

set(SRC
  bluetoothprovider.cpp
  )

set(HDRS
  bluetoothprovider.h
  )

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_ckit_plugin(${PROVIDER} MODULE ${SRC} ${CSRC} ${MOC_SRC})
target_link_libraries(${PROVIDER} ${BTQT_LIBRARIES})

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
