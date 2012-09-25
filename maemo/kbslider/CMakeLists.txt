pkg_check_modules(UDEV libudev)
set(PROVIDER kbslider)
set(INTERFACE internal_keyboard)

CKIT_GENERATE_CONTEXT(${INTERFACE} ${PROVIDER})
CKIT_GENERATE_TEST_MAIN(${INTERFACE} ${PROVIDER})

include_directories(
  ${UDEV_INCLUDE_DIRS}
)

link_directories(
  ${UDEV_LIBRARY_DIRS}
)

set(SRC
  kbsliderplugin.cpp
  )

set(HDRS
  kbsliderplugin.h
  )

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_ckit_plugin(${PROVIDER} MODULE ${SRC} ${CSRC} ${MOC_SRC})
TARGET_LINK_LIBRARIES(${PROVIDER} common)

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
