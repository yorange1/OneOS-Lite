if(NOT DRIVER_POWER_S_LPC55S69_cm33_core0_INCLUDED)
    
    set(DRIVER_POWER_S_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "driver_power_s component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/drivers/fsl_power.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/drivers
    )


    include(driver_common_LPC55S69_cm33_core0)

endif()