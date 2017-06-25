project(gvfspanel)

set(HEADERS
    src/Configuration.h
    src/dialogs.h
    src/GvfsService.h
    src/GvfsServiceException.h
    src/GvfsServiceMonitor.h
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

if(OPENSSL_FOUND)
    list(APPEND HEADERS src/Crypto.h)
    list(APPEND SOURCES src/Crypto.cpp)
endif(OPENSSL_FOUND)

if(SECRET_FOUND)
    list(APPEND HEADERS src/SecretServiceStorage.h)
    list(APPEND SOURCES src/SecretServiceStorage.cpp)
endif(SECRET_FOUND)

add_library(${PROJECT_NAME} MODULE ${HEADERS} ${SOURCES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    -DWINPORT_DIRECT
    -DUNICODE
    -DFAR_DONT_USE_INTERNALS
    -DPLUGIN_NAME="${PROJECT_NAME}"
)

target_include_directories(${PROJECT_NAME} PRIVATE ./inc)
target_include_directories(${PROJECT_NAME} PRIVATE ../far2l)
target_include_directories(${PROJECT_NAME} PRIVATE ../far2l/Include)
target_include_directories(${PROJECT_NAME} PRIVATE ../WinPort)
target_include_directories(${PROJECT_NAME} PRIVATE ${GTKMM_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${UUID_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
    utils
    ${GTKMM_LIBRARIES}
    ${UUID_LIBRARIES}
)

if(OPENSSL_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_OPENSSL)
    target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${OPENSSL_CRYPTO_LIBRARY})
endif(OPENSSL_FOUND)

if(SECRET_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_SECRET_STORAGE)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${SECRET_INCLUDE_DIRS}
    )
    target_link_libraries(${PROJECT_NAME}
        ${SECRET_LIBRARIES}
    )
endif(SECRET_FOUND)

set_target_properties(${PROJECT_NAME}
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${INSTALL_DIR}/Plugins/${PROJECT_NAME}/plug"
    PREFIX ""
    SUFFIX ".far-plug-wide")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/configs
   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/configs "${INSTALL_DIR}/Plugins/${PROJECT_NAME}"
)
