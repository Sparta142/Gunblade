# [vcpkg] Find compiled dependencies
find_package(nlohmann_json CONFIG REQUIRED)
find_package(libtins       CONFIG REQUIRED)
find_package(spdlog        CONFIG REQUIRED)
find_package(ZLIB                 REQUIRED)

# [vcpkg] Find header-only dependencies
find_path(GZIP_HPP_INCLUDE_DIR
    NAMES compress.hpp decompress.hpp
    PATH_SUFFIXES gzip/
    REQUIRED
)
find_path(PCAP_INCLUDE_DIR
    pcap/pcap.h
    REQUIRED
)

add_executable(machinist
    ffxiv/decoder.cpp
    ffxiv/stream_handler.cpp
    ffxiv/structs.cpp
    main.cpp
    tcp_table.cpp
    utils.cpp

    ffxiv/decoder.h
    ffxiv/stream_handler.h
    ffxiv/structs.h
    tcp_table.h
    utils.h
)

# C++20, no compiler extensions, and position-independent code
target_compile_features(machinist PUBLIC cxx_std_20)
set_target_properties(machinist PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_link_libraries(machinist PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    tins
    ZLIB::ZLIB
)

target_include_directories(machinist PRIVATE
    ${GZIP_HPP_INCLUDE_DIR}
    ${PCAP_INCLUDE_DIR}
)

# Necessary for Tins to export its API correctly - it's not done by vcpkg
target_compile_definitions(machinist PRIVATE
    TINS_STATIC=1
)

if(MSVC)
    target_link_libraries(machinist PRIVATE
        Ws2_32.lib   #
        Iphlpapi.lib
        psapi.lib
    )

    # Fix MSVC/Windows API annoyances
    target_compile_definitions(machinist PRIVATE
        WIN32_LEAN_AND_MEAN=1
        VC_EXTRALEAN=1
        NOMINMAX=1
        _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING=1
    )

    # Enable warnings
    target_compile_options(machinist PRIVATE /W4)

    # Use static Windows CRT
    set_target_properties(machinist PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
else()
    # Enable warnings
    target_compile_options(machinist PRIVATE -Wall -Wextra -pedantic)
endif()
