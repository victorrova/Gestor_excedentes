# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "." "../include"
                    EMBED_FILES "../src/web/logo.ico" "../src/web/index.html" "../src/web/app.css" "../src/web/app.js")
