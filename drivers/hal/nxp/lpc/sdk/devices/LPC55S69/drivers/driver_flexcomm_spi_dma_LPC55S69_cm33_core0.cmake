if(NOT DRIVER_FLEXCOMM_SPI_DMA_LPC55S69_cm33_core0_INCLUDED)
    
    set(DRIVER_FLEXCOMM_SPI_DMA_LPC55S69_cm33_core0_INCLUDED true CACHE BOOL "driver_flexcomm_spi_dma component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_spi_dma.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_flexcomm_LPC55S69_cm33_core0)

    include(driver_flexcomm_spi_LPC55S69_cm33_core0)

    include(driver_lpc_dma_LPC55S69_cm33_core0)

endif()
