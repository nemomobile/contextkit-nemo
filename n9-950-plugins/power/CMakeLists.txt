set(CONTEXT power.context)
set(CONTEXT_SRC include/power_properties.hpp)
set(PROVIDER power)

include_directories(./include)

add_custom_command(
  OUTPUT ${CONTEXT}
  COMMAND awk -f ${CMAKE_SOURCE_DIR}/scripts/gen_context.awk
  -v PROVIDER=${PROVIDER} ${CONTEXT_SRC} > ${CONTEXT}
  DEPENDS ${CONTEXT_SRC}
  )

ADD_CUSTOM_TARGET(power_properties
  ALL DEPENDS ${CONTEXT}
  )

add_subdirectory(src)

install(FILES ${CONTEXT} DESTINATION share/contextkit/providers)
