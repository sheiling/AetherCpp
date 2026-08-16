if(NOT DEFINED LRELEASE_EXECUTABLE)
    message(FATAL_ERROR "LRELEASE_EXECUTABLE is not set.")
endif()

# VS/MSBuild 下 -D 可能带转义引号，这里做一次清洗
string(REPLACE "\"" "" LRELEASE_EXECUTABLE "${LRELEASE_EXECUTABLE}")

if(NOT DEFINED SOURCE_DIR)
    if(NOT DEFINED SOURCE_CONFIG_DIR)
        message(FATAL_ERROR "SOURCE_DIR or SOURCE_CONFIG_DIR must be set.")
    endif()
    string(REPLACE "\"" "" SOURCE_CONFIG_DIR "${SOURCE_CONFIG_DIR}")
    set(SOURCE_DIR "${SOURCE_CONFIG_DIR}")
else()
    string(REPLACE "\"" "" SOURCE_DIR "${SOURCE_DIR}")
endif()

if(NOT DEFINED OUTPUT_DIR)
    set(OUTPUT_DIR "${SOURCE_DIR}")
else()
    string(REPLACE "\"" "" OUTPUT_DIR "${OUTPUT_DIR}")
endif()

if(NOT DEFINED TS_FILES)
    set(TS_FILES
        "app_zh_CN.ts"
        "app_en_US.ts"
    )
else()
    string(REPLACE "\"" "" TS_FILES "${TS_FILES}")
    string(REPLACE "|" ";" TS_FILES "${TS_FILES}")
endif()

file(MAKE_DIRECTORY "${OUTPUT_DIR}")

foreach(_ts_file IN LISTS TS_FILES)
    set(_ts_path "${SOURCE_DIR}/${_ts_file}")
    string(REPLACE ".ts" ".qm" _qm_file "${_ts_file}")
    set(_qm_path "${OUTPUT_DIR}/${_qm_file}")

    execute_process(
        COMMAND "${LRELEASE_EXECUTABLE}" "${_ts_path}" -qm "${_qm_path}"
        RESULT_VARIABLE _ret
        OUTPUT_VARIABLE _out
        ERROR_VARIABLE _err
    )

    if(NOT _ret EQUAL 0)
        message(FATAL_ERROR "lrelease failed for ${_ts_file}\n${_out}\n${_err}")
    endif()

    message(STATUS "已生成翻译文件 ${_qm_path}")
endforeach()
