if(NOT CMSIS_INCLUDE_CORE_CM33_LPC55S69_cm33_core1_INCLUDED)
    
    set(CMSIS_INCLUDE_CORE_CM33_LPC55S69_cm33_core1_INCLUDED true CACHE BOOL "CMSIS_Include_core_cm33 component is included.")


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )

    include(CMSIS_Include_common_LPC55S69_cm33_core1)

    include(CMSIS_Include_dsp_LPC55S69_cm33_core1)

endif()
