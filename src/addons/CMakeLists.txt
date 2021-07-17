project(score_addons LANGUAGES CXX)

macro(SUBDIRLIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
        list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()


subdirlist(SUBDIRS "${CMAKE_CURRENT_SOURCE_DIR}")
set(SCORE_ADDON True)
foreach(subdir ${SUBDIRS})
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt")
    add_subdirectory("${subdir}")
  endif()
endforeach()


if(NOT TARGET score_lib_base)
  include("${SCORE_SOURCE_DIR}/cmake/ScoreExternalAddon.cmake")
endif()

project(score_plugin_analysis_gist LANGUAGES CXX)

if(NOT "${OSSIA_FFT}" STREQUAL "DOUBLE")
  message(FATAL_ERROR "Analysis needs FFTW (double)")
  return()
endif()

add_library(
        score_plugin_analysis_gist
        Analysis/Envelope.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/score_plugin_analysis.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/score_plugin_analysis.cpp

        3rdparty/Gist/src/CoreFrequencyDomainFeatures.cpp
        3rdparty/Gist/src/CoreFrequencyDomainFeatures.h
        3rdparty/Gist/src/CoreTimeDomainFeatures.cpp
        3rdparty/Gist/src/CoreTimeDomainFeatures.h
        3rdparty/Gist/src/Gist.cpp
        3rdparty/Gist/src/Gist.h
        3rdparty/Gist/src/MFCC.cpp
        3rdparty/Gist/src/MFCC.h
        3rdparty/Gist/src/OnsetDetectionFunction.cpp
        3rdparty/Gist/src/OnsetDetectionFunction.h
        3rdparty/Gist/src/WindowFunctions.cpp
        3rdparty/Gist/src/WindowFunctions.h
        3rdparty/Gist/src/Yin.cpp
        3rdparty/Gist/src/Yin.h
)

target_compile_definitions(score_plugin_analysis_gist PRIVATE USE_OSSIA_FFT=1)
target_include_directories(score_plugin_analysis_gist PRIVATE 3rdparty/Gist/src)
target_link_libraries(score_plugin_analysis_gist PUBLIC score_plugin_engine ${FFTW3_LIBRARY})

setup_score_plugin(score_plugin_analysis_gist)