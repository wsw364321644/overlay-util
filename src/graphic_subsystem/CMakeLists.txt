
list(APPEND PublicFiles
    ${CMAKE_CURRENT_SOURCE_DIR}/public/Graphic/GraphicSubsystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/public/graphic_subsystem_exports.h
)
list(APPEND SourceFiles
    ${CMAKE_CURRENT_SOURCE_DIR}/private/Graphic/GraphicSubsystem.cpp
)

find_package(OpenGL)
find_package(GLUT)
find_package(GLFW)

if(OPENGL_FOUND)
    list(APPEND SourceFiles
        ${CMAKE_CURRENT_SOURCE_DIR}/private/Graphic/GraphicSubsystemOpenGL.h
        ${CMAKE_CURRENT_SOURCE_DIR}/private/Graphic/GraphicSubsystemOpenGL.cpp
    )

    if(GLUT_FOUND)
    endif()

    if(GLFW_FOUND)
    endif()
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    list(APPEND PublicFiles
        ${CMAKE_CURRENT_SOURCE_DIR}/public/Graphic/GraphicSubsystemDXGI.h
    )
    list(APPEND SourceFiles
        ${CMAKE_CURRENT_SOURCE_DIR}/private/Graphic/GraphicSubsystemDXGI.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/private/Graphic/GraphicSubsystemDX9.h
        ${CMAKE_CURRENT_SOURCE_DIR}/private/Graphic/GraphicSubsystemDX9.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/private/Graphic/GraphicSubsystemDX11.h
        ${CMAKE_CURRENT_SOURCE_DIR}/private/Graphic/GraphicSubsystemDX11.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/private/Graphic/GraphicSubsystemDX12.h
        ${CMAKE_CURRENT_SOURCE_DIR}/private/Graphic/GraphicSubsystemDX12.cpp
    )
endif()

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SourceFiles})

function(add_lib TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES C_STANDARD 23)
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "overlay")

    if(WIN32)
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:LIBCMT.LIB /NODEFAULTLIB:msvcrt")
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE "/NODEFAULTLIB:LIBCMTD.LIB /NODEFAULTLIB:msvcrtd")
    endif()

    set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_23)

    if(CMAKE_SYSTEM_NAME MATCHES "Windows")
        target_link_libraries(${TARGET_NAME} PRIVATE dxgi d3d9 d3d11 d3d12)
    endif()

    if(OPENGL_FOUND)
        target_link_libraries(${TARGET_NAME} PRIVATE OpenGL::GL)

        if(GLUT_FOUND)
            target_link_libraries(${TARGET_NAME} PRIVATE GLUT::GLUT)
        endif()

        if(GLFW_FOUND)
            target_link_libraries(${TARGET_NAME} PRIVATE GLFW::GLFW)
        endif()
    endif()

    target_link_libraries(${TARGET_NAME} PRIVATE simple_utilpp_a)
    target_link_libraries(${TARGET_NAME} PRIVATE simple_logger_a)

    target_include_directories(${TARGET_NAME}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private
    )
    target_sources(${TARGET_NAME}
        PRIVATE
        ${SourceFiles}
    )

    foreach(PublicFile ${PublicFiles})
        cmake_path(RELATIVE_PATH PublicFile
            BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/public
            OUTPUT_VARIABLE PublicFileRelativePath)
        cmake_path(APPEND TARGET_NAME CMAKE_INSTALL_INCLUDEDIR InterfaceIncludeFileRelativeDir OUTPUT_VARIABLE InterfaceIncludeFileInstallDir])
        target_sources(${TARGET_NAME}
            PUBLIC
            FILE_SET HEADERS
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/public
            FILES
            $<BUILD_INTERFACE:${PublicFile}>
            $<INSTALL_INTERFACE:${InterfaceIncludeFileInstallDir}>
        )
    endforeach(PublicFile ${PublicFiles})

    AddTargetInstall(${TARGET_NAME} UTILPP)
endfunction()

set(TARGET_NAME graphic_subsystem)
add_library(${TARGET_NAME} SHARED)
add_lib(${TARGET_NAME})
target_compile_definitions(${TARGET_NAME} PRIVATE -DGRAPHIC_SUBSYSTEM_EXPORTS)

set(TARGET_NAME graphic_subsystem_a)
add_library(${TARGET_NAME} STATIC)
add_lib(${TARGET_NAME})
target_compile_definitions(${TARGET_NAME} PUBLIC -DGRAPHIC_SUBSYSTEM_NODLL)
