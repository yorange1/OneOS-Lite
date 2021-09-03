if(NOT MIDDLEWARE_SDMMC_OSA_FREERTOS_LPC55S69_cm33_core0_INCLUDED)
    
    set(MIDDLEWARE_SDMMC_OSA_FREERTOS_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "middleware_sdmmc_osa_freertos component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/osa/fsl_sdmmc_osa.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/osa
    )


    include(component_osa_free_rtos_LPC55S69_cm33_core0)

endif()
