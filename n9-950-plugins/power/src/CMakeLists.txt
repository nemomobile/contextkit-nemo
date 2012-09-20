PROJECT(contextkit_power)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBME_API=11")

set(SRC
  power.cpp
  )

set(CSRC
  bmeipc.c
  )

set(HDRS
  power.hpp
  )

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_definitions(-DQT_SHARED)
add_library(${PROVIDER} MODULE ${SRC} ${CSRC} ${MOC_SRC})
set_target_properties(power PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(${PROVIDER}
  ${QT_QTCORE_LIBRARY}
  ${CONTEXTSUBSCRIBER_LIBRARIES}
  ${CONTEXTPROVIDER_LIBRARIES}
  rt
  common)

install(TARGETS power DESTINATION lib/contextkit/subscriber-plugins)
