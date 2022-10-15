include(FetchContent)

FetchContent_Declare(
    salticidae
    GIT_REPOSITORY https://github.com/Determinant/salticidae.git
    GIT_TAG        v0.3.1
)
FetchContent_GetProperties(salticidae)
if(NOT salticidae_POPULATED)
    FetchContent_Populate(salticidae)
endif()



include_directories(${salticidae_SOURCE_DIR}/include )
message("include path:${salticidae_SOURCE_DIR}/include")
file(GLOB SALIBS "${salticidae_SOURCE_DIR}/src/*.cpp")
add_library(network_lib ${SALIBS})
