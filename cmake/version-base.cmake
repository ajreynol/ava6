# These are updated when making a release
set(AVA6_LAST_RELEASE "1.4.0")
set(AVA6_IS_RELEASE "false")

# These are used in other places in cmake
# If possible, they are updated by version.cmake
set(GIT_BUILD "false")
set(AVA6_VERSION "${AVA6_LAST_RELEASE}")
set(AVA6_VERSION_NUMBER "${AVA6_LAST_RELEASE}")
set(AVA6_FULL_VERSION "${AVA6_LAST_RELEASE}")
set(AVA6_WHEEL_VERSION "${AVA6_LAST_RELEASE}")
set(AVA6_MAVEN_VERSION "${AVA6_LAST_RELEASE}")
set(AVA6_GIT_INFO "")

# Shared library versioning: extract and use the major version.
# This assumes the ava6 major version is incremented whenever
# API/ABI compatibility is broken.
string(REGEX MATCH "^[0-9]+" AVA6_SOVERSION "${AVA6_LAST_RELEASE}")
