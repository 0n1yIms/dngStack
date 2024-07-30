cmake_minimum_required(VERSION 3.6.0)
PROJECT("Polar")

set(CMAKE_CXX_STANDARD 17)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()


# libs
add_subdirectory(libs/pngImage/libs/zlib)
add_subdirectory(libs/pngImage/libs/libpng)
add_subdirectory(libs/pngImage/PngImage)


add_subdirectory(libs/dngImage/libs/dngSdk)
add_subdirectory(libs/dngImage/DngImage)

# src

add_executable(main main.cpp)
target_link_libraries(main
        dngImage
        pngImage
        )


# install

# executables
install(TARGETS main
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION bin
        ARCHIVE DESTINATION lib
        )



