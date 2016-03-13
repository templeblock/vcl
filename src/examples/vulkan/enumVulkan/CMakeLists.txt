PROJECT(enumvk)

SET(SRC
	main.cpp
)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(enumvk ${SRC})
SET_TARGET_PROPERTIES(enumvk PROPERTIES FOLDER examples/vulkan)

TARGET_LINK_LIBRARIES(enumvk
	vcl_graphics
)
