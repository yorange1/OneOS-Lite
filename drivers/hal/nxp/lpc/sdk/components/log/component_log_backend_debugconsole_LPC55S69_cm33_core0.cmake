if(NOT COMPONENT_LOG_BACKEND_DEBUGCONSOLE_LPC55S69_cm33_core0_INCLUDED)
    
    set(COMPONENT_LOG_BACKEND_DEBUGCONSOLE_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "component_log_backend_debugconsole component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_component_log_backend_debugconsole.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_common_LPC55S69_cm33_core0)

    include(component_log_LPC55S69_cm33_core0)

    include(utility_debug_console_LPC55S69_cm33_core0)

endif()
