set(NPCAP_VERSION 1.31)
set(NPCAP_SDK_VERSION 1.07)

vcpkg_download_distfile(ARCHIVE
    URLS "https://nmap.org/npcap/dist/npcap-sdk-${NPCAP_SDK_VERSION}.zip"
    FILENAME "npcap-sdk-${NPCAP_SDK_VERSION}.zip"
    SHA512 6e9a77552f5cdd66d4c8ffc0ad8c354eb38ea7407a6e8c8ab19efe88f824164e41e52345b0b7e6b73beff973c89c2eafb68fb90059c090c6ccf2ef06043cc642
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE ${ARCHIVE}
    NO_REMOVE_ONE_LEVEL
)

# x64 or x86?
if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
    set(NPCAP_LIB_DIRECTORY "${SOURCE_PATH}/Lib/x64")
elseif(VCPKG_TARGET_ARCHITECTURE STREQUAL "x86")
    set(NPCAP_LIB_DIRECTORY "${SOURCE_PATH}/Lib")
endif()

set(NPCAP_LIBRARIES
    ${NPCAP_LIB_DIRECTORY}/Packet.lib
    ${NPCAP_LIB_DIRECTORY}/wpcap.lib
)

# Copy binaries into release lib directory
if (NOT VCPKG_BUILD_TYPE OR VCPKG_BUILD_TYPE STREQUAL "release")
    file(COPY
        ${NPCAP_LIBRARIES}
        DESTINATION ${CURRENT_PACKAGES_DIR}/lib
    )
endif()

# Copy binaries into debug lib directory
if(NOT VCPKG_BUILD_TYPE OR VCPKG_BUILD_TYPE STREQUAL "debug")
    file(COPY
        ${NPCAP_LIBRARIES}
        DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib
    )
endif()

# Copy include directory
file(RENAME ${SOURCE_PATH}/Include ${SOURCE_PATH}/include)
file(COPY
    ${SOURCE_PATH}/include
    DESTINATION ${CURRENT_PACKAGES_DIR}
)

# Handle copyright
file(DOWNLOAD
    "https://raw.githubusercontent.com/nmap/npcap/v${NPCAP_VERSION}/LICENSE"
    ${CURRENT_PACKAGES_DIR}/share/npcap-sdk/copyright
)
