pkg_check_modules(UDEV libudev)
set(PROVIDER keyboard-generic)
set(INTERFACE internal_keyboard)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

CKIT_GENERATE_CONTEXT(${INTERFACE} ${PROVIDER})
CKIT_GENERATE_TEST_MAIN(${INTERFACE} ${PROVIDER})

include_directories(
  ${UDEV_INCLUDE_DIRS}
)

link_directories(
  ${UDEV_LIBRARY_DIRS}
)

set(SRC
  plugin.cpp
  )

set(HDRS
  plugin.hpp
  )

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_ckit_plugin(${PROVIDER} MODULE ${SRC} ${MOC_SRC})
TARGET_LINK_LIBRARIES(${PROVIDER} ${UDEV_LIBRARIES})

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
