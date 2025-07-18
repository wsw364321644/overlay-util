
NewTargetSource()
AddSourceFolder(RECURSE PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/public")
AddSourceFolder(RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/private")
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SourceFiles})

function(add_simple_hook_helper TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES C_STANDARD 23)
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "overlay")

    if(WIN32)
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:LIBCMT.LIB /NODEFAULTLIB:msvcrt")
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE "/NODEFAULTLIB:LIBCMTD.LIB /NODEFAULTLIB:msvcrtd")
    endif()

    set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_23)

    AddTargetInclude(${TARGET_NAME})
    AddTargetInstall(${TARGET_NAME} UTILPP)
endfunction()

set(TARGET_NAME simple_hook_helper)
add_library(${TARGET_NAME} SHARED ${SourceFiles})
add_simple_hook_helper(${TARGET_NAME})
target_compile_definitions(${TARGET_NAME} PRIVATE -DHOOK_HELPER_EXPORTS)
target_link_libraries(${TARGET_NAME} PRIVATE UTILPP::ipc_util)
target_link_libraries(${TARGET_NAME} PRIVATE UTILPP::crypto_lib_helper)
target_link_libraries(${TARGET_NAME} PRIVATE windows_util)
target_link_libraries(${TARGET_NAME} PUBLIC SDL2::SDL2)
target_link_libraries(${TARGET_NAME} PRIVATE simdjson::simdjson)


set(TARGET_NAME simple_hook_helper_a)
add_library(${TARGET_NAME} STATIC ${SourceFiles})
add_simple_hook_helper(${TARGET_NAME})
target_compile_definitions(${TARGET_NAME} PUBLIC -DHOOK_HELPER_NODLL)
target_link_libraries(${TARGET_NAME} PRIVATE UTILPP::ipc_util_a)
target_link_libraries(${TARGET_NAME} PRIVATE UTILPP::crypto_lib_helper_a)
target_link_libraries(${TARGET_NAME} PRIVATE windows_util_a)
target_link_libraries(${TARGET_NAME} PUBLIC SDL2::SDL2-static)
target_link_libraries(${TARGET_NAME} PRIVATE simdjson::simdjson_static)