function(add_qalam_test test_name source_file)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
        message(FATAL_ERROR "Missing test source for ${test_name}: ${source_file}")
    endif()

    # QalamTestSettings.cpp keeps every test away from the real user settings.
    add_executable(${test_name} ${source_file} QalamTestSettings.cpp)
    # Unit tests must be console executables on Windows. If a parent project or
    # Qt helper has initialized WIN32_EXECUTABLE, MinGW will look for WinMain()
    # instead of main(), which breaks QTEST_MAIN-based tests.
    set_target_properties(${test_name} PROPERTIES WIN32_EXECUTABLE OFF)
    target_link_libraries(${test_name} PRIVATE qalam_core Qt6::Test)
    qalam_harden_windows_runtime(${test_name})

    if(WIN32)
        # Launch every test with one normalized Path entry. Some Windows hosts
        # expose both "Path" and "PATH"; CTest/libuv can then lose the MinGW
        # runtime directory after its first child process, making later tests
        # fail at load time with 0xc0000135. The CMake wrapper itself uses only
        # Windows system DLLs, removes both spellings, and supplies the Qt and
        # compiler runtime directories explicitly for the actual test process.
        get_filename_component(qalam_compiler_runtime_dir
            "${CMAKE_CXX_COMPILER}" DIRECTORY)
        string(REPLACE ";" "\\;" qalam_host_path "$ENV{Path}")
        set(qalam_test_path
            "$<TARGET_FILE_DIR:Qt6::Core>\\;${qalam_compiler_runtime_dir}\\;${qalam_host_path}")
        set(qalam_test_platform "offscreen")
        if(ARGC GREATER 2)
            set(qalam_test_platform "${ARGV2}")
        endif()
        add_test(
            NAME ${test_name}
            COMMAND ${CMAKE_COMMAND} -E env
                --unset=Path
                --unset=PATH
                "Path=${qalam_test_path}"
                "QT_QPA_PLATFORM=${qalam_test_platform}"
                ${CMAKE_COMMAND}
                "-DQALAM_TEST_EXECUTABLE=$<TARGET_FILE:${test_name}>"
                "-DQALAM_TEST_LOG=${CMAKE_CURRENT_BINARY_DIR}/${test_name}.txt"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/RunQtTest.cmake"
        )
    else()
        add_test(NAME ${test_name} COMMAND ${test_name})
    endif()
endfunction()

add_test(
    NAME test_qalam_naming
    COMMAND ${CMAKE_COMMAND}
        "-DQALAM_ROOT=${CMAKE_SOURCE_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/CheckQalamNaming.cmake"
)

add_test(
    NAME test_qalam_icons
    COMMAND ${CMAKE_COMMAND}
        "-DQALAM_ROOT=${CMAKE_SOURCE_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/CheckQalamIcons.cmake"
)

add_qalam_test(test_diagnostic_parser TestDiagnosticParser.cpp)
add_qalam_test(test_workspace_indexer TestWorkspaceIndexer.cpp)
add_qalam_test(test_explorer_workspace_roots TestExplorerWorkspaceRoots.cpp)
target_sources(test_explorer_workspace_roots PRIVATE ../qalam/resources.qrc)
add_qalam_test(test_title_bar TestTitleBar.cpp)
target_sources(test_title_bar PRIVATE ../qalam/resources.qrc)
if(WIN32)
    # This test deliberately uses the real Windows QPA plugin. The offscreen
    # backend cannot expose HWND styles or non-client hit-test behavior, which
    # is exactly where frameless resize, maximize and Snap regressions occur.
    add_qalam_test(test_windows_frame TestWindowsFrame.cpp windows)
    target_sources(test_windows_frame PRIVATE ../qalam/resources.qrc)
