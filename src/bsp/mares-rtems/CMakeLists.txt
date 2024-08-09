######################################################################
#
# CMAKE build recipe for mares-rtems Board Support Package (BSP)
#
######################################################################

add_library(osal_mares-rtems_impl OBJECT
	src/bsp_start.c
	src/bsp_console.c
)

# This definition is needed for the gethostname call
# By defining this, it avoids the need to use the -std=gnu99
#  instead of the preferred -std=c99 GCC switch
target_compile_definitions(osal_public_api INTERFACE
   _BSD_SOURCE
)

# This BSP only works with "rtems" OS layer.
# Confirming this reduces risk of accidental misconfiguration
set_property(TARGET osal_mares-rtems_impl PROPERTY OSAL_EXPECTED_OSTYPE "rtems")
