if(NOT COMPONENT_MFLASH_LPC55XXX_LPC55S69_cm33_core0_INCLUDED)
    
    set(COMPONENT_MFLASH_LPC55XXX_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "component_mflash_lpc55xxx component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/mflash_drv.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(component_mflash_common_LPC55S69_cm33_core0)

    include(driver_iap1_LPC55S69_cm33_core0)

endif()
