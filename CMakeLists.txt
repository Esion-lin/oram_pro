cmake_minimum_required(VERSION 3.12)
project(ram_mpc_dt)
SET(CMAKE_BUILD_TYPE "Debug")  
set(CMAKE_CXX_STANDARD 14)
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g2 -ggdb -g -O2")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -g2 -ggdb -fsanitize=address -fno-omit-frame-pointer -g -O2")   
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")  
SET(CMAKE_EXE_LINKER_FLAGS 
          "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,/usr/local/lib")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/salticidae/cmake/Modules/")
set(NTL_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
add_definitions(-DNEED_MUL -DNEED_CMOV -DTotal_bitwise -w)

IF (CMAKE_BUILD_TYPE STREQUAL Debug)        
    add_definitions(-DDEBUG) 
ENDIF()

option(ENABLE_FFS "Enable FFS" OFF)
option(ENABLE_GC "Enable GC" OFF)
option(ENABLE_DT "Enable DT" OFF)
option(TEST_FOREST_DT "test forest dt protocol" OFF)
option(TEST_RISC "test risc protocol" OFF)
option(TEST_GCM "test GCM protocol" OFF)

option(TEST_AUDI "test auditable protocol" OFF)

option(TEST_PPML "test PPML protocol" ON)

find_package(OpenSSL REQUIRED)
find_path(NTL_INCLUDE_DIR
              NAMES NTL/ZZ.h
              HINTS ENV NTL_INC_DIR
                    ENV NTL_DIR
              PATH_SUFFIXES include
              DOC "The directory containing the NTL include files"
             )

find_library(NTL_LIBRARY
                NAMES ntl
                HINTS ENV NTL_LIB_DIR
                    ENV NTL_DIR
                PATH_SUFFIXES lib
                DOC "Path to the NTL library"
            )
find_path(GMP_INCLUDE_DIR NAMES gmp.h)
find_library(GMP_LIBRARIES NAMES gmp libgmp)
find_library(GMPXX_LIBRARIES NAMES gmpxx libgmpxx)
find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
if( OPENSSL_FOUND )
    include_directories(${OPENSSL_INCLUDE_DIRS})
    message("Using OpenSSL ${OPENSSL_VERSION}")
else()
    message("can't find Openssl")
endif()

include(cmake/utils.cmake)
include(cmake/log.cmake)
include(cmake/json.cmake)
if (ENABLE_GC)
    include(cmake/emp.cmake)
    include(cmake/empot.cmake)
endif()
#include(cmake/salticidae.cmake)
include_directories(include)

if(TEST_RISC)
    include_directories(protocol)
    include_directories(protocol/3_parties)
    include_directories(protocol/2_parties)
    include_directories(encoding)
    add_library(encoder encoding/encoder.cc)
    message("enable risc") 
    add_library(risc protocol/src/risc.cpp
                    protocol/src/convert.cpp)
    target_link_libraries(risc fss gc)
    # add_executable(test src/Test_ram.cpp)
    add_executable(test_prf src/Test_prf.cpp)
    add_executable(ORAM_TEST src/oram_rw_test.cpp)
    add_executable(test_risc src/Test_risc.cpp)
    add_executable(test_reveal src/Test_reveal.cpp)
    target_link_libraries(ORAM_TEST PRIVATE risc log fss_mpc)
    target_link_libraries(test_risc PRIVATE risc log)
    target_link_libraries(test_prf PRIVATE fss gc log fss_mpc)
    target_link_libraries(test_reveal PRIVATE  log )
endif()

#add_subdirectory(salticidae)
#include_directories(salticidae/include)



if (ENABLE_FFS)
    message("enable ffs") 
    include_directories(fss/include)
    add_library(fss fss/fss-client.cpp
                    fss/fss-common.cpp
                    fss/fss-server.cpp)
    add_library(fss_mpc include/fss_mpc.cpp)
    target_link_libraries(fss gmp gmpxx ${OPENSSL_LIBRARIES})
    add_executable(test_fss src/Test_ram.cpp)
    target_link_libraries(test_fss PRIVATE fss)
endif()

if (ENABLE_GC)
    message("enable gc") 
    add_library(gc protocol/src/gc_op.cpp)
    target_link_libraries(gc ${OPENSSL_LIBRARIES} emp-tool emp empot)
    add_executable(test_gc src/Test_gc.cpp)
    target_link_libraries(test_gc PRIVATE gc log emp-tool emp empot)   
endif()
if (TEST_GCM)
    add_library(gcm protocol/2_parties/GCM.cpp)
    add_executable(test_gcm src/test_gcm.cpp)
    target_link_libraries(test_gcm PRIVATE gcm gc log emp-tool emp empot )
endif()


    

if (TEST_FOREST_DT)
    message("enable dt") 
    add_executable(test_ic src/test_ic.cpp)
    add_executable(test_csot src/Test_csot.cpp)
    # add_executable(test_dt_con src/Test_3_dt_const.cpp)
    add_executable(test_dt src/dt_test.cpp)
    add_executable(test_dt_64 src/dt_test_64.cpp)
    target_link_libraries(test_ic PRIVATE fss log)
    target_link_libraries(test_csot PRIVATE fss log)
    target_link_libraries(test_dt PRIVATE fss log )
    target_link_libraries(test_dt_64 PRIVATE fss log )
endif()

if(TEST_AUDI)
    message("enable audi") 
    include_directories(protocol/auditable)
    include_directories(protocol/mparties)
    #add_library(bmr protocol/mparties/BMR.cpp)
    add_library(audit protocol/mparties/BMR.cpp protocol/auditable/auditable.cc)
    add_executable(test_audit protocol/auditable/testAB.cpp)
    add_executable(test_audit2 src/test_auditable.cpp)
    target_link_libraries(test_audit PUBLIC gmp gmpxx ${OPENSSL_LIBRARIES} log encoder  audit)
    target_link_libraries(test_audit2 PUBLIC gmp gmpxx ${OPENSSL_LIBRARIES} log encoder  audit)
endif()

#add_executable(test_asyn src/test_asyn_net.cpp)
#target_link_libraries(test_asyn PUBLIC salticidae_static)
if(TEST_PPML)
    message("TEST PPML")
    include_directories(protocol/malicious_ppml)
    add_library(ppml protocol/malicious_ppml/semi.cc)
    #add_executable(test_ppml src/test_malicious_ppml.cpp)
    add_executable(test_mul_ver src/test_mul_ver.cpp)
    add_executable(test_mul_ver2 src/test_mul_ver2.cpp)
    #target_link_libraries(test_ppml PUBLIC ntl ${OPENSSL_LIBRARIES} ppml)
    target_link_libraries(test_mul_ver PUBLIC ntl ${OPENSSL_LIBRARIES} ppml)
    target_link_libraries(test_mul_ver2 PUBLIC ntl ${OPENSSL_LIBRARIES} ppml)
endif()

# 



# add_executable(test_thr src/Test_3_parties.cpp)

# target_link_libraries(test PRIVATE risc log)



# target_link_libraries(test_thr PRIVATE fss log)

# target_link_libraries(test_dt_con PRIVATE fss log )
