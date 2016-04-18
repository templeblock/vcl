PROJECT(vcl_graphics_vulkan_test)

SET(VCL_TEST_SRC
	context.cpp
)
SET(VCL_TEST_INC
)

SET(SOURCE
	${VCL_TEST_SRC} ${VCL_TEST_INC}
)

SOURCE_GROUP("" FILES ${VCL_TEST_SRC} ${VCL_TEST_INC})

# Generate library
ADD_EXECUTABLE(vcl_graphics_vulkan_test
	${SOURCE}
)

SET_TARGET_PROPERTIES(vcl_graphics_vulkan_test PROPERTIES FOLDER tests)

TARGET_LINK_LIBRARIES(vcl_graphics_vulkan_test
	vcl_graphics
	debug ${VCL_GTEST_LIBRARY_DEBUG}/gtest.lib
	debug ${VCL_GTEST_LIBRARY_DEBUG}/gtest_main.lib
	optimized ${VCL_GTEST_LIBRARY_RELEASE}/gtest.lib
	optimized ${VCL_GTEST_LIBRARY_RELEASE}/gtest_main.lib
)