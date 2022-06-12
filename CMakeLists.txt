cmake_minimum_required(VERSION 3.5)

project(CAPTURE)

aux_source_directory(${PROJECT_SOURCE_DIR} MAIN_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/capture CAPTURE_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/base BASE_SRCS)

# -I
include_directories(${PROJECT_SOURCE_DIR}/3rdparty)
include_directories(${PROJECT_SOURCE_DIR})

# option
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAG ${CMAKE_CXX_FLAG} "-Ofast -Wall")

# macro


# -L

# -o
add_executable(h264_capture ${MAIN_SRCS} ${CAPTURE_SRCS} ${BASE_SRCS})

# -l
set(LIBS avcodec avdevice avfilter avformat avutil postproc swresample swscale x264 pthread)
target_link_libraries(h264_capture ${LIBS})


install(TARGETS h264_capture DESTINATION .)