endif()
add_qalam_test(test_workspace_file_service TestWorkspaceFileService.cpp)
add_qalam_test(test_project_search_service TestProjectSearchService.cpp)
add_qalam_test(test_command_registry TestCommandRegistry.cpp)
add_qalam_test(test_build_manager TestBuildManager.cpp)
add_qalam_test(test_toolchain_discovery TestToolchainDiscovery.cpp)
add_qalam_test(test_takween_protocol TestTakweenProtocol.cpp)
add_qalam_test(test_process_worker TestProcessWorker.cpp)
add_qalam_test(test_session_manager TestSessionManager.cpp)
add_qalam_test(test_session_slot TestSessionSlot.cpp)
add_qalam_test(test_welcome_page TestWelcomePage.cpp)
target_sources(test_welcome_page PRIVATE ../qalam/resources.qrc)
add_qalam_test(test_editor_workspace TestEditorWorkspace.cpp)
add_qalam_test(test_qalam_console TestQalamConsole.cpp)
target_sources(test_qalam_console PRIVATE ../qalam/resources.qrc)
add_qalam_test(test_snippet_manager TestSnippetManager.cpp)
add_qalam_test(test_symbol_outline_view TestSymbolOutlineView.cpp)
add_qalam_test(test_panel_area TestPanelArea.cpp)
add_qalam_test(test_search_panel TestSearchPanel.cpp)
add_qalam_test(test_arabic_search TestArabicSearch.cpp)
target_sources(test_arabic_search PRIVATE ../qalam/resources.qrc)
add_qalam_test(test_search_view TestSearchView.cpp)
target_sources(test_panel_area PRIVATE ../qalam/resources.qrc)
target_sources(test_search_panel PRIVATE ../qalam/resources.qrc)
target_sources(test_search_view PRIVATE ../qalam/resources.qrc)

add_qalam_test(test_lsp_message_framer TestLspMessageFramer.cpp)
add_qalam_test(test_editor_semantic_requests TestEditorSemanticRequests.cpp)
add_qalam_test(test_bracket_handler TestBracketHandler.cpp)
add_qalam_test(test_bracket_matcher TestBracketMatcher.cpp)
add_qalam_test(test_completion_popup TestCompletionPopup.cpp)
add_qalam_test(test_completion_history TestCompletionHistory.cpp)
add_qalam_test(test_syntax_highlighter TestSyntaxHighlighter.cpp)
add_qalam_test(test_workspace_edit TestWorkspaceEdit.cpp)
add_qalam_test(test_arabic_editing TestArabicEditing.cpp)
add_qalam_test(test_text_position TestTextPosition.cpp)
add_qalam_test(test_keybindings TestKeybindings.cpp)
add_qalam_test(test_auto_save TestAutoSave.cpp)
add_qalam_test(test_qalam_theme TestQalamTheme.cpp)
target_sources(test_keybindings PRIVATE ../qalam/resources.qrc)

add_executable(qalam_fake_baa_lsp
    FakeBaaLsp.cpp
    ../source/language/LspMessageFramer.cpp
    ../source/language/LspMessageFramer.h
)
set_target_properties(qalam_fake_baa_lsp PROPERTIES WIN32_EXECUTABLE OFF)
target_include_directories(qalam_fake_baa_lsp PRIVATE ../source/language)
target_link_libraries(qalam_fake_baa_lsp PRIVATE Qt6::Core)
qalam_harden_windows_runtime(qalam_fake_baa_lsp)

add_qalam_test(test_baa_language_client TestBaaLanguageClient.cpp)
add_dependencies(test_baa_language_client qalam_fake_baa_lsp)
target_compile_definitions(test_baa_language_client PRIVATE
    QALAM_FAKE_BAA_LSP_PATH="$<TARGET_FILE:qalam_fake_baa_lsp>")

# Opt-in release gate using the actual workbench, server, and compiler.
set(QALAM_TEST_BAA_LSP "" CACHE FILEPATH "Baa-LSP executable for the completion integration gate")
set(QALAM_TEST_BAA_COMPILER "" CACHE FILEPATH "Baa compiler for the completion integration gate")
if(QALAM_TEST_BAA_LSP AND QALAM_TEST_BAA_COMPILER)
    if(WIN32)
        add_qalam_test(test_workbench_completion TestWorkbenchCompletion.cpp windows)
    else()
        add_qalam_test(test_workbench_completion TestWorkbenchCompletion.cpp)
    endif()
    set_tests_properties(test_workbench_completion PROPERTIES TIMEOUT 90)
    target_sources(test_workbench_completion PRIVATE
        ../qalam/Qalam.cpp ../qalam/Qalam.h ../qalam/resources.qrc)
    target_compile_definitions(test_workbench_completion PRIVATE
        QALAM_TEST_BAA_LSP="${QALAM_TEST_BAA_LSP}"
        QALAM_TEST_BAA_COMPILER="${QALAM_TEST_BAA_COMPILER}")
endif()

# Always-on workbench chrome guard; also saves review screenshots on request.
if(WIN32)
    # The real platform plugin renders fonts and native frames for review.
    add_qalam_test(test_workbench_chrome TestWorkbenchChrome.cpp windows)
else()
    add_qalam_test(test_workbench_chrome TestWorkbenchChrome.cpp)
endif()
target_sources(test_workbench_chrome PRIVATE
    ../qalam/Qalam.cpp ../qalam/Qalam.h ../qalam/resources.qrc)
