######################################################################
#
# CMAKE build recipe for NOS OSAL implementation
#
######################################################################

# This CMake script generates targets specific to the NOS implementation
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../posix/inc)

# The basic set of files which are always built
set(NOS_BASE_SRCLIST
    ../posix/src/os-impl-console.c
    ../posix/src/os-impl-dirs.c
    ../posix/src/os-impl-errors.c
    ../posix/src/os-impl-files.c
    ../posix/src/os-impl-filesys.c
    ../posix/src/os-impl-heap.c
    ../posix/src/os-impl-idmap.c
    ../posix/src/os-impl-mutex.c
    src/NOS-time.c
    src/os-impl-binsem.c
    src/os-impl-bsd-select.c
    src/os-impl-common.c
    src/os-impl-countsem.c
    src/os-impl-nos-gettime.c
    src/os-impl-queues.c
    src/os-impl-tasks.c
    src/os-impl-timebase.c
)

# The FPU and interrupt modules are deprecated.
# If the "OMIT_DEPRECATED" switch is set, then these are not built.
if (NOT OMIT_DEPRECATED)
    list(APPEND NOS_BASE_SRCLIST
        ../posix/src/os-impl-fpu.c
        ../posix/src/os-impl-interrupts.c
    )
endif (NOT OMIT_DEPRECATED)



# Use portable blocks for basic I/O
set(NOS_IMPL_SRCLIST
    ../portable/os-impl-console-bsp.c
    ../portable/os-impl-posix-io.c
    ../portable/os-impl-posix-files.c
    ../portable/os-impl-posix-dirs.c
)

if (OSAL_CONFIG_INCLUDE_SHELL)
    list(APPEND NOS_IMPL_SRCLIST
       ../posix/src/os-impl-shell.c
    )
else ()
    list(APPEND NOS_IMPL_SRCLIST
       ../portable/os-impl-no-shell.c
    )
endif ()

# If some form of module loading is configured,
# then build the module loader
if (OSAL_CONFIG_INCLUDE_DYNAMIC_LOADER)
    list(APPEND NOS_IMPL_SRCLIST
        ../posix/src/os-impl-loader.c
        ../portable/os-impl-posix-dl-loader.c
        ../portable/os-impl-posix-dl-symtab.c
    )
else ()
    list(APPEND NOS_IMPL_SRCLIST
        ../posix/src/os-impl-no-module.c
        ../portable/os-impl-no-loader.c
        ../portable/os-impl-no-symtab.c
    )
endif ()

if (OSAL_CONFIG_INCLUDE_NETWORK)
    list(APPEND NOS_IMPL_SRCLIST
        ../portable/os-impl-bsd-sockets.c   # Use BSD socket layer implementation
        ../portable/os-impl-posix-network.c # Use POSIX-defined hostname/id implementation
    )
else()
    list(APPEND NOS_IMPL_SRCLIST
        ../portable/os-impl-no-network.c    # non-implemented versions of all network APIs
        ../portable/os-impl-no-sockets.c    # non-implemented versions of all socket APIs
    )
endif ()

# Defines an OBJECT target named "osal_nos_impl" with selected source files
add_library(osal_nos_impl OBJECT
    ${NOS_BASE_SRCLIST}
    ${NOS_IMPL_SRCLIST}
)
