if(NOT COMPONENT_PANIC_LPC55S69_cm33_core0_INCLUDED)
    
    set(COMPONENT_PANIC_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "component_panic component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_component_panic.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_common_LPC55S69_cm33_core0)

    include(utility_debug_console_LPC55S69_cm33_core0)

endif()
