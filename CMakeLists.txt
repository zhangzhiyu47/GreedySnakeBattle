cmake_minimum_required(VERSION 3.13)
project(GreedySnakeBattle LANGUAGES C)

set(VERSION_MAJOR 6)
set(VERSION_MINOR 2)
set(VERSION_PATCH 0)
set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set(SOURCES
    src/main.c
    src/exitApp.c
    src/initializeApp.c
    src/gameMainLogic.c
    src/initGameData.c
    src/global.c
    src/logger.c
    src/obstacleSnake.c
    src/painting.c
    src/terminal.c
    src/gameMenu.c
    src/userSnake.c
    src/gameConfig.c
    src/menu.c
    src/browser.c
    src/button.c
    src/number.c
)

add_executable(gsb ${SOURCES})

target_link_libraries(gsb PRIVATE m)

target_compile_options(gsb PRIVATE
    -Wall
    -Wextra
    -g
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Release>:-O2>
)

target_link_options(gsb PRIVATE
    $<$<CONFIG:Release>:-flto>
)

add_dependencies(gsb gen_res_headers)

set(RES_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/res")
set(RES_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/include/res")

file(MAKE_DIRECTORY "${RES_INC_DIR}")

set(RES_MAPPINGS
    "gameIntro.txt:gameIntro"
    "EULA.txt:EULA"
)

foreach(mapping IN LISTS RES_MAPPINGS)
    string(REPLACE ":" ";" parts "${mapping}")
    list(GET parts 0 txt_name)
    list(GET parts 1 var_prefix)

    set(txt_path "${RES_SRC_DIR}/${txt_name}")
    string(REGEX REPLACE "\\.txt$" ".h" h_name "${txt_name}")
    set(h_path "${RES_INC_DIR}/${h_name}")

    add_custom_command(
        OUTPUT "${h_path}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${RES_INC_DIR}"
        COMMAND xxd -i -n "${var_prefix}" "${txt_path}" > "${h_path}"
        DEPENDS "${txt_path}"
        COMMENT "Generating C header ${h_name}"
        VERBATIM
    )

    list(APPEND GENERATED_RES_HEADERS "${h_path}")
endforeach()

add_custom_target(gen_res_headers ALL DEPENDS ${GENERATED_RES_HEADERS})

if(ANDROID)
    if(DEFINED ENV{PREFIX})
        set(CMAKE_INSTALL_PREFIX $ENV{PREFIX})
    else()
        set(CMAKE_INSTALL_PREFIX "/data/data/com.termux/files/usr")
    endif()
endif()

if(ANDROID)  
    install(TARGETS gsb
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
else()
    include(GNUInstallDirs)
    
    install(TARGETS gsb
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_uninstall.cmake
    COMMENT "Uninstalling..."
)
