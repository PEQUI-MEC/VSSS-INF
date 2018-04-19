function(preNotFound)
    echo "\033[0;31m Resolvendo pré-requisitos do OpenCV \033[0m\n"
    set(MAKE_CMD "${CMAKE_CURRENT_SOURCE_DIR}/dependencies.sh -d essential")
    execute_process(COMMAND ${MAKE_CMD} ${INC_DIR} 
                    RESULT_VARIABLE CMD_ERROR
                    OUTPUT_FILE CMD_OUTPUT )
    MESSAGE( STATUS "CMD_ERROR:" ${CMD_ERROR})
endfunction(preNotFound)


function(opencvNotFound)
    echo "\033[0;31m Resolvendo OpenCV \033[0m\n"
    set(MAKE_CMD "${CMAKE_CURRENT_SOURCE_DIR}/dependencies.sh -d opencv")
    execute_process(COMMAND ${MAKE_CMD} ${INC_DIR} 
                    RESULT_VARIABLE CMD_ERROR
                    OUTPUT_FILE CMD_OUTPUT )
    MESSAGE( STATUS "CMD_ERROR:" ${CMD_ERROR})
endfunction(opencvNotFound)