add_library(GL STATIC
	GraphicsObjects.cpp
	DrawingFunctions.cpp
	Renderer.cpp
	NullContext.cpp
)
target_include_directories(GL PRIVATE ${MAIN_INCLUDE_DIR})

target_link_libraries(GL PRIVATE
	GLEW::GLEW
	fmt::fmt
)

if(APPLE)
	target_sources(GL PRIVATE CocoaContext.mm)
	target_link_libraries(GL PRIVATE
		"-framework Foundation"
		"-framework Cocoa"
	)
endif()

if(WIN32)
	target_sources(GL PRIVATE WGLContext.cpp)
endif()
