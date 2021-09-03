if(NOT DRIVER_INPUTMUX_LPC55S69_cm33_core0_INCLUDED)
    
    set(DRIVER_INPUTMUX_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "driver_inputmux component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_inputmux.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_common_LPC55S69_cm33_core0)

    include(driver_inputmux_connections_LPC55S69_cm33_core0)

endif()
