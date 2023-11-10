# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "." "../include"
                    EMBED_FILES "../web/favicon.ico" "../web/index.html" "../web/app.css" "../web/app.js" "../web/jquery-3.3.1.min.js")

