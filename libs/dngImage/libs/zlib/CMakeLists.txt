cmake_minimum_required(VERSION 3.6.0)


file (GLOB zlib_HDRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file (GLOB zlib_SRCS  
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_library(zlib STATIC ${zlib_SRCS} ${zlib_HDRS})

target_include_directories(zlib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

# install
# install(TARGETS zlib
        # ARCHIVE DESTINATION lib
        # LIBRARY DESTINATION lib
        # RUNTIME DESTINATION bin
        # )
# install(FILES ${zlib_HDRS} DESTINATION include/zlib)
