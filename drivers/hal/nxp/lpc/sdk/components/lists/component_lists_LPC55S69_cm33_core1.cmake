if(NOT COMPONENT_LISTS_LPC55S69_cm33_core1_INCLUDED)
    
    set(COMPONENT_LISTS_LPC55S69_cm33_core1_INCLUDED true CACHE BOOL "component_lists component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_component_generic_list.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_common_LPC55S69_cm33_core1)

endif()
