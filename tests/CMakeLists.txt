function(add_qalam_test test_name source_file)
    add_executable(${test_name} ${source_file})
    target_link_libraries(${test_name} PRIVATE qalam_core Qt6::Test)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_qalam_test(test_diagnostic_parser TestDiagnosticParser.cpp)
add_qalam_test(test_workspace_indexer TestWorkspaceIndexer.cpp)
add_qalam_test(test_command_registry TestCommandRegistry.cpp)
