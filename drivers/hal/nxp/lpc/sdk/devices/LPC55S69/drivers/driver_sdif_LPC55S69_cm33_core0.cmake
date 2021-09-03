if(NOT DRIVER_SDIF_LPC55S69_cm33_core0_INCLUDED)
    
    set(DRIVER_SDIF_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "driver_sdif component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_sdif.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_common_LPC55S69_cm33_core0)

endif()
