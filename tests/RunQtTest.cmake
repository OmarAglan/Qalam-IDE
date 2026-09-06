if(NOT DEFINED QALAM_TEST_EXECUTABLE OR NOT DEFINED QALAM_TEST_LOG)
    message(FATAL_ERROR "A QtTest executable and log path are required")
endif()

# Some Windows launch environments lose QtTest's inherited stdout stream.
# A named log retains assertion details; relay it to CTest even on failure.
# Truncate first so an early loader failure cannot report an older result.
file(WRITE "${QALAM_TEST_LOG}" "")
execute_process(
    COMMAND "${QALAM_TEST_EXECUTABLE}" -o "${QALAM_TEST_LOG},txt"
    RESULT_VARIABLE qalam_test_result
)
file(READ "${QALAM_TEST_LOG}" qalam_test_output)
if(NOT qalam_test_output STREQUAL "")
    message("${qalam_test_output}")
endif()
if(NOT qalam_test_result STREQUAL "0")
    message(FATAL_ERROR "QtTest exited with ${qalam_test_result}")
endif()
if(qalam_test_output STREQUAL "")
    message(FATAL_ERROR "QtTest exited without producing a test log")
endif()
