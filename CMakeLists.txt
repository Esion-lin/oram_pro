cmake_minimum_required(VERSION 3.12)
project(ram_mpc_dt)
SET(CMAKE_BUILD_TYPE "Debug")  
set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -g2 -ggdb -g -O2")
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -g2 -ggdb -fsanitize=address -fno-omit-frame-pointer -g -O2")   
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")  
SET(CMAKE_EXE_LINKER_FLAGS 
          "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,/usr/local/lib")

find_package(OpenSSL REQUIRED)
find_path(GMP_INCLUDE_DIR NAMES gmp.h)
find_library(GMP_LIBRARIES NAMES gmp libgmp)
find_library(GMPXX_LIBRARIES NAMES gmpxx libgmpxx)

if( OPENSSL_FOUND )
    include_directories(${OPENSSL_INCLUDE_DIRS})
    message("Using OpenSSL ${OPENSSL_VERSION}")
else()
    message("can't find Openssl")
endif()

include(cmake/utils.cmake)
include(cmake/log.cmake)
include(cmake/json.cmake)

include_directories(include)
include_directories(protocol/include)
include_directories(fss/include)
add_library(fss fss/fss-client.cpp
                fss/fss-common.cpp
                fss/fss-server.cpp)
target_link_libraries(fss gmp gmpxx ${OPENSSL_LIBRARIES})

add_executable(oshift src/applications/ot_and_shift.cpp)
add_executable(fss_test src/applications/fss_test.cpp)
target_link_libraries(oshift PRIVATE ${OPENSSL_LIBRARIES} log nlohmann_json)
target_link_libraries(fss_test PRIVATE gmp gmpxx fss log nlohmann_json)