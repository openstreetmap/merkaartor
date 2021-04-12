# Minimum required version is 3.12.0 for the list(TRANSFORM), and 3.6.0 for list(FILTER)
cmake_minimum_required(VERSION 3.12.0)

# This file is processed by git archive and the template is filled in. It is
# always safe to include. The variable VCS_SNAPSHOT is set to 1 when the
# template was filled in, to 0 otherwise.

# If this file is exported with git, we set the snapshot to 1 and define some values to the variables.
if ("$Format:YES$" STREQUAL "YES")
  set(VCS_SNAPSHOT 1)
  set(VCS_BRANCH "SNAPSHOT")
  set(VCS_DIRTY 0)
  string(SUBSTRING "$Format:%H$" 0 12 VCS_COMMIT)
  set(VCS_DESCRIBE "0.0.0-SNAPSHOT-1-g$Format:%h$")

  # Here, we do some magic to extract tags on this commit. The %D will result in a string in form:
  # HEAD -> master, tag: mytag1, mytag2
  # To process it, we split it by ", ", filter only elements starting with "tag:" and replace the "tag: " out.
  # The remaining variable VCS_TAGS is a CMAKE list of exactly the tags, and can be conveniently test if some tags are found on the current commit.
  # Note: this does not help us in generating something like git describe --tags, as I see no way of extracting the last ancestor tag.
  set(VCS_REFS "$Format:%D$")
  string(REPLACE ", " ";" VCS_TAGS ${VCS_REFS})
  list(FILTER VCS_TAGS INCLUDE REGEX "^tag: ")
  list(TRANSFORM VCS_TAGS REPLACE "^tag: " "")
  if (VCS_TAGS)
      # Pop first tag into VCS_DESCRIBE if any tags are present.
      list(POP_FRONT VCS_TAGS VCS_DESCRIBE)
  endif()
else()
  set(VCS_SNAPSHOT 0)
endif()
