cmake_minimum_required(VERSION 3.13)

if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
    set(WIFI_SSID $ENV{WIFI_SSID})
    message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
endif()

if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
    set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
    message("Using WIFI_PASSWORD from environment")
endif()

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

# We also need PICO EXTRAS
include(pico_extras_import.cmake)

project(picobutton)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(picobutton picobutton.cpp)

target_compile_definitions(picobutton PRIVATE
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
)

target_include_directories(picobutton PRIVATE ${CMAKE_CURRENT_LIST_DIR} )

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(
    picobutton
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
)

# enable usb output, disable uart output
pico_enable_stdio_usb(picobutton 1)
pico_enable_stdio_uart(picobutton 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(picobutton)
