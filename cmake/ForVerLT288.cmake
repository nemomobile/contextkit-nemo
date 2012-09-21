# no_namespace is added only in cmake 2.8.8
# so copying this functionality to earlier versions

if(${CMAKE_VERSION} VERSION_GREATER 2.8.7)

  MACRO(QT4_ADD_DBUS_INTERFACE_288 _sources _interface _basename)
    QT4_ADD_DBUS_INTERFACE(_sources _interface _basename)
  ENDMACRO(QT4_ADD_DBUS_INTERFACE_288)

else(${CMAKE_VERSION} VERSION_GREATER 2.8.7)

  MACRO(QT4_ADD_DBUS_INTERFACE_288 _sources _interface _basename)
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

    ADD_CUSTOM_COMMAND(OUTPUT ${_impl} ${_header}
      COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} ${_params} -p ${_basename} ${_infile}
      DEPENDS ${_infile})

    SET_SOURCE_FILES_PROPERTIES(${_impl} PROPERTIES SKIP_AUTOMOC TRUE)

    QT4_GENERATE_MOC(${_header} ${_moc})

    SET(${_sources} ${${_sources}} ${_impl} ${_header} ${_moc})
    MACRO_ADD_FILE_DEPENDENCIES(${_impl} ${_moc})

  ENDMACRO(QT4_ADD_DBUS_INTERFACE_288)

endif(${CMAKE_VERSION} VERSION_GREATER 2.8.7)
