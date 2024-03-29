cmake_minimum_required(VERSION 3.18)
include(pico_sdk_import.cmake)
project(Farnsworth C CXX ASM)
set(PICO_BOARD pico_w)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(Farnsworth 
		main.c
		iniparser.c
		hw_config.c
		device_sdfs.c
		dictionary.c
		rotenc.c
)

pico_enable_stdio_usb(Farnsworth 1)

pico_add_extra_outputs(Farnsworth)


target_include_directories(Farnsworth PRIVATE ${CMAKE_CURRENT_LIST_DIR} )
target_link_libraries(Farnsworth
	camera
	graphics
        pico_cyw43_arch_lwip_threadsafe_background
	pico_lwip_mbedtls
	pico_stdlib
	hardware_i2c
	hardware_gpio
	hardware_spi
	pico_multicore	
	FatFs_SPI
	hardware_adc
	hardware_pwm
)

add_subdirectory(FatFs_SPI build)
add_subdirectory(camera)
add_subdirectory(graphics)

add_compile_definitions(PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE=1)
add_compile_definitions(PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE=0)

