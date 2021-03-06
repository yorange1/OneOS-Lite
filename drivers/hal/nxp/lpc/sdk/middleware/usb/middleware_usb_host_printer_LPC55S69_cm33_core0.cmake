if(NOT MIDDLEWARE_USB_HOST_PRINTER_LPC55S69_cm33_core0_INCLUDED)
    
    set(MIDDLEWARE_USB_HOST_PRINTER_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "middleware_usb_host_printer component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/host/class/usb_host_printer.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/host/class
    )


    include(middleware_usb_host_stack_LPC55S69_cm33_core0)

endif()
