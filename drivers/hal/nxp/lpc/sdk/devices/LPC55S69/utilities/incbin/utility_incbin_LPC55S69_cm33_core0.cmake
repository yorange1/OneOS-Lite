if(NOT UTILITY_INCBIN_LPC55S69_cm33_core0_INCLUDED)
    
    set(UTILITY_INCBIN_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "utility_incbin component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_incbin.S
    )


endif()
