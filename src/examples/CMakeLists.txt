# Add GLFW to build graphics examples with simple windows
SUBDIRS(glfw)

SUBDIRS(rtti)

SUBDIRS(meshviewer)
SUBDIRS(simd)

IF(VCL_OPENGL_SUPPORT)
	SUBDIRS(meshviewer)
	SUBDIRS(opengl)
ENDIF()

IF(OPENCL_FOUND AND VCL_OPENCL_SUPPORT)
	SUBDIRS(opencl)
ENDIF()

IF(VULKANSDK_FOUND AND VCL_VULKAN_SUPPORT)
	SUBDIRS(vulkan)
ENDIF()
