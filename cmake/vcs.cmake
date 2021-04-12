
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
    # If git directory is present, we require git executable and gather our info from git commands.
    find_package(Git)
    if (NOT ${GIT_FOUND})
        message(FATAL_ERROR "Git not found. Cannot proceed with version info.")
    endif()

    # Set dependencies on git index to reconfigure on changed index/branch (like switching branches, commiting, etc.).
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/.git/index")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/.git/HEAD")

    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE  VCS_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short=12 HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE  VCS_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --dirty
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE  VCS_DESCRIBE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND ${GIT_EXECUTABLE} diff-index --quiet HEAD --
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE  VCS_DIRTY
    )

    # Workaround to update dirty flag when anything changes, as CMake can not
    # depend on a command that would check it the git way. This check is only used
    # in not dirty state to avoid unnecessary reconfigures during development.
    # There are a few drawbacks:
    #  - reconfigure is not triggered by any other change than in cpp/h file (or
    #    CMakeLists files, that is the default)
    #  - When changes are manually reverted from dirty to non-dirty state (without
    #    touching index/HEAD, for example editor undo), the flag is not updated
    #    properly
    # This workaround shall be replaced if a better way is found.
    if (NOT ${VCS_DIRTY})
        file(GLOB_RECURSE DEP_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/*.cpp" "${CMAKE_SOURCE_DIR}/*.h")
    endif()

else()
    # Git was not found, we will try to use snapshot info.
    include(${CMAKE_SOURCE_DIR}/cmake/vcs-snapshot.cmake)
    if (NOT ${VCS_SNAPSHOT})
        message(FATAL_ERROR "VCS directory does not contain a .git subdirectory, but the snapshot info is not present. Cannot build.")
    endif()
endif()


# Describe has format 0.18.4-137-gXXX, or 0.18.4 if exactly on tag. We normalize that to:
# 0.18.4 -> 0.18.4
# 0.18.4-137-gXXX -> 0.18.4.137
# 0.18.4-rcX-137-gXXX -> 0.18.4.137
# And use it as version that can be parsed by the cmake VERSION

# Strip the -rcX and -SHAPSHOT version specifies
string(REGEX REPLACE "(-rc[0-9]+|-SNAPSHOT)" "" VCS_VERSION ${VCS_DESCRIBE})

# Replace the -N-gXXX-dirty with .N for the patchlevel
string(REGEX REPLACE "-([0-9]*)-g[0-9a-z]*(-dirty)?" ".\\2" VCS_VERSION ${VCS_VERSION})
string(REGEX REPLACE "-dirty" "" VCS_VERSION ${VCS_VERSION})

if (NOT VCS_VERSION MATCHES "^[0-9]+\.[0-9]+\.[0-9]+(\.[0-9]+)?$")
	message("VCS_DESCRIBE ${VCS_DESCRIBE}")
	message("VCS_VERSION  ${VCS_VERSION}")
	message(FATAL_ERROR "Unable to parse version description into a version in major.minor.patch.tweak format. Is the latest tag ill-formed?")
endif()
