# initial implementation is taken from cmake 2.8.8

MACRO(QT4_ADD_DBUS_INTERFACE_CLS _sources _interface _basename_pre)
  set(_basename ${_basename_pre}_interface)

  GET_FILENAME_COMPONENT(_infile ${_interface} ABSOLUTE)
  SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
  SET(_impl   ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
  SET(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)

  GET_SOURCE_FILE_PROPERTY(_nonamespace ${_interface} NO_NAMESPACE)
  IF ( _nonamespace )
    SET(_params -N -m)
  ELSE ( _nonamespace )
    SET(_params -m)
  ENDIF ( _nonamespace )

  GET_SOURCE_FILE_PROPERTY(_include ${_interface} INCLUDE)
  IF ( _include )
    SET(_params ${_params} -i ${_include})
  ENDIF ( _include )

  GET_SOURCE_FILE_PROPERTY(_class ${_interface} CLASS)
  IF ( _class )
    SET(_params ${_params} -c ${_class})
  ENDIF ( _class )

  ADD_CUSTOM_COMMAND(OUTPUT ${_impl} ${_header}
    COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} ${_params} -p ${_basename} ${_infile}
    DEPENDS ${_infile})

  SET_SOURCE_FILES_PROPERTIES(${_impl} PROPERTIES SKIP_AUTOMOC TRUE)

  QT4_GENERATE_MOC(${_header} ${_moc})

  SET(${_sources} ${${_sources}} ${_impl} ${_header} ${_moc})
  MACRO_ADD_FILE_DEPENDENCIES(${_impl} ${_moc})

ENDMACRO(QT4_ADD_DBUS_INTERFACE_CLS)

MACRO(QT4_ADD_DBUS_INTERFACE_NO_NS _sources _interface _basename _class)
  set_source_files_properties(${_interface} PROPERTIES CLASS ${_class})
  set_source_files_properties(${_interface} PROPERTIES NO_NAMESPACE TRUE)
  qt4_add_dbus_interface_cls(
    ${_sources}
    ${_interface}
    ${_basename}
    )
ENDMACRO(QT4_ADD_DBUS_INTERFACE_NO_NS)

