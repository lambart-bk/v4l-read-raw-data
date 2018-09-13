#list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS}  )
set( CMAKE_CXX_FLAGS "-std=c++11 -O3" )
set(CMAKE_BUILD_TYPE "Debug")
add_executable(v4l-read-raw main.cpp)
target_link_libraries(v4l-read-raw ${OpenCV_LIBRARIES} )
#target_link_libraries(for_test ${OpenCV_LIBRARIES} -ljpeg)

