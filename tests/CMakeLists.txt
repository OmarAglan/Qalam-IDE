function(add_qalam_test test_name source_file)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
        message(FATAL_ERROR "Missing test source for ${test_name}: ${source_file}")
    endif()

    add_executable(${test_name} ${source_file})
    # Unit tests must be console executables on Windows. If a parent project or
    # Qt helper has initialized WIN32_EXECUTABLE, MinGW will look for WinMain()
    # instead of main(), which breaks QTEST_MAIN-based tests.
    set_target_properties(${test_name} PROPERTIES WIN32_EXECUTABLE OFF)
    target_link_libraries(${test_name} PRIVATE qalam_core Qt6::Test)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_qalam_test(test_diagnostic_parser TestDiagnosticParser.cpp)
add_qalam_test(test_workspace_indexer TestWorkspaceIndexer.cpp)
add_qalam_test(test_command_registry TestCommandRegistry.cpp)
add_qalam_test(test_build_manager TestBuildManager.cpp)
add_qalam_test(test_takween_protocol TestTakweenProtocol.cpp)
add_qalam_test(test_process_worker TestProcessWorker.cpp)
