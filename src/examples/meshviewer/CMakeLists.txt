PROJECT(meshviewer)

# Status message
MESSAGE(STATUS "Configuring 'meshviewer'")

# Find includes in corresponding build directories
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
SET(CMAKE_AUTOMOC ON)

# Find the Qt libraries
FIND_PACKAGE(Qt5 COMPONENTS Core Gui OpenGL Qml Quick)

# Do not automatically link Qt executables to qtmain target on Windows.
# Uses console mode only. Not need to link against qtmain.lib.
cmake_policy(SET CMP0020 OLD)

SET(COMPONENTS_INC
	components/gpusurfacemesh.h
	components/gpuvolumemesh.h
	components/meshstatisticscomponent.h
)

SET(COMPONENTS_SRC
	components/gpusurfacemesh.cpp
	components/gpuvolumemesh.cpp
	components/meshstatisticscomponent.cpp
)

SET(UTIL_INC
	util/frustumhelpers.h
)
SET(UTIL_SRC
	util/frustumhelpers.cpp
)


SET(INC
	meshview.h
	scene.h
)

SET(SRC
	main.cpp
	meshview.cpp
	scene.cpp
)

SET(QML
	main.qml

	CreateBarDialog.qml
	OpenMeshDialog.qml
)

SET(SHADERS_DEBUG
    shaders/debug/plane.vert
    shaders/debug/plane.geom
    shaders/debug/object.frag
	
	shaders/debug/boundinggrid.vert
	shaders/debug/boundinggrid.geom

	shaders/debug/staticcolour.frag
)

SET(SHADERS_FRAMEBUFFER
    shaders/framebuffer/simpleframebuffer.h
    shaders/framebuffer/simpleoitbuffer.h
)
    
SET(SHADERS
	shaders/UniformBuffer.h
	shaders/MarchingCubes.h
	shaders/3DSceneBindings.h
	
	shaders/trimesh.vert
	shaders/trimesh.geom
	shaders/tetramesh.vert
	shaders/tetramesh.geom
	shaders/tetramesh_sphere.geom
	shaders/tetramesh_wireframe.geom
	shaders/mesh.frag
	
	shaders/objectid_tetramesh.vert
	shaders/objectid_tetramesh.geom
	shaders/objectid.frag
)

QT5_ADD_RESOURCES(RESOURCES meshviewer.qrc)

SOURCE_GROUP("qml" FILES ${QML})
SOURCE_GROUP("res" FILES ${RESOURCES})
SOURCE_GROUP("shaders\\debug" FILES ${SHADERS_DEBUG})
SOURCE_GROUP("shaders\\framebuffer" FILES ${SHADERS_FRAMEBUFFER})
SOURCE_GROUP("shaders" FILES ${SHADERS})

SOURCE_GROUP("components" FILES ${COMPONENTS_SRC} ${COMPONENTS_INC})
SOURCE_GROUP("utils" FILES ${UTIL_SRC} ${UTIL_INC})
SOURCE_GROUP("" FILES ${SRC} ${INC})

ADD_EXECUTABLE(meshviewer
	${COMPONENTS_SRC} ${COMPONENTS_INC}
	${UTIL_SRC} ${UTIL_INC}
	${SRC} ${INC}
	${QML} ${RESOURCES}
	${SHADERS} ${SHADERS_DEBUG} ${SHADERS_FRAMEBUFFER}
)
SET_TARGET_PROPERTIES(meshviewer PROPERTIES FOLDER examples)

TARGET_LINK_LIBRARIES(meshviewer
	vcl_components
	vcl_geometry
	vcl_graphics
	Qt5::Core
	Qt5::Gui
	Qt5::OpenGL
	Qt5::Qml
	Qt5::Quick
)
