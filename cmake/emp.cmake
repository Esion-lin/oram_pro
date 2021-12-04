add_pkg(emp
        GIT_TAG        0.2.2
        HEAD_ONLY ./
        GIT_REPOSITORY https://github.com/emp-toolkit/emp-tool.git)

        
include_directories(${emp_INC})
message("........ ${emp_INC}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -Wall -funroll-loops -Wno-ignored-attributes -Wno-unused-result")
message("${Blue}-- Platform: ${CMAKE_SYSTEM_PROCESSOR}${ColourReset}")
IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "(aarch64)|(arm64)")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a+simd+crypto+crc")
ELSE(${CMAKE_SYSTEM_PROCESSOR} MATCHES "(aarch64)|(arm64)")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -maes -mrdseed")
ENDIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "(aarch64)|(arm64)" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11")

execute_process(
  OUTPUT_FILE ${emp_INC}/emp-tool/circuits/files/bristol_fashion/Keccak_f.txt.cpp
  COMMAND xxd -i ${emp_INC}/emp-tool/circuits/files/bristol_fashion/Keccak_f.txt ${emp_INC}/emp-tool/circuits/files/bristol_fashion/Keccak_f.txt.hex
  COMMAND echo "\\#include \\\"emp-tool/circuits/sha3_256.h\\\"" 
  COMMAND cat ${emp_INC}/emp-tool/circuits/files/bristol_fashion/Keccak_f.txt.hex 
  WORKING_DIRECTORY ${emp_INC}/
  )

add_subdirectory(${emp_INC})