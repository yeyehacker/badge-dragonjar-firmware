file(GLOB_RECURSE MININO_SOURCE_FILES "$ENV{MININO_PATH}/main/*")
file(GLOB_RECURSE LOCAL_SOURCE_FILES "./*")
set(all_src_files "")

foreach(file ${MININO_SOURCE_FILES})
    get_filename_component(extension ${file} EXT)
    # Exclude main.c from Minino
    if(NOT extension STREQUAL "" AND NOT ${file} MATCHES "main.c")
        list(APPEND all_src_files ${file})
    endif()
endforeach()

# Agregando archivos fuente locales
foreach(file ${LOCAL_SOURCE_FILES})
    get_filename_component(extension ${file} EXT)

    if(NOT extension STREQUAL "")
        list(APPEND all_src_files ${file})
    endif()
endforeach()

# 💡 INICIO DE CORRECCIÓN: Añadir archivos .c de la subcarpeta del módulo
# Buscamos en la carpeta donde pusiste deauthalizer_module.c
file(GLOB_RECURSE DEAUTHALIZER_SOURCE_FILES "apps/badge_link/*.c")
foreach(file ${DEAUTHALIZER_SOURCE_FILES})
    list(APPEND all_src_files ${file})
endforeach()
# 💡 FIN DE CORRECCIÓN

file(GLOB_RECURSE ALL_INCLUDE_PATHS "$ENV{MININO_PATH}/main/*")
set(minino_headers_dirs "")

foreach(path ${ALL_INCLUDE_PATHS})
    get_filename_component(parentDir ${path} DIRECTORY)
    # Exclude menus_include/menus.h from Minino
    if(IS_DIRECTORY ${parentDir} AND NOT ${parentDir} MATCHES "menus_include")
        list(APPEND minino_headers_dirs ${parentDir})
    endif()
endforeach()

file(GLOB_RECURSE ALL_INCLUDE_PATHS "./*")
set(local_headers_dirs "")

foreach(path ${ALL_INCLUDE_PATHS})
    get_filename_component(parentDir ${path} DIRECTORY)

    if(IS_DIRECTORY ${parentDir})
        list(APPEND local_headers_dirs ${parentDir})
    endif()
endforeach()

# 💡 CORRECCIÓN: Asegurar que el directorio de inclusión del módulo se encuentre
list(APPEND local_headers_dirs "apps/badge_link")

idf_component_register(
    SRCS
    "${all_src_files}"
    "main.c"
    EMBED_FILES
    $ENV{MININO_PATH}/main/modules/web_file_browser/http_server/header.html
    $ENV{MININO_PATH}/main/modules/web_file_browser/http_server/favicon_b.ico
    $ENV{MININO_PATH}/main/modules/web_file_browser/http_server/style.css
    INCLUDE_DIRS
    "${minino_headers_dirs}"
    "${local_headers_dirs}")
