# - Find PhysX
# Find the PhysX includes and library
#
#  PhysX_INCLUDE_DIRS - Where to find PhysX includes
#  PhysX_LIBRARIES    - List of libraries when using PhysX
#  PhysX_FOUND        - True if PhysX was found

IF(PhysX_INCLUDE_DIR)
    SET(PhysX_FIND_QUIETLY TRUE)
ENDIF(PhysX_INCLUDE_DIR)

FIND_PATH(PhysX_INCLUDE_DIR
    NAMES
    PxPhysicsAPI.h
    PATH_SUFFIXES
    include
    Include
    HINTS
    ${PHYSX_HOME}
    $ENV{PHYSX_HOME}
    ${PROJECT_SOURCE_DIR}/dependencies/extra/physx/
    /home/fabi/Documents/PhysX-4.0.0/physx/
)

FIND_PATH(PXShared_INCLUDE_DIR
    NAMES
    Px.h
    PATH_SUFFIXES
    include
    Include
    HINTS
    ${PHYSX_HOME}
    $ENV{PHYSX_HOME}
    ${PROJECT_SOURCE_DIR}/dependencies/extra/pxshared/foundation/
    /home/fabi/Documents/PhysX-4.0.0/pxshared/foundation/
)

IF (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(LIBFOLDERSUFFIX "64")
    SET(PHYSXPREFIX "_64")
ELSE()
    SET(LIBFOLDERSUFFIX "32")
ENDIF()

IF (NOT PhysX_LIBRARY_DIR)
    IF (MSVC)
        IF (MSVC_VERSION EQUAL 1800)
            SET(LIBFOLDER win.x86_${LIBFOLDERSUFFIX}.vc120.mt)
        ELSEIF (MSVC_VERSION EQUAL 1900)
            SET(LIBFOLDER win.x86_${LIBFOLDERSUFFIX}.vc140.mt)
        ELSEIF (MSVC_VERSION EQUAL 1910)
            SET(LIBFOLDER win.x86_${LIBFOLDERSUFFIX}.vc141.mt)
        ELSEIF (MSVC_VERSION GREATER_EQUAL 1920)
            SET(LIBFOLDER win.x86_${LIBFOLDERSUFFIX}.vc142.mt)
        ENDIF()
    ELSE()
        SET(LIBFOLDER linux.clang)
    ENDIF()

    SET(PhysX_LIBRARY_DIR ${PhysX_INCLUDE_DIR}/../lib/${LIBFOLDER})
ENDIF()

FIND_LIBRARY(PhysX_LIBRARY_RELEASE
    NAMES
    PhysX${PHYSXPREFIX}
    HINTS
    ${CMAKE_SOURCE_DIR}/dependencies/lib/${LIBFOLDER}/release/
    /home/fabi/Documents/PhysX-4.0.0/physx/bin/${LIBFOLDER}/release/
)
FIND_LIBRARY(PhysX_LIBRARY_PROFILE
    NAMES
    PhysX${PHYSXPREFIX}
    HINTS
    ${CMAKE_SOURCE_DIR}/dependencies/lib/${LIBFOLDER}/profile/
    /home/fabi/Documents/PhysX-4.0.0/physx/bin/${LIBFOLDER}/profile/
)
FIND_LIBRARY(PhysX_LIBRARY_DEBUG
    NAMES
    PhysX${PHYSXPREFIX}
    HINTS
    ${CMAKE_SOURCE_DIR}/dependencies/lib/${LIBFOLDER}/debug/
    /home/fabi/Documents/PhysX-4.0.0/physx/bin/${LIBFOLDER}/debug/
)

SET(PhysX_LIBRARIES debug ${PhysX_LIBRARY_DEBUG})

IF (PhysX_PROFILE)
    SET(PhysX_LIBRARIES ${PhysX_LIBRARIES} optimized ${PhysX_LIBRARY_PROFILE})
ELSE()
    SET(PhysX_LIBRARIES ${PhysX_LIBRARIES} optimized ${PhysX_LIBRARY_RELEASE})
ENDIF()

SET(NECESSARY_COMPONENTS "")
FOREACH(component ${PhysX_FIND_COMPONENTS})
    FIND_LIBRARY(PhysX_LIBRARY_COMPONENT_${component}_DEBUG
        NAMES
        PhysX${component}_static
        PhysX${component}_static${PHYSXPREFIX}
        PhysX${component}
        PhysX${component}${PHYSXPREFIX}
        ${component}
        ${component}${PHYSXPREFIX}
        HINTS
        ${CMAKE_SOURCE_DIR}/dependencies/lib/${LIBFOLDER}/debug/
        /home/fabi/Documents/PhysX-4.0.0/physx/bin/${LIBFOLDER}/debug/
    )
    IF (PhysX_LIBRARY_COMPONENT_${component}_DEBUG)
        SET(PhysX_LIBRARIES ${PhysX_LIBRARIES} debug "${PhysX_LIBRARY_COMPONENT_${component}_DEBUG}")
    ENDIF()

    FIND_LIBRARY(PhysX_LIBRARY_COMPONENT_${component}_PROFILE
        NAMES
        PhysX${component}_static
        PhysX${component}_static${PHYSXPREFIX}
        PhysX${component}
        PhysX${component}${PHYSXPREFIX}
        ${component}
        ${component}${PHYSXPREFIX}
        HINTS
        ${CMAKE_SOURCE_DIR}/dependencies/lib/${LIBFOLDER}/profile/
        /home/fabi/Documents/PhysX-4.0.0/physx/bin/${LIBFOLDER}/profile/
    )

    FIND_LIBRARY(PhysX_LIBRARY_COMPONENT_${component}_RELEASE
        NAMES
        PhysX${component}_static
        PhysX${component}_static${PHYSXPREFIX}
        PhysX${component}
        PhysX${component}${PHYSXPREFIX}
        ${component}
        ${component}${PHYSXPREFIX}
        HINTS
        ${CMAKE_SOURCE_DIR}/dependencies/lib/${LIBFOLDER}/release/
        /home/fabi/Documents/PhysX-4.0.0/physx/bin/${LIBFOLDER}/release/
    )

    MARK_AS_ADVANCED(PhysX_LIBRARY_COMPONENT_${component}_DEBUG PhysX_LIBRARY_COMPONENT_${component}_PROFILE PhysX_LIBRARY_COMPONENT_${component}_RELEASE)

    IF (PhysX_PROFILE)
        SET(TARGET "PhysX_LIBRARY_COMPONENT_${component}_PROFILE")
    ELSE()
        SET(TARGET "PhysX_LIBRARY_COMPONENT_${component}_RELEASE")
    ENDIF()

    IF (${TARGET})
        SET(PhysX_LIBRARIES ${PhysX_LIBRARIES} optimized "${${TARGET}}")
    ENDIF()

    SET(NECESSARY_COMPONENTS ${NECESSARY_COMPONENTS} PhysX_LIBRARY_COMPONENT_${component}_DEBUG ${TARGET})
ENDFOREACH()

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(PhysX DEFAULT_MSG PhysX_INCLUDE_DIR PXShared_INCLUDE_DIR PhysX_LIBRARY_DEBUG PhysX_LIBRARY_RELEASE ${NECESSARY_COMPONENTS})
MARK_AS_ADVANCED(PhysX_INCLUDE_DIR PXShared_INCLUDE_DIR PhysX_LIBRARY_DIR PhysX_LIBRARY_DEBUG PhysX_LIBRARY_RELEASE PhysX_LIBRARY_PROFILE)

IF(PhysX_INCLUDE_DIR AND PhysX_LIBRARIES)
    SET(PhysX_FOUND TRUE)
    SET(PhysX_INCLUDE_DIRS ${PhysX_INCLUDE_DIR} ${PXShared_INCLUDE_DIR})
ENDIF(PhysX_INCLUDE_DIR AND PhysX_LIBRARIES)
