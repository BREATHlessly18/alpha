cmake_minimum_required(VERSION 3.5)

project(CAMERA)

aux_source_directory(${PROJECT_SOURCE_DIR} DIR_MAIN_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/source DIR_OTHER_SRCS)

# -I
include_directories(${PROJECT_SOURCE_DIR}/include)
#include_directories(${PROJECT_SOURCE_DIR}/common/include)
include_directories(/usr/softinstall/ffmpeg/include)

# parameterss
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAG ${CMAKE_CXX_FLAG} "-O2 -Wall -g ")

# macro
#add_definitions(-D 预处理宏)
#add_definitions(-D 预处理宏)

# -L
#set(LIB_DIR ${PROJECT_SOURCE_DIR}/common/lib)
set(LIB_DIR /usr/softinstall/ffmpeg/lib)  
link_directories(${LIB_DIR})

# -o
add_executable(camera ${DIR_MAIN_SRCS} ${DIR_OTHER_SRCS})

# -l
set(LIB_NAME avcodec avdevice avfilter avformat avutil postproc swresample swscale x264 pthread)
target_link_libraries(camera ${LIB_NAME})
