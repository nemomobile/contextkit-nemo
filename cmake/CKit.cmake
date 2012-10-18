MACRO(ADD_CKIT_PLUGIN _name)
  add_definitions(-DQT_SHARED)
  add_library(${_name} MODULE ${ARGN})
  set_target_properties(${_name} PROPERTIES PREFIX "")
  target_link_libraries(${_name} ${QT_PLUGIN_LIBRARIES})
ENDMACRO(ADD_CKIT_PLUGIN)


MACRO(CKIT_GENERATE _target _src _interface _provider _mode)
  set(_awk_path ${CMAKE_SOURCE_DIR}/scripts)
  set(_gen_script ${_awk_path}/generator.awk)

  add_custom_command(OUTPUT ${_target}
    COMMAND gawk -vinterface=${_interface} -vmode=${_mode} -vprovider=${_provider}
    -f ${_gen_script} ${_src} > ${_target}
    DEPENDS ${_src} ${_gen_script}
    )
ENDMACRO(CKIT_GENERATE)


MACRO(CKIT_GENERATE_TEST_MAIN _interface _provider)
  set(_impl test_${_interface}.cpp)
  set(_infile ${CMAKE_SOURCE_DIR}/data/${_interface}.list)
  set(_test_name contextkit-test-${_provider})

  ckit_generate(${_impl} ${_infile} ${_interface} ${_provider} test)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CKIT_INCLUDES} -Wno-unused-function")

  qt4_wrap_cpp(_mocs ${CKIT_INCLUDES}/contextkit_test.hpp)
  add_definitions(-DQT_SHARED)
  add_executable(${_test_name} ${_impl} ${_mocs})

  TARGET_LINK_LIBRARIES(${_test_name}
    ${QT_QTCORE_LIBRARY}
    ${CONTEXTSUBSCRIBER_LIBRARIES}
    )

  install(TARGETS ${_test_name} DESTINATION bin)

ENDMACRO(CKIT_GENERATE_TEST_MAIN)


MACRO(CKIT_GENERATE_CONTEXT _interface _provider)
  set(_impl ${_provider}.context)
  set(_infile ${CMAKE_SOURCE_DIR}/data/${_interface}.list)

  ckit_generate(${_impl} ${_infile} ${_interface} ${_provider} xml)
  ADD_CUSTOM_TARGET(${_provider}_properties ALL DEPENDS ${_impl})

  install(FILES ${_impl} DESTINATION share/contextkit/providers)

ENDMACRO(CKIT_GENERATE_CONTEXT)

MACRO(CKIT_GENERATE_HEADER _interface)
  set(_impl ${_interface}.hpp)
  set(_provider UNKNOWN)
  set(_infile ${CMAKE_SOURCE_DIR}/data/${_interface}.list)

  ckit_generate(${_impl} ${_infile} ${_interface} ${_provider} header)
  add_custom_target(${_interface}_header ALL DEPENDS ${_impl})

ENDMACRO(CKIT_GENERATE_HEADER)
