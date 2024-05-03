cmake_minimum_required(VERSION 3.25)
project(cpp_lab_2 C)

add_executable(cpp_lab_2 main.c audio_util.c)

set(CMAKE_C_STANDARD 23)

set(FFMPEG_DIR "C:/Users/ffmpeg-7.0-full_build-shared")
set(FFTW_DIR "C:/Users/fftw-3.3.5-dll64")

set(FFMPEG_INCLUDE_DIR ${FFMPEG_DIR}/include)
set(FFMPEG_LIB_DIR ${FFMPEG_DIR}/lib)

set(FFTW_INCLUDE_DIR ${FFTW_DIR}/include)
set(FFTW_LIB_DIR ${FFTW_DIR})

target_include_directories(cpp_lab_2 PRIVATE ${FFMPEG_INCLUDE_DIR} ${FFTW_INCLUDE_DIR})
link_directories(${FFMPEG_LIB_DIR} ${FFTW_LIB_DIR})

set(FFMPEG_LIBRARIES avcodec avdevice avfilter avformat avutil swresample swscale)
set(FFTW_LIBRARIES fftw3-3 fftw3f-3 fftw3l-3)

target_link_libraries(cpp_lab_2
        ${FFMPEG_LIB_DIR}/avcodec.lib
        ${FFMPEG_LIB_DIR}/avdevice.lib
        ${FFMPEG_LIB_DIR}/avfilter.lib
        ${FFMPEG_LIB_DIR}/avformat.lib
        ${FFMPEG_LIB_DIR}/avutil.lib
        ${FFMPEG_LIB_DIR}/swresample.lib
        ${FFMPEG_LIB_DIR}/swscale.lib
        ${FFTW_LIB_DIR}/libfftw3-3.dll
        ${FFTW_LIB_DIR}/libfftw3f-3.dll
        ${FFTW_LIB_DIR}/libfftw3l-3.dll
        )
