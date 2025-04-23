cmake_minimum_required(VERSION 3.24)
include(FetchContent)
include(CMakePackageConfigHelpers)

set(MY_PRODUCT_NUMBER 1)
set(MY_PRODUCT_VERSION 0)
set(MY_BUILD_NUMBER 0)
set(VERSION "${MY_PRODUCT_NUMBER}.${MY_PRODUCT_VERSION}.${MY_BUILD_NUMBER}")

set(OCmakeUtil_TAG 3c4a24dce08efba73a57bea8efadaf953865c2d9)
set(UTILPP_TAG 5c1db5b1bbe578d0110703de6007454c00938bb3)

project(overlay-util VERSION ${VERSION})
enable_language(CXX ASM)

option(OVERLAY_STATIC_CRT "OVERLAY STATIC CRT Build ." OFF)

if(PROJECT_IS_TOP_LEVEL)
  if(OVERLAY_STATIC_CRT)
    if(MSVC)
      set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
      add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    endif()

    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
  endif()
endif()

FetchContent_Declare(
  OCmakeUtil
  GIT_REPOSITORY git@ssh.github.com:wsw364321644/OCmakeUtil.git
  GIT_TAG ${OCmakeUtil_TAG}
  GIT_SUBMODULES_RECURSE true
)
FetchContent_MakeAvailable(OCmakeUtil)

ImportProject(LIBUV STATIC_CRT SSH FIND TAG be6b81a352d17513c95be153afcb3148f1a451cd) # 1.47.0
ImportProject(SDL2 STATIC_CRT SSH FIND TAG f461d91cd265d7b9a44b4d472b1df0c0ad2855a0) # 2.30.2
ImportProject(MbedTLS STATIC STATIC_CRT SSH FIND TAG 2ca6c285a0dd3f33982dd57299012dacab1ff206) # 3.6.0

FetchContent_Declare(
  utilpp
  GIT_REPOSITORY git@ssh.github.com:wsw364321644/utilpp.git
  GIT_TAG ${UTILPP_TAG}
)
FetchContent_MakeAvailable(utilpp)

add_subdirectory(src/simple_hook_helper)
add_subdirectory(src/graphic_subsystem)