if(NOT DEVICE_LPC55S69_CMSIS_LPC55S69_cm33_core1_INCLUDED)
    
    set(DEVICE_LPC55S69_CMSIS_LPC55S69_cm33_core1_INCLUDED true CACHE BOOL "device_LPC55S69_CMSIS component is included.")


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )

    include(CMSIS_Include_core_cm33_LPC55S69_cm33_core1)

endif()
