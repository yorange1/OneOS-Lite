if(NOT COMPONENT_OSA_LPC55S69_cm33_core0_INCLUDED)
    
    set(COMPONENT_OSA_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "component_osa component is included.")

    if(CONFIG_USE_middleware_baremetal_LPC55S69_cm33_core0)
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_os_abstraction_bm.c
    )
    elseif(CONFIG_USE_middleware_freertos-kernel_LPC55S69_cm33_core0)
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_os_abstraction_free_rtos.c
    )
    else()
        message(WARNING "please config middleware.baremetal_LPC55S69_cm33_core0 or middleware.freertos-kernel_LPC55S69_cm33_core0 first.")
    endif()


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_common_LPC55S69_cm33_core0)

    include(component_lists_LPC55S69_cm33_core0)

endif()
