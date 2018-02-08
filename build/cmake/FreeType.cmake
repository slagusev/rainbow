if(EMSCRIPTEN)
  set(EMCC_FLAGS "${EMCC_FLAGS} -s USE_FREETYPE=1")
else()
  add_library(freetype STATIC ${THIRD_PARTY}/FreeType/freetype.c)
  target_include_directories(
      freetype
      PUBLIC
          ${THIRD_PARTY}/FreeType
          ${LOCAL_LIBRARY}/FreeType/include
      PRIVATE
          ${LOCAL_LIBRARY}/FreeType/builds
          ${LOCAL_LIBRARY}/FreeType/src
  )
  target_link_libraries(rainbow freetype)
endif()
