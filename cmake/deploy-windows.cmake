message("Running Windows deploy script.")
find_program(WINDEPLOYQT windeployqt)
message("Found windeployqt: ${WINDEPLOYQT}")
set(MERKAARTOR_BINARY "${CMAKE_INSTALL_PREFIX}/bin/merkaartor.exe")
message("Working on binary: ${MERKAARTOR_BINARY}")
execute_process(COMMAND ${WINDEPLOYQT} ${MERKAARTOR_BINARY})


# Remove when done debugging
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
message("Prefix is: ${CMAKE_INSTALL_PREFIX}")
execute_process(COMMAND ls ${CMAKE_INSTALL_PREFIX})
