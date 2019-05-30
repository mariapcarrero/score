set(CMAKE_BUILD_TYPE Release CACHE INTERNAL "")
set(SCORE_COTIRE True)
set(DEPLOYMENT_BUILD True)
set(SCORE_AUDIO_PLUGINS True CACHE INTERNAL "")

if(NOT DEFINED SCORE_ENABLE_LTO)
set(SCORE_ENABLE_LTO True)
endif()
if(UNIX AND EXISTS "/etc/centos-release")
  set(SCORE_ENABLE_LTO False)
endif()

if(MSYS OR MINGW)
  set(SCORE_ENABLE_LTO False)
endif()

include(default-plugins)