if(NOT DRIVER_RESET_LPC55S69_cm33_core0_INCLUDED)
    
    set(DRIVER_RESET_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "driver_reset component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_reset.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_common_LPC55S69_cm33_core0)

endif()
