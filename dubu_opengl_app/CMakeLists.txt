set(target_name "dubu_opengl_app")

set(src_precompiled
    "src/dubu_opengl_app/precompiled.hpp")

set(src_dubu_opengl_app
    "src/dubu_opengl_app/AppBase.cpp"
    "src/dubu_opengl_app/AppBase.hpp"
    "src/dubu_opengl_app/dubu_opengl_app.hpp")

set(src_files
    ${src_precompiled}
    ${src_dubu_opengl_app})

# Project
add_library(${target_name} STATIC ${src_files})

target_link_libraries(${target_name}
    dubu_log
    dubu_window
    glm
    glad
    imgui
    compiler_features
    compiler_warnings)

target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_precompile_headers(${target_name} PUBLIC ${src_precompiled})

source_group("src" FILES ${src_dubu_opengl_app})
source_group("src/precompiled" FILES ${src_precompiled})
source_group("src/precompiled" REGULAR_EXPRESSION "cmake_pch\.[ch]xx")

set_target_properties(${target_name} PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
