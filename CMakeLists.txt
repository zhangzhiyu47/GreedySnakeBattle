cmake_minimum_required(VERSION 3.10)
project(GreedySnakeBattle LANGUAGES C)

# 设置项目版本
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

# 根据操作系统设置不同的配置
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(EXECUTABLE_EXTENSION ".exe")
    add_definitions(-DSNAKE_OS_WINDOWS=1)
elseif(ANDROID)  # Termux检测
    set(EXECUTABLE_EXTENSION "")
    add_definitions(-DSNAKE_OS_TERMUX=1)
elseif(APPLE)
    set(EXECUTABLE_EXTENSION "")
    add_definitions(-DSNAKE_OS_MACOS=1)
else()  # 其他Linux系统
    set(EXECUTABLE_EXTENSION "")
    add_definitions(-DSNAKE_OS_LINUX=1)
endif()

# 查找需要的包
find_package(Doxygen REQUIRED)

# 设置源文件
set(MAIN_SOURCES
    source/main.c
    source/exitApp.c
    source/food.c
    source/gameApplicationStartupRelated.c
    source/gameMainLogic.c
    source/gameSetConfiguration.c
    source/gameStartupRelated.c
    source/globalVariable.c
    source/obstacleSnake.c
    source/painting.c
    source/signalCapture.c
    source/snakeFullCompat.c
    source/standardIO.c
    source/terminal.c
    source/userInterfaceBeforeGameStarts.c
    source/userSnake.c
    source/wall.c
)

set(LIB_SOURCES
    source/GreedySnakeBattleGameExternalInterface.c
    ${MAIN_SOURCES}
)

# 添加可执行文件
add_executable(GreedySnakeBattle ${MAIN_SOURCES})

# 设置可执行文件属性
if(WIN32)
    # Windows下设置图标
    if(EXISTS "${CMAKE_SOURCE_DIR}/picture/greedy_snake_battle_logo.jpg")
        # 需要先将jpg转换为ico文件
        find_program(CONVERT_EXE convert PATHS "C:/Program Files/ImageMagick*")
        if(CONVERT_EXE)
            add_custom_command(
                OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/greedy_snake_battle_logo.ico
                COMMAND ${CONVERT_EXE} 
                    ${CMAKE_SOURCE_DIR}/picture/greedy_snake_battle_logo.jpg 
                    ${CMAKE_CURRENT_BINARY_DIR}/greedy_snake_battle_logo.ico
                DEPENDS ${CMAKE_SOURCE_DIR}/picture/greedy_snake_battle_logo.jpg
            )
            add_custom_target(convert_logo ALL 
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/greedy_snake_battle_logo.ico)
            
            # 创建资源文件
            file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/app.rc
                "IDI_ICON1 ICON DISCARDABLE \"greedy_snake_battle_logo.ico\"\n")
            
            target_sources(GreedySnakeBattle PRIVATE 
                ${CMAKE_CURRENT_BINARY_DIR}/app.rc
                ${CMAKE_CURRENT_BINARY_DIR}/greedy_snake_battle_logo.ico)
        endif()
    endif()
endif()

# 添加静态库
add_library(gsnakebg_static STATIC ${LIB_SOURCES})
set_target_properties(gsnakebg_static PROPERTIES OUTPUT_NAME "gsnakebg")

# 添加动态库
add_library(gsnakebg_shared SHARED ${LIB_SOURCES})
set_target_properties(gsnakebg_shared PROPERTIES OUTPUT_NAME "gsnakebg")

# 包含目录
target_include_directories(GreedySnakeBattle PRIVATE 
    source/Include
    source/Include/Compat
    source/Include/Functions
    source/Include/GlobalVariable
    source/Include/Struct
    source/Include/TypeDefine)

target_include_directories(gsnakebg_static PRIVATE 
    source/Include
    source/Include/Compat
    source/Include/Functions
    source/Include/GlobalVariable
    source/Include/Struct
    source/Include/TypeDefine)

