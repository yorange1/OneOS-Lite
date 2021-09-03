if(NOT MIDDLEWARE_SDMMC_COMMON_LPC55S69_cm33_core0_INCLUDED)
    
    set(MIDDLEWARE_SDMMC_COMMON_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "middleware_sdmmc_common component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/common/fsl_sdmmc_common.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/common
    )


endif()
