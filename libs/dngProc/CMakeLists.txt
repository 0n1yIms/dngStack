cmake_minimum_required(VERSION 3.6.0)


add_library(dngProc STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/dngProc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dngProcGPU.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dngProc.h
        ${CMAKE_CURRENT_SOURCE_DIR}/DngTemp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DngTemp.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ColorCore.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ColorCorrect.h
        ${CMAKE_CURRENT_SOURCE_DIR}/matrix_vector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/matrix_vector.h
        )
target_include_directories(dngProc PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )
        
target_link_libraries(dngProc
        dngImage
        # Halide::Halide
        )