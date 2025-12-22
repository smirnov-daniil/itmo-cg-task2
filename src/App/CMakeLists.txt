set(SRCS
    Camera.cpp
    Camera.h
    Entity.cpp
    Entity.h
    main.cpp
    ModelEntity.cpp
    ModelEntity.h
    OpenGLContext.cpp
    OpenGLContext.h
    SceneGraph.cpp
    SceneGraph.h
    SceneRenderer.cpp
    SceneRenderer.h
    SkyboxEntity.cpp
    SkyboxEntity.h
    Window.cpp
    Window.h

    Shaders/model.fs
    Shaders/model.vs
    Shaders/skybox.fs
    Shaders/skybox.vs

    Models/sponza.glb
)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

qt_add_big_resources(RES resources.qrc)

add_executable(demo-app ${SRCS} ${RES})

target_link_libraries(demo-app
    PRIVATE
        Qt5::Widgets
        FGL::Base
        thirdparty::tinygltf
)