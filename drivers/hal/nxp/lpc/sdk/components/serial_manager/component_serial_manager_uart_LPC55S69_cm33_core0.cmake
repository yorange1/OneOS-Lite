if(NOT COMPONENT_SERIAL_MANAGER_UART_LPC55S69_cm33_core0_INCLUDED)
    
    set(COMPONENT_SERIAL_MANAGER_UART_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "component_serial_manager_uart component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_component_serial_port_uart.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    #OR Logic component
    if(CONFIG_USE_driver_flexcomm_usart_LPC55S69_cm33_core0)
         include(driver_flexcomm_usart_LPC55S69_cm33_core0)
    endif()
    if(NOT (CONFIG_USE_driver_flexcomm_usart_LPC55S69_cm33_core0))
        message(WARNING "Since driver_flexcomm_usart_LPC55S69_cm33_core0 is not included at first or config in config.cmake file, use driver_uart_LPC55S69_cm33_core0 by default.")
        include(driver_uart_LPC55S69_cm33_core0)
    endif()

    include(component_usart_adapter_LPC55S69_cm33_core0)

    include(component_serial_manager_LPC55S69_cm33_core0)

endif()
