set(nlohmann_json_CXXFLAGS "-D_FORTIFY_SOURCE=2 -O2")
set(nlohmann_json_CFLAGS "-D_FORTIFY_SOURCE=2 -O2")
add_pkg(nlohmann_json
        VER 3.6.1
        HEAD_ONLY ./
        URL https://github.com/nlohmann/json/releases/download/v3.6.1/include.zip
        MD5 d41d8cd98f00b204e9800998ecf8427e)
include_directories(${nlohmann_json_INC})
message("include path:${nlohmann_json_INC}")
add_library(json ALIAS nlohmann_json)