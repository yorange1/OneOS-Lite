if(NOT DRIVER_FT6X06_LPC55S69_cm33_core0_INCLUDED)
    
    set(DRIVER_FT6X06_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "driver_ft6x06 component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_ft6x06.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(CMSIS_Driver_Include_I2C_LPC55S69_cm33_core0)

endif()
