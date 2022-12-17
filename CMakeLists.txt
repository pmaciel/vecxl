cmake_minimum_required(VERSION 3.18)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(vecxl VERSION 0.0.1 LANGUAGES C CXX)

add_compile_options(-Wall -Wextra -pedantic -Werror)

# requirements
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_gfx REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libusb REQUIRED libusb-1.0)

# library: vecx
add_library(vecx SHARED
    dep/vecx/e6809.c
    dep/vecx/e8910.c
    dep/vecx/osint.c
    dep/vecx/vecx.c)
target_link_libraries(vecx SDL2::Main SDL2::Image SDL2::GFX)

# library: helios_dac
add_library(helios_dac SHARED
    dep/helios_dac/sdk/HeliosDacAPI.cpp
    dep/helios_dac/sdk/HeliosDac.cpp)
set_target_properties(helios_dac PROPERTIES
    CXX_STANDARD 17
    COMPILE_OPTIONS ${libusb_CFLAGS}
    LINK_LIBRARIES ${libusb_LINK_LIBRARIES})

# executable: vecxl
add_executable(vecxl src/vecxl.cc)
target_link_libraries(vecxl vecx helios_dac)

# data
foreach(rom fastrom.dat rom.dat)
    file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/dep/vecx/${rom} ${rom} SYMBOLIC)
endforeach()

