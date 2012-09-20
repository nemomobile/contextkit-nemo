PROJECT(contextkit_power_mon)

set(power_mon_HDRS
  power_mon.hpp
  )

set(power_mon_SRC
  power_mon.cpp
  )

qt4_wrap_cpp(MOC_SRC ${power_mon_HDRS})

add_definitions(-DQT_SHARED)
add_executable(power_mon ${power_mon_SRC} ${MOC_SRC})

TARGET_LINK_LIBRARIES(power_mon
  ${QT_QTCORE_LIBRARY}
  ${CONTEXTSUBSCRIBER_LIBRARIES}
)

install(TARGETS power_mon DESTINATION bin)
