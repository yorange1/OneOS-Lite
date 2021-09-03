if(NOT DRIVER_FLEXCOMM_I2C_LPC55S69_cm33_core0_INCLUDED)
    
    set(DRIVER_FLEXCOMM_I2C_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "driver_flexcomm_i2c component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_i2c.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_flexcomm_LPC55S69_cm33_core0)

    include(driver_common_LPC55S69_cm33_core0)

endif()
