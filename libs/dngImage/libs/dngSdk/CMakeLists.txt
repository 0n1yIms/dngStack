cmake_minimum_required(VERSION 3.6.0)

file (GLOB dngSdk_SRCS 
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file (GLOB dngSdk_HDRS 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(dngSdk STATIC ${dngSdk_SRCS} ${dngSdk_HDRS})
target_include_directories(dngSdk PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

target_link_libraries(dngSdk zlib)

target_compile_options(dngSdk PUBLIC
        -DUNIX_ENV=1
        -DqDNGBigEndian=0
        -DqDNGThreadSafe=0
        -DqDNGUseLibJPEG=0
        -DqDNGUseXMP=0
        -DqDNGValidateTarget=1
        -DqAndroid=0
        )

#install 
# install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include/dngSdk)
# install(TARGETS dngSdk 
        # ARCHIVE DESTINATION lib
        # LIBRARY DESTINATION lib)


        