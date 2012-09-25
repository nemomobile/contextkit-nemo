set(INTERFACE power)
set(PROVIDER power-bme)

CKIT_GENERATE_CONTEXT(${INTERFACE} ${PROVIDER})
CKIT_GENERATE_TEST_MAIN(${INTERFACE} ${PROVIDER})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBME_API=11")

set(SRC power.cpp)

set(CSRC bmeipc.c)

set(HDRS power.hpp)

qt4_wrap_cpp(MOC_SRC ${HDRS})

add_ckit_plugin(${PROVIDER} ${SRC} ${CSRC} ${MOC_SRC})
target_link_libraries(${PROVIDER} rt common)

install(TARGETS ${PROVIDER} DESTINATION lib/contextkit/subscriber-plugins)
