cmake_minimum_required(VERSION 3.0.0)
project(inkbox-audio VERSION 0.1.0)

include(CTest)
enable_testing()

# https://stackoverflow.com/questions/1662909/undefined-reference-to-pthread-create-in-linux
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lasound")

include_directories(${CMAKE_SOURCE_DIR}/lib/boost_1_79_0/)

# https://stackoverflow.com/questions/3201154/automatically-add-all-files-in-a-folder-to-a-target-using-cmake
file(GLOB fileSrc
     "src/*.h"
     "src/*.cpp"
     "src/*.c"
     # subdirs
     "src/*/*.h"
     "src/*/*.cpp"
)

# https://stackoverflow.com/questions/28597351/how-do-i-add-a-library-path-in-cmake
add_library(libsndfile SHARED IMPORTED) # or STATIC instead of SHARED
set_target_properties(libsndfile PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/prebuild/libs/libsndfile.so.1"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/libsndfile/include/"
)

add_executable(inkaudio ${fileSrc})

# https://stackoverflow.com/questions/33149878/experimentalfilesystem-linker-error
target_link_libraries(inkaudio libsndfile)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
