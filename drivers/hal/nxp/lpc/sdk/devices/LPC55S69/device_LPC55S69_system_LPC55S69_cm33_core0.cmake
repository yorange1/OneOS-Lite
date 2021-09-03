if(NOT DEVICE_LPC55S69_SYSTEM_LPC55S69_cm33_core0_INCLUDED)
    
    set(DEVICE_LPC55S69_SYSTEM_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "device_LPC55S69_system component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/system_LPC55S69_cm33_core0.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(device_LPC55S69_CMSIS_LPC55S69_cm33_core0)

endif()
