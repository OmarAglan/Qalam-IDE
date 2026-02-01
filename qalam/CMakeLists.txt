qt_add_executable(Qalam
    main.cpp
    Qalam.cpp
    Qalam.h
    resources.qrc
    ../source/pages/TWelcomeWindow.cpp
)

# Platform-specific icons
if(WIN32)
    # Ensure the directory exists or the path is correct relative to this CMakeLists.txt
    target_sources(Qalam PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/resources/QalamLogo.ico)
    set_target_properties(Qalam PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    set_target_properties(Qalam PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE QalamLogo.icns
    )
endif()

target_link_libraries(Qalam PRIVATE qalam_core)

# Add the current directory (qalam/) to include paths so TWelcomeWindow can find Qalam.h
target_include_directories(Qalam PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
