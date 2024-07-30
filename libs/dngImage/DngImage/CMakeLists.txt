cmake_minimum_required(VERSION 3.6.0)
project(DngImage)

if(BUILD_SHARED_LIBS)
    set(LIBS_TYPE SHARED)
else()
    set(LIBS_TYPE STATIC)
endif()

add_library(dngImage ${LIBS_TYPE}
        ${CMAKE_CURRENT_SOURCE_DIR}/dngImage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dngImage.hpp
        )
target_include_directories(dngImage PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        )
target_link_libraries(dngImage dngSdk)

# install 
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dngImage.hpp
        DESTINATION include/DngImage
)

install(TARGETS dngImage
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        )