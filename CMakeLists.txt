cmake_minimum_required(VERSION 3.0.0)
project(inkbox-audio VERSION 0.1.0)

include(CTest)
enable_testing()

# https://stackoverflow.com/questions/1662909/undefined-reference-to-pthread-create-in-linux
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lasound")

include_directories(${CMAKE_SOURCE_DIR}/lib/boost_1_79_0/)
include_directories(${CMAKE_SOURCE_DIR}/lib/libsndfile/include/)

# https://stackoverflow.com/questions/3201154/automatically-add-all-files-in-a-folder-to-a-target-using-cmake
file(GLOB fileSrc
     "src/*.h"
     "src/*.cpp"
     "src/*.c"
     # subdirs
     "src/*/*.h"
     "src/*/*.cpp"
)

add_executable(inkaudio ${fileSrc})

# https://stackoverflow.com/questions/33149878/experimentalfilesystem-linker-error
target_link_libraries(inkaudio libsndfile.so)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
