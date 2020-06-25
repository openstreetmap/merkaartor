find_package(Git)

if (NOT ${GIT_FOUND})
    message(FATAL_ERROR "Git not found. Cannot proceed with version info.")
endif()

if (NOT EXISTS "${PROJECT_SOURCE_DIR}/.git")
    message(FATAL_ERROR "VCS directory does not contain a .git subdirectory. Not a repository?")
endif()

# Set dependencies on git index to reconfigure on changed index/branch (like switching branches, commiting, etc.).
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/.git/index")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/.git/HEAD")

execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE  VCS_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short=12 HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE  VCS_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --dirty
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE  VCS_DESCRIBE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

