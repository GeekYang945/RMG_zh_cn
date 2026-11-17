#
# Rosalie's Mupen GUI CMakeLists.txt
#
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Gui Widgets Core REQUIRED)
find_package(Qt6 COMPONENTS LinguistTools REQUIRED)

if (NOT WIN32)
    find_package(Qt6 REQUIRED COMPONENTS DBus)
endif(NOT WIN32)

if (PORTABLE_INSTALL)
    add_definitions(-DPORTABLE_INSTALL)
endif(PORTABLE_INSTALL)

if (UPDATER)
    find_package(Qt6 COMPONENTS Network REQUIRED)
    add_definitions(-DUPDATER)
endif(UPDATER)

if (APPIMAGE_UPDATER)
    add_definitions(-DAPPIMAGE_UPDATER)
endif(APPIMAGE_UPDATER)

if (KCA_DRAG_DROP)
    find_package(KF6CoreAddons REQUIRED)
    add_definitions(-DKCA_DRAG_DROP)
endif(KCA_DRAG_DROP)

if (NETPLAY)
    find_package(Qt6 COMPONENTS WebSockets REQUIRED)
    add_definitions(-DNETPLAY)
endif(NETPLAY)

find_package(SDL3 REQUIRED)

set(RMG_SOURCES
    UserInterface/MainWindow.cpp
    UserInterface/MainWindow.ui
    UserInterface/Widget/RomBrowser/RomBrowserWidget.cpp
    UserInterface/Widget/RomBrowser/RomBrowserListViewWidget.cpp
    UserInterface/Widget/RomBrowser/RomBrowserGridViewWidget.cpp
    UserInterface/Widget/RomBrowser/RomBrowserLoadingWidget.cpp
    UserInterface/Widget/RomBrowser/RomBrowserEmptyWidget.cpp
    UserInterface/Widget/RomBrowser/RomBrowserEmptyWidget.ui
    UserInterface/Widget/RomBrowser/RomBrowserSearchWidget.cpp
    UserInterface/Widget/Render/DummyWidget.cpp
    UserInterface/Widget/Render/OGLWidget.cpp
    UserInterface/Widget/Render/VKWidget.cpp
    UserInterface/Widget/KeybindButton.cpp
    UserInterface/Dialog/SettingsDialog.cpp
    UserInterface/Dialog/SettingsDialog.ui
    UserInterface/Dialog/Cheats/CheatsCommon.cpp
    UserInterface/Dialog/Cheats/CheatsDialog.cpp
    UserInterface/Dialog/Cheats/CheatsDialog.ui
    UserInterface/Dialog/Cheats/AddCheatDialog.cpp
    UserInterface/Dialog/Cheats/AddCheatDialog.ui
    UserInterface/Dialog/Cheats/ChooseCheatOptionDialog.cpp
    UserInterface/Dialog/Cheats/ChooseCheatOptionDialog.ui
    UserInterface/Dialog/RomInfoDialog.cpp
    UserInterface/Dialog/RomInfoDialog.ui
    UserInterface/Dialog/AboutDialog.cpp
    UserInterface/Dialog/AboutDialog.ui
    UserInterface/Dialog/LogDialog.cpp
    UserInterface/Dialog/LogDialog.ui
    UserInterface/NoFocusDelegate.cpp
    UserInterface/EventFilter.cpp
    UserInterface/UIResources.rc
    UserInterface/UIResources.qrc
    Translations/Translations.qrc
    Thread/RomSearcherThread.cpp
    Thread/EmulationThread.cpp
    Utilities/QtKeyToSdl3Key.cpp
    Utilities/QtMessageBox.cpp
    OnScreenDisplay.cpp
    Callbacks.cpp
    VidExt.cpp
    main.cpp
)

if (UPDATER)
    list(APPEND RMG_SOURCES
        UserInterface/Dialog/Update/UpdateDialog.cpp
        UserInterface/Dialog/Update/UpdateDialog.ui
        UserInterface/Dialog/Update/DownloadUpdateDialog.cpp
        UserInterface/Dialog/Update/DownloadUpdateDialog.ui
        UserInterface/Dialog/Update/InstallUpdateDialog.cpp
        UserInterface/Dialog/Update/InstallUpdateDialog.ui
    )
endif(UPDATER)

