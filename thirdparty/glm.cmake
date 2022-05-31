message("-- External Project: glm")
include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         761a842a595f58acfc01cc5c7760ff351feaaece
)

FetchContent_MakeAvailable(glm)

if(TARGET glm_shared)
    set_target_properties(glm_shared PROPERTIES FOLDER "thirdparty/glm")
endif()