target_include_directories(gsnakebg_shared PRIVATE 
    source/Include
    source/Include/Compat
    source/Include/Functions
    source/Include/GlobalVariable
    source/Include/Struct
    source/Include/TypeDefine)

# 链接库
if(WIN32)
    target_link_libraries(GreedySnakeBattle PRIVATE kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib)
endif()

# Doxygen文档生成
add_custom_target(API_documents
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile.txt
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
    
# 检测Termux环境
if(ANDROID)
    if(DEFINED ENV{PREFIX})
        set(CMAKE_INSTALL_PREFIX $ENV{PREFIX})
    else()
        set(CMAKE_INSTALL_PREFIX "/data/data/com.termux/files/usr")
    endif()
endif()

# 安装规则
if(WIN32)
    # Windows安装规则
    install(TARGETS GreedySnakeBattle
        RUNTIME DESTINATION .
        BUNDLE DESTINATION .)
    
    install(TARGETS gsnakebg_static
        ARCHIVE DESTINATION lib)
    
    install(TARGETS gsnakebg_shared
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
    
    install(DIRECTORY source/Include/
        DESTINATION include/GreedySnakeBattle
        FILES_MATCHING PATTERN "*.h")
    
    # 创建快捷方式
    install(CODE "
        include(CMakeDesktopShortcuts)
        create_desktop_shortcut(
            TARGET GreedySnakeBattle
            NAME \"Greedy Snake Battle\"
            ICON \"${CMAKE_SOURCE_DIR}/picture/greedy_snake_battle_logo.jpg\"
            WORKING_DIRECTORY \"$ENV{USERPROFILE}\\Desktop\"
        )
    ")
    
elseif(ANDROID)  # Termux安装规则
    # Termux的特殊安装路径
    install(TARGETS GreedySnakeBattle
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    
    install(TARGETS gsnakebg_static gsnakebg_shared
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
    
    install(DIRECTORY source/Include/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/GreedySnakeBattle
        FILES_MATCHING PATTERN "*.h")
    
else()  # Linux/macOS安装规则
    # 标准Unix安装路径
    include(GNUInstallDirs)
    
    install(TARGETS GreedySnakeBattle
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    
    install(TARGETS gsnakebg_static
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    
    install(TARGETS gsnakebg_shared
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    
    install(DIRECTORY source/Include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GreedySnakeBattle
        FILES_MATCHING PATTERN "*.h")
    
    # 文档安装
    install(DIRECTORY ${CMAKE_BINARY_DIR}/API_documents/
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/GreedySnakeBattle)
    
    # 桌面快捷方式(Linux)
    if(UNIX AND NOT APPLE)
        install(CODE "
            file(WRITE \"${CMAKE_BINARY_DIR}/GreedySnakeBattle.desktop\"
                \"[Desktop Entry]\\n\"
                \"Type=Application\\n\"
                \"Name=Greedy Snake Battle\\n\"
                \"Exec=${CMAKE_INSTALL_FULL_BINDIR}/GreedySnakeBattle\\n\"
                \"Icon=${CMAKE_INSTALL_FULL_DATADIR}/icons/greedy_snake_battle_logo.jpg\\n\"
                \"Categories=Game;\\n\"
                \"Terminal=true\\n\")
            
            install(FILES \"${CMAKE_BINARY_DIR}/GreedySnakeBattle.desktop\"
                DESTINATION \"$ENV{HOME}/Desktop\")
            
            install(FILES \"${CMAKE_SOURCE_DIR}/picture/greedy_snake_battle_logo.jpg\"
                DESTINATION \"${CMAKE_INSTALL_FULL_DATADIR}/icons\")
        ")
    endif()
endif()

# 文档安装(通用)
install(DIRECTORY ${CMAKE_BINARY_DIR}/API_documents/
    DESTINATION share/doc/GreedySnakeBattle
    PATTERN ".svn" EXCLUDE)