if (NETPLAY)
    list(APPEND RMG_SOURCES
        UserInterface/Dialog/Netplay/NetplayCommon.cpp
        UserInterface/Dialog/Netplay/CreateNetplaySessionDialog.cpp
        UserInterface/Dialog/Netplay/CreateNetplaySessionDialog.ui
        UserInterface/Dialog/Netplay/NetplaySessionBrowserDialog.cpp
        UserInterface/Dialog/Netplay/NetplaySessionBrowserDialog.ui
        UserInterface/Dialog/Netplay/NetplaySessionDialog.cpp
        UserInterface/Dialog/Netplay/NetplaySessionDialog.ui
        UserInterface/Dialog/Netplay/NetplaySessionPasswordDialog.cpp
        UserInterface/Dialog/Netplay/NetplaySessionPasswordDialog.ui
        UserInterface/Widget/Netplay/NetplaySessionBrowserWidget.cpp
        UserInterface/Widget/Netplay/NetplaySessionBrowserLoadingWidget.cpp
        UserInterface/Widget/Netplay/NetplaySessionBrowserEmptyWidget.cpp
        UserInterface/Widget/Netplay/NetplaySessionBrowserEmptyWidget.ui
        UserInterface/Widget/Netplay/CreateNetplaySessionWidget.cpp
        UserInterface/Widget/Netplay/CreateNetplaySessionEmptyWidget.cpp
        UserInterface/Widget/Netplay/CreateNetplaySessionEmptyWidget.ui
    )
endif(NETPLAY)

set_source_files_properties(${IMGUI_SOURCES} PROPERTIES GENERATED TRUE)
list(APPEND RMG_SOURCES ${IMGUI_SOURCES})

if (CMAKE_BUILD_TYPE MATCHES Release)
    add_executable(RMG WIN32 ${RMG_SOURCES})
else()
    add_executable(RMG ${RMG_SOURCES})
endif()

target_link_libraries(RMG 
    RMG-Core
    imgui
)

# needed for dynamically linked RMG-Core
if (PORTABLE_INSTALL)
    set_target_properties(RMG PROPERTIES
        INSTALL_RPATH "$ORIGIN"
    )
endif(PORTABLE_INSTALL)

target_include_directories(RMG PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${IMGUI_DIR}
)

target_link_libraries(RMG Qt6::Gui Qt6::Widgets)

if (NOT WIN32)
    target_link_libraries(RMG Qt6::DBus)
endif(NOT WIN32)

if (UPDATER)
    target_link_libraries(RMG Qt6::Network)
endif(UPDATER)

if (KCA_DRAG_DROP)
    target_link_libraries(RMG KF6::CoreAddons)
endif(KCA_DRAG_DROP)

if (NETPLAY)
    target_link_libraries(RMG Qt6::WebSockets)
endif(NETPLAY)

# translations for RMG itself,
# including the configuration dialogs of the
# input and audio plugins since those are
# shown from within RMG
file(GLOB_RECURSE RMG_EXTRA_TRANSLATION_SOURCES
    "${CMAKE_SOURCE_DIR}/Source/RMG-Input/UserInterface/*.cpp"
    "${CMAKE_SOURCE_DIR}/Source/RMG-Input/UserInterface/*.ui"
    "${CMAKE_SOURCE_DIR}/Source/RMG-Input-GCA/UserInterface/*.cpp"
    "${CMAKE_SOURCE_DIR}/Source/RMG-Input-GCA/UserInterface/*.ui"
    "${CMAKE_SOURCE_DIR}/Source/RMG-Audio/UserInterface/*.cpp"
    "${CMAKE_SOURCE_DIR}/Source/RMG-Audio/UserInterface/*.ui"
)

qt_add_translations(RMG
    TS_FILES
        Translations/rmg_zh_CN.ts
    RESOURCE_PREFIX "/i18n"
    SOURCES ${RMG_EXTRA_TRANSLATION_SOURCES}
)

# translations which lupdate doesn't extract from RMG's sources,
# such as Qt's standard dialog buttons and strings of GLideN64's
# configuration dialog which are missing in GLideN64's own translation
qt_add_lrelease(RMG
    TS_FILES
        Translations/rmg_extra_zh_CN.ts
    QM_FILES_OUTPUT_VARIABLE RMG_EXTRA_QM_FILES
)

qt_add_resources(RMG "extra_translations"
    PREFIX "/i18n"
    BASE "${CMAKE_CURRENT_BINARY_DIR}"
    FILES ${RMG_EXTRA_QM_FILES}
)

