if(NOT DRIVER_INPUTMUX_CONNECTIONS_LPC55S69_cm33_core1_INCLUDED)
    
    set(DRIVER_INPUTMUX_CONNECTIONS_LPC55S69_cm33_core1_INCLUDED true CACHE BOOL "driver_inputmux_connections component is included.")


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )

    include(driver_common_LPC55S69_cm33_core1)

endif()
