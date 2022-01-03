project(gvfspanel)

OPTION(STORE_ENCRYPTED_PASSWORDS "Encrypt stored passwords." ON)
OPTION(USE_SECRET_STORAGE "Store passwords in system secret storage." ON)

message(STATUS "GVFS panel plugin options:")
message(STATUS "  \"Encrypt stored passwords\" is ${STORE_ENCRYPTED_PASSWORDS};")
message(STATUS "  \"Store passwords in system secret storage\" is ${STORE_ENCRYPTED_PASSWORDS}.")

set(HEADERS
    src/Configuration.h
    src/dialogs.h
    src/glibmmconf.h
    src/GvfsService.h
    src/GvfsServiceException.h
    src/GvfsServiceMonitor.h
    src/JobUnitQueue.h
    src/KeyBarTitlesHelper.h
    src/LngStringIDs.h
    src/MountPoint.h
    src/MountPointStorage.h
    src/Plugin.h
    src/RegistryStorage.h
    src/TextFormatter.h
    src/UiCallbacks.h
)

set(SOURCES
    src/Configuration.cpp
    src/dialogs.cpp
    src/GvfsService.cpp
    src/GvfsServiceMonitor.cpp
    src/KeyBarTitlesHelper.cpp
    src/MountPoint.cpp
    src/MountPointStorage.cpp
    src/Plugin.cpp
    src/RegistryStorage.cpp
    src/TextFormatter.cpp
    src/UiCallbacks.cpp
    src/PluginMain.cpp
)

find_package(PkgConfig REQUIRED MODULE)

pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules(UUID REQUIRED uuid)
pkg_check_modules(SECRET libsecret-1)

find_package(OpenSSL MODULE)

if(OPENSSL_FOUND AND STORE_ENCRYPTED_PASSWORDS)
    list(APPEND HEADERS src/Crypto.h)
    list(APPEND SOURCES src/Crypto.cpp)
else(OPENSSL_FOUND AND STORE_ENCRYPTED_PASSWORDS)
    if(STORE_ENCRYPTED_PASSWORDS)
        message(FATAL_ERROR "OpenSSL not found, password encryption not possible.")
    endif(STORE_ENCRYPTED_PASSWORDS)
endif(OPENSSL_FOUND AND STORE_ENCRYPTED_PASSWORDS)

if(SECRET_FOUND AND USE_SECRET_STORAGE)
    list(APPEND HEADERS src/SecretServiceStorage.h)
    list(APPEND SOURCES src/SecretServiceStorage.cpp)
else(SECRET_FOUND AND USE_SECRET_STORAGE)
    if(USE_SECRET_STORAGE)
        message(FATAL_ERROR "libsecret not found, secret system storage not accessable.")
    endif(USE_SECRET_STORAGE)
endif(SECRET_FOUND AND USE_SECRET_STORAGE)

add_library(${PROJECT_NAME} MODULE ${HEADERS} ${SOURCES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    WINPORT_DIRECT
    UNICODE
    FAR_DONT_USE_INTERNALS
    PLUGIN_NAME="${PROJECT_NAME}"
)

if(NOT DEFINED FAR2L_PLUGINS_INCLUDE_DIRS)
    set(FAR2L_PLUGINS_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/far2l/far2sdk
        ${CMAKE_SOURCE_DIR}/WinPort
    )
endif(NOT DEFINED FAR2L_PLUGINS_INCLUDE_DIRS)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${FAR2L_PLUGINS_INCLUDE_DIRS}
    ${GTKMM_INCLUDE_DIRS}
    ${UUID_INCLUDE_DIRS}
)

if(NOT DEFINED FAR2L_PLUGINS_LIBRARIES)
    set(FAR2L_PLUGINS_LIBRARIES utils)
endif(NOT DEFINED FAR2L_PLUGINS_LIBRARIES)

target_link_libraries(${PROJECT_NAME}
    ${FAR2L_PLUGINS_LIBRARIES}
    ${GTKMM_LIBRARIES}
    ${UUID_LIBRARIES}
)

if(OPENSSL_FOUND AND STORE_ENCRYPTED_PASSWORDS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENSSL)
    target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${OPENSSL_CRYPTO_LIBRARIES})
endif(OPENSSL_FOUND AND STORE_ENCRYPTED_PASSWORDS)

if(SECRET_FOUND AND USE_SECRET_STORAGE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_SECRET_STORAGE)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SECRET_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${SECRET_LIBRARIES})
endif(SECRET_FOUND AND USE_SECRET_STORAGE)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${INSTALL_DIR}/Plugins/${PROJECT_NAME}/plug"
    PREFIX ""
    SUFFIX ".far-plug-wide")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/configs
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/configs "${INSTALL_DIR}/Plugins/${PROJECT_NAME}"
)

message(STATUS "GVFS panel plugin configured.")
