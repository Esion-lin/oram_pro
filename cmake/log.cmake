add_pkg(easyloggingpp
        GIT_TAG        v9.97.0
        HEAD_ONLY ./
        GIT_REPOSITORY https://github.com/amrayn/easyloggingpp.git
        MD5 0dc903888211db3a0f170304cd9f3a89)
include_directories(${easyloggingpp_INC}src/)
message("include path:${easyloggingpp_INC}src")
add_library(log ${easyloggingpp_INC}src/easylogging++.cc)

