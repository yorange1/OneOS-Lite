if(NOT COMPONENT_MRT_ADAPTER_LPC55S69_cm33_core0_INCLUDED)
    
    set(COMPONENT_MRT_ADAPTER_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "component_mrt_adapter component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_adapter_mrt.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_common_LPC55S69_cm33_core0)

    include(driver_mrt_LPC55S69_cm33_core0)

endif()
