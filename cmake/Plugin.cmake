MACRO(ADD_CKIT_PLUGIN _name)
  message("@${ARGN}@")
  add_definitions(-DQT_SHARED)
  add_library(${_name} MODULE ${ARGN})
  set_target_properties(${_name} PROPERTIES PREFIX "")
  target_link_libraries(${_name}
    ${QT_QTCORE_LIBRARY}
    ${CONTEXTSUBSCRIBER_LIBRARIES}
    )

ENDMACRO(ADD_CKIT_PLUGIN)