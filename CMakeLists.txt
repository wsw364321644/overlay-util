cmake_minimum_required(VERSION 3.24)
include(FetchContent)
include(CMakePackageConfigHelpers)

set(MY_PRODUCT_NUMBER 1)
set(MY_PRODUCT_VERSION 0)
set(MY_BUILD_NUMBER 0)
set(VERSION "${MY_PRODUCT_NUMBER}.${MY_PRODUCT_VERSION}.${MY_BUILD_NUMBER}")

set(OCmakeUtil_TAG 9344823811211257ade55a23e010a3b09820dfa7)
set(UTILPP_TAG 4f273909e85baee1170cd55d2057586d6186abb2)

project(OverlayUtil VERSION ${VERSION})
enable_language(CXX ASM)

option(OVERLAY_STATIC_CRT "OVERLAY STATIC CRT Build ." OFF)
option(OVERLAY_GIT_SSH "Git port 22." OFF)

if(OVERLAY_GIT_SSH)
  set(SSH SSH)
else()
  unset(SSH)
endif()

if(OVERLAY_STATIC_CRT)
  if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  endif()

  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
  set(STATIC_CRT STATIC_CRT)
else()
  unset(STATIC_CRT)
endif()

FetchContent_Declare(
  OCmakeUtil
  GIT_REPOSITORY https://github.com/wsw364321644/OCmakeUtil.git
  GIT_TAG ${OCmakeUtil_TAG}
  GIT_SUBMODULES_RECURSE true
)
FetchContent_MakeAvailable(OCmakeUtil)

ImportProject(simdjson STATIC_CRT ${SSH} TAG v4.2.4)
ImportProject(libuv STATIC_CRT ${SSH} TAG v1.51.0)
ImportProject(SDL2 STATIC_CRT ${SSH} FIND TAG release-2.32.10)
ImportProject(MbedTLS STATIC STATIC_CRT ${SSH} FIND TAG mbedtls-3.6.5)

FetchContent_Declare(
  utilpp
  GIT_REPOSITORY https://github.com/wsw364321644/utilpp.git
  GIT_TAG ${UTILPP_TAG}
)
FetchContent_MakeAvailable(utilpp)

add_subdirectory(src/simple_hook_helper)
add_subdirectory(src/graphic_subsystem)

ExportFromInstall(${PROJECT_NAME})
