if(NOT DRIVER_CMP_1_LPC55S69_cm33_core0_INCLUDED)
    
    set(DRIVER_CMP_1_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "driver_cmp_1 component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_cmp.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_common_LPC55S69_cm33_core0)

endif()
