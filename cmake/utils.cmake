include(FetchContent)

function(add_pkg pkg_name )
    set(options )
    set(oneValueArgs URL MD5 GIT_REPOSITORY GIT_TAG VER EXE DIR HEAD_ONLY)
    set(multiValueArgs CMAKE_OPTION)
    cmake_parse_arguments(PKG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    if(NOT PKG_EXE)
        set(PKG_EXE 0)
    endif()
    if (NOT PKG_LIB_PATH)
        set(PKG_LIB_PATH lib)
    endif ()
    if (PKG_GIT_REPOSITORY)
        __download_pkg_with_git(${pkg_name} ${PKG_GIT_REPOSITORY} ${PKG_GIT_TAG} ${PKG_MD5})
    else()
        __download_pkg(${pkg_name} ${PKG_URL} ${PKG_MD5})
    endif()
    fetchcontent_makeavailable(${pkg_name})
    foreach(_SUBMODULE_FILE ${PKG_SUBMODULES})
        STRING( REGEX REPLACE "(.+)_(.+)" "\\1" _SUBMODEPATH ${_SUBMODULE_FILE})
        STRING( REGEX REPLACE "(.+)/(.+)" "\\2" _SUBMODENAME ${_SUBMODEPATH})
        file(GLOB ${pkg_name}_INSTALL_SUBMODULE ${_SUBMODULE_FILE}/*)
        file(COPY ${${pkg_name}_INSTALL_SUBMODULE} DESTINATION ${${pkg_name}_SOURCE_DIR}/3rdparty/${_SUBMODENAME})
    endforeach (_SUBMODULE_FILE)
    file(WRITE ${${pkg_name}_SOURCE_DIR}/options.txt ${${pkg_name}_CONFIG_TXT})

    if(EXISTS ${${pkg_name}_SOURCE_DIR}/options.txt AND PKG_HEAD_ONLY)
        set(${pkg_name}_INC ${${pkg_name}_SOURCE_DIR}/${PKG_HEAD_ONLY} PARENT_SCOPE)
        add_library(${pkg_name} INTERFACE)
        target_include_directories(${pkg_name} INTERFACE ${${pkg_name}_INC})
        if (${PKG_RELEASE})
            __find_pkg_then_add_target(${pkg_name} ${PKG_EXE} ${PKG_LIB_PATH} ${PKG_LIBS})
        endif ()
        return()
    endif ()
    find_package(${pkg_name} ${PKG_VER} REQUIRED)
    if (${pkg_name}_FOUND)
        set(${pkg_name}_INC ${${pkg_name}_SOURCE_DIR}/include PARENT_SCOPE)
        message("Found pkg: ${${pkg_name}_LIBRARIES}")
        return()
    endif ()
endfunction()


function(__download_pkg_with_git pkg_name pkg_url pkg_git_commit)
	FetchContent_Declare(
            ${pkg_name}
	    GIT_REPOSITORY      ${pkg_url}
	    GIT_TAG             ${pkg_git_commit})
    FetchContent_GetProperties(${pkg_name})
    message("download: ${${pkg_name}_SOURCE_DIR} , ${pkg_name} , ${pkg_url}")
    if(NOT ${pkg_name}_POPULATED)
        FetchContent_Populate(${pkg_name})
        set(${pkg_name}_SOURCE_DIR ${${pkg_name}_SOURCE_DIR} PARENT_SCOPE)
    endif()

endfunction()

function(__download_pkg pkg_name pkg_url pkg_md5)
    FetchContent_Declare(
            ${pkg_name}
            URL      ${pkg_url}
            URL_HASH MD5=${pkg_md5}
    )
    FetchContent_GetProperties(${pkg_name})
    message("download: ${${pkg_name}_SOURCE_DIR} , ${pkg_name} , ${pkg_url}")
    if(NOT ${pkg_name}_POPULATED)
        FetchContent_Populate(${pkg_name})
        set(${pkg_name}_SOURCE_DIR ${${pkg_name}_SOURCE_DIR} PARENT_SCOPE)
    endif()

endfunction()

#                                                0     lib
function(__find_pkg_then_add_target pkg_name lib_path)

    unset(${pkg_name}_LIBS)

    message("_FIND:${${pkg_name}_BASE_DIR}")


    foreach(_LIB_NAME ${ARGN})
        set(_LIB_SEARCH_NAME ${_LIB_NAME})
        set(_LIB_TYPE SHARED)
        if (${pkg_name}_USE_STATIC_LIBS)
            set(_LIB_SEARCH_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}${_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
            set(_LIB_TYPE STATIC)
        endif ()
        set(${_LIB_NAME}_LIB ${_LIB_NAME}_LIB-NOTFOUND)
        find_library(${_LIB_NAME}_LIB ${_LIB_SEARCH_NAME} PATHS ${${pkg_name}_BASE_DIR}/${lib_path} NO_DEFAULT_PATH)

        if(NOT ${_LIB_NAME}_LIB)
            return()
        endif()

        add_library(${pkg_name}::${_LIB_NAME} ${_LIB_TYPE} IMPORTED GLOBAL)
        if (WIN32 AND ${_LIB_TYPE} STREQUAL "SHARED")
            set_target_properties(${pkg_name}::${_LIB_NAME} PROPERTIES IMPORTED_IMPLIB_RELEASE ${${_LIB_NAME}_LIB})
        else()
            set_target_properties(${pkg_name}::${_LIB_NAME} PROPERTIES IMPORTED_LOCATION ${${_LIB_NAME}_LIB})
        endif()

        if (EXISTS ${${pkg_name}_BASE_DIR}/include)
            set_target_properties(${pkg_name}::${_LIB_NAME} PROPERTIES 
                INTERFACE_INCLUDE_DIRECTORIES "${${pkg_name}_BASE_DIR}/include")
        endif ()

        list(APPEND ${pkg_name}_LIBS ${pkg_name}::${_LIB_NAME})
        message("found ${${_LIB_NAME}_LIB}")
        STRING( REGEX REPLACE "(.+)/(.+)" "\\1" LIBPATH ${${_LIB_NAME}_LIB})
        set(${pkg_name}_LIBPATH ${LIBPATH} CACHE STRING INTERNAL)
    endforeach(_LIB_NAME)

    set(${pkg_name}_LIBS ${${pkg_name}_LIBS} PARENT_SCOPE)
endfunction()