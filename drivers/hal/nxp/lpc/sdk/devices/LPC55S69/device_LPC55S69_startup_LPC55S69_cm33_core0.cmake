if(NOT DEVICE_LPC55S69_STARTUP_LPC55S69_cm33_core0_INCLUDED)
    
    set(DEVICE_LPC55S69_STARTUP_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "device_LPC55S69_startup component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/gcc/startup_LPC55S69_cm33_core0.S
    )


    include(device_LPC55S69_system_LPC55S69_cm33_core0)

endif()
