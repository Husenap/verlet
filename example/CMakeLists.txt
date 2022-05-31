set(target_name "dubu_opengl_app_example")

set(src_precompiled
    "src/precompiled.hpp")

set(src_example
    "src/solver.hpp"
    "src/main.cpp")

set(src_files
    ${src_precompiled}
    ${src_example})

# Project
add_executable(${target_name} ${src_files})

target_link_libraries(${target_name}
    dubu_opengl_app
    compiler_features
    compiler_warnings)

target_precompile_headers(${target_name} PUBLIC ${src_precompiled})

source_group("src" FILES ${src_example})
source_group("src/precompiled" FILES ${src_precompiled})
source_group("src/precompiled" REGULAR_EXPRESSION "cmake_pch\.[ch]xx")

set_target_properties(${target_name} PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER}/test)
