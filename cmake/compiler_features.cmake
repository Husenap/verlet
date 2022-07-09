if(NOT TARGET compiler_features)
    add_library(compiler_features INTERFACE)

    target_compile_features(compiler_features INTERFACE cxx_std_20)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
endif()