# assemble a runnable portable layout next to RMG.exe,
# so that RMG can be run/debugged directly from an IDE
# (i.e CLion) without having to install it first
set(RMG_RUN_DIR "$<TARGET_FILE_DIR:RMG>")

add_custom_command(TARGET RMG POST_BUILD
    # create the directory structure
    COMMAND ${CMAKE_COMMAND} -E make_directory "${RMG_RUN_DIR}/Core"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${RMG_RUN_DIR}/Plugin/Audio"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${RMG_RUN_DIR}/Plugin/GFX"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${RMG_RUN_DIR}/Plugin/Input"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${RMG_RUN_DIR}/Plugin/RSP"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${RMG_RUN_DIR}/Data"

    # core library (linked by RMG)
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:RMG-Core>" "${RMG_RUN_DIR}/"

    # mupen64plus core
    COMMAND ${CMAKE_COMMAND} -E copy "${MUPEN64PLUSCORE_LIB}" "${RMG_RUN_DIR}/Core/"

    # plugins
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:RMG-Audio>" "${RMG_RUN_DIR}/Plugin/Audio/"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:RMG-Input>" "${RMG_RUN_DIR}/Plugin/Input/"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:RMG-Input-GCA>" "${RMG_RUN_DIR}/Plugin/Input/"
    COMMAND ${CMAKE_COMMAND} -E copy "${MUPEN64PLUS_PLUGIN_INPUT_RAPHNET}" "${RMG_RUN_DIR}/Plugin/Input/"
    COMMAND ${CMAKE_COMMAND} -E copy "${MUPEN64PLUS_PLUGIN_GFX_GLIDEN64}" "${RMG_RUN_DIR}/Plugin/GFX/"
    COMMAND ${CMAKE_COMMAND} -E copy "${MUPEN64PLUS_PLUGIN_GFX_PARALLEL}" "${RMG_RUN_DIR}/Plugin/GFX/"
    COMMAND ${CMAKE_COMMAND} -E copy "${MUPEN64PLUS_PLUGIN_GFX_SOFTRDP}" "${RMG_RUN_DIR}/Plugin/GFX/"
    COMMAND ${CMAKE_COMMAND} -E copy "${MUPEN64PLUS_PLUGIN_RSP_CXD4}" "${RMG_RUN_DIR}/Plugin/RSP/"
    COMMAND ${CMAKE_COMMAND} -E copy "${MUPEN64PLUS_PLUGIN_RSP_HLE}" "${RMG_RUN_DIR}/Plugin/RSP/"
    COMMAND ${CMAKE_COMMAND} -E copy "${MUPEN64PLUS_PLUGIN_RSP_PARALLEL}" "${RMG_RUN_DIR}/Plugin/RSP/"

    # data files
    COMMAND ${CMAKE_COMMAND} -E copy "${MUPEN64PLUSCORE_INI}" "${RMG_RUN_DIR}/Data/"
    COMMAND ${CMAKE_COMMAND} -E copy "${SDL_GAMECONTROLLERDB}" "${RMG_RUN_DIR}/Data/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Data/InputProfileDB.json" "${RMG_RUN_DIR}/Data/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Data/font.ttf" "${RMG_RUN_DIR}/Data/"
    COMMAND ${CMAKE_COMMAND} -E copy "${MUPEN64PLUS_PLUGIN_GFX_GLIDEN64_DATA}" "${RMG_RUN_DIR}/Data/"

    # cheats
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Data/Cheats/Generated" "${RMG_RUN_DIR}/Data/Cheats"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Data/Cheats/Custom" "${RMG_RUN_DIR}/Data/Cheats"

    # force portable mode, so that config & saves are stored next to the exe
    COMMAND ${CMAKE_COMMAND} -E touch "${RMG_RUN_DIR}/portable.txt"
)

if (USE_ANGRYLION)
    add_custom_command(TARGET RMG POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${MUPEN64PLUS_PLUGIN_GFX_ANGRYLION}" "${RMG_RUN_DIR}/Plugin/GFX/"
    )
endif()

if (VRU)
    add_custom_command(TARGET RMG POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${VOSK_API_LIBRARY}" "${RMG_RUN_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E copy "${VOSK_API_MODEL}" "${RMG_RUN_DIR}/Data/"
    )
endif()

