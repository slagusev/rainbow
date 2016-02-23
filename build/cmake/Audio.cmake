if(ANDROID)
  # Pass through.
elseif(EMSCRIPTEN)
  find_package(OpenAL REQUIRED)
  set(AUDIO_LIBRARIES ${OPENAL_LIBRARY})
elseif(USE_FMOD_STUDIO)
  set(AUDIO_INCLUDE_DIRS ${LOCAL_LIBRARY}/FMOD/inc)
  if(WIN32)
    set(FMOD_ARCH windows)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(FMOD_LIBRARY_SUFFIX 64_vc.lib)
    else()
      set(FMOD_LIBRARY_SUFFIX _vc.lib)
    endif()
  else()
    if(APPLE)
      set(FMOD_ARCH mac)
    else()
      if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(FMOD_ARCH linux/x86_64)
      else()
        set(FMOD_ARCH linux/x86)
      endif()
    endif()
    set(FMOD_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
  endif()
  set(FMOD_LIBRARY_PREFIX ${LOCAL_LIBRARY}/FMOD/lib/${FMOD_ARCH}/${CMAKE_SHARED_LIBRARY_PREFIX})
  set(AUDIO_LIBRARIES ${FMOD_LIBRARY_PREFIX}fmod${FMOD_LIBRARY_SUFFIX}
                      ${FMOD_LIBRARY_PREFIX}fmodstudio${FMOD_LIBRARY_SUFFIX})
  message(STATUS "Found FMOD Studio: ${AUDIO_LIBRARIES}")
else()
  if(WIN32)
    download_library(
        openal-soft
        http://kcat.strangesoft.net/openal-binaries/openal-soft-1.17.2-bin.zip
        2128f30ddc32baa77c49a28d9dd9a5e53178851d
        ${LOCAL_LIBRARY}/openal-soft)
    copy_to_build_dir(openal-soft bin/Win32/soft_oal.dll OpenAL32.dll)
    add_dependencies(rainbow openal-soft)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(OPENAL_ARCH Win64)
    else()
      set(OPENAL_ARCH Win32)
    endif()
    set(OPENAL_INCLUDE_DIR ${LOCAL_LIBRARY}/openal-soft/include)
    set(OPENAL_LIBRARY ${LOCAL_LIBRARY}/openal-soft/libs/${OPENAL_ARCH}/libOpenAL32.dll.a)
    message(STATUS "Found OpenAL: ${OPENAL_LIBRARY}")
  else()
    find_package(OpenAL REQUIRED)
    if(APPLE)
      find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox REQUIRED)
      set(PLATFORM_LIBRARIES ${AUDIOTOOLBOX_LIBRARY} ${PLATFORM_LIBRARIES})
    endif()
  endif()
  find_package(Vorbisfile REQUIRED)
  set(AUDIO_INCLUDE_DIRS ${VORBISFILE_INCLUDE_DIR} ${OPENAL_INCLUDE_DIR})
  set(AUDIO_LIBRARIES ${VORBISFILE_LIBRARIES} ${OPENAL_LIBRARY})
endif()
