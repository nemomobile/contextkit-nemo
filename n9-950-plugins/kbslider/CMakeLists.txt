pkg_check_modules(UDEV udev)
set(PROVIDER kbslider)
set(CONTEXT kbslider.context)

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

add_definitions(-DQT_SHARED)
add_library(${PROVIDER} MODULE ${SRC} ${CSRC} ${MOC_SRC})
set_target_properties(${PROVIDER} PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(${PROVIDER} common)

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
install(FILES ${CONTEXT} DESTINATION share/contextkit/providers)
