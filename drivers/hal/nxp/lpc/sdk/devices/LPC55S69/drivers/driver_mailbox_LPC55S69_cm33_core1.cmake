if(NOT DRIVER_MAILBOX_LPC55S69_cm33_core1_INCLUDED)
    
    set(DRIVER_MAILBOX_LPC55S69_cm33_core1_INCLUDED true CACHE BOOL "driver_mailbox component is included.")


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )

    include(driver_common_LPC55S69_cm33_core1)

endif()
