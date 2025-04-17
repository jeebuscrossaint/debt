cmake_minimum_required(VERSION 3.10)
project(debt VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define source files
set(SOURCE_FILES
    debt.c
)

set(HEADER_FILES
    debt.h
)

# Add executable
add_executable(debt ${SOURCE_FILES} ${HEADER_FILES})

# Add platform-specific compile definitions
if(WIN32)
    target_compile_definitions(debt PRIVATE _WIN32)
    # Windows may need additional libraries
    target_link_libraries(debt PRIVATE wsock32 ws2_32)
endif()

# Set optimization flags based on build type
if(CMAKE_BUILD_TYPE MATCHES "Release")
    if(MSVC)
        # MSVC optimization flags
        target_compile_options(debt PRIVATE /O2 /Ot /Oi /GL /Gy /Qpar)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
    else()
        # GCC/Clang optimization flags
        target_compile_options(debt PRIVATE -O3 -march=native -flto -fomit-frame-pointer -pipe -ffast-math -DNDEBUG)
    endif()
endif()

# Installation rules
include(GNUInstallDirs)

if(WIN32)
    # On Windows, we copy the executable as $ and >
    install(TARGETS debt
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # Create a custom target to copy the executable with $ and > names
    add_custom_target(symlinks ALL
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:debt>
            ${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:debt>.dollar
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:debt>
            ${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:debt>.gt
        DEPENDS debt
    )

    # Install the copies
    install(
        FILES
            ${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:debt>.dollar
            ${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:debt>.gt
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        RENAME $<TARGET_FILE_NAME:debt>.dollar $
        RENAME $<TARGET_FILE_NAME:debt>.gt >
    )
else()
    # On Unix, we create symlinks named $ and >
    install(TARGETS debt
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # Create symlinks at install time
    install(CODE "
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            debt
            \$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/\$
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            debt
            \$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/\\>
        )
    ")
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "debt")
set(CPACK_PACKAGE_VENDOR "debt developers")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Strips leading $ or > from commands for easy copy-pasting")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Platform-specific package settings
if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_PACKAGE_NAME "debt")
    set(CPACK_NSIS_MODIFY_PATH ON)
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ;DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "debt maintainer")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_RPM_PACKAGE_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
endif()

include(CPack)

# Print configuration info
message(STATUS "Configuring debt v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(WIN32)
    message(STATUS "Platform: Windows")
elseif(APPLE)
    message(STATUS "Platform: macOS")
else()
    message(STATUS "Platform: Unix/Linux")
endif()
