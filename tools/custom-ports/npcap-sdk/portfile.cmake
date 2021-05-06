# Common Ambient Variables:
#   CURRENT_BUILDTREES_DIR    = ${VCPKG_ROOT_DIR}\buildtrees\${PORT}
#   CURRENT_PACKAGES_DIR      = ${VCPKG_ROOT_DIR}\packages\${PORT}_${TARGET_TRIPLET}
#   CURRENT_PORT_DIR          = ${VCPKG_ROOT_DIR}\ports\${PORT}
#   CURRENT_INSTALLED_DIR     = ${VCPKG_ROOT_DIR}\installed\${TRIPLET}
#   DOWNLOADS                 = ${VCPKG_ROOT_DIR}\downloads
#   PORT                      = current port name (zlib, etc)
#   TARGET_TRIPLET            = current triplet (x86-windows, x64-windows-static, etc)
#   VCPKG_CRT_LINKAGE         = C runtime linkage type (static, dynamic)
#   VCPKG_LIBRARY_LINKAGE     = target library linkage type (static, dynamic)
#   VCPKG_ROOT_DIR            = <C:\path\to\current\vcpkg>
#   VCPKG_TARGET_ARCHITECTURE = target architecture (x64, x86, arm)
#   VCPKG_TOOLCHAIN           = ON OFF
#   TRIPLET_SYSTEM_ARCH       = arm x86 x64
#   BUILD_ARCH                = "Win32" "x64" "ARM"
#   MSBUILD_PLATFORM          = "Win32"/"x64"/${TRIPLET_SYSTEM_ARCH}
#   DEBUG_CONFIG              = "Debug Static" "Debug Dll"
#   RELEASE_CONFIG            = "Release Static"" "Release DLL"
#   VCPKG_TARGET_IS_WINDOWS
#   VCPKG_TARGET_IS_UWP
#   VCPKG_TARGET_IS_LINUX
#   VCPKG_TARGET_IS_OSX
#   VCPKG_TARGET_IS_FREEBSD
#   VCPKG_TARGET_IS_ANDROID
#   VCPKG_TARGET_IS_MINGW
#   VCPKG_TARGET_EXECUTABLE_SUFFIX
#   VCPKG_TARGET_STATIC_LIBRARY_SUFFIX
#   VCPKG_TARGET_SHARED_LIBRARY_SUFFIX
#
# 	See additional helpful variables in /docs/maintainers/vcpkg_common_definitions.md

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
