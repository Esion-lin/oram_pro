add_pkg(empot
        GIT_TAG        0.2.2
        HEAD_ONLY ./
        GIT_REPOSITORY https://github.com/emp-toolkit/emp-ot.git)
include_directories(${empot_INC})
message("include path:${empot_INC}")