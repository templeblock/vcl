PROJECT(svd33correctness)

# Status message
MESSAGE(STATUS "Configuring 'svd33correctness'")

SET(SRC
	main.cpp
)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(svd33correctness ${SRC})
SET_TARGET_PROPERTIES(svd33correctness PROPERTIES FOLDER benchmarks)

TARGET_LINK_LIBRARIES(svd33correctness
	vcl_core
	vcl_math
)

IF(CUDA_FOUND AND VCL_CUDA_SUPPORT)
	TARGET_LINK_LIBRARIES(svd33correctness
		vcl_compute_cuda
		vcl_math_cuda
	)
ENDIF()

IF(OPENCL_FOUND AND VCL_OPENCL_SUPPORT)
	TARGET_LINK_LIBRARIES(svd33correctness
		vcl_compute_opencl
		vcl_math_opencl
	)
ENDIF()
