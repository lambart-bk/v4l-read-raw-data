list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS}  )
set( CMAKE_CXX_FLAGS "-std=c++11 -O3" )
set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY /home/pi/work/ASC/run/)

add_executable(IRcamera main.cpp uart.c)
target_link_libraries(IRcamera ${OpenCV_LIBRARIES} -lpthread)
#target_link_libraries(IRcamera ${OpenCV_LIBRARIES} -ljpeg -lz )

