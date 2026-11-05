plasma_add_applet(local.twodpager
    QML_SOURCES qml/main.qml qml/PagerGrid.qml
    CPP_SOURCES pagerbackend.h
    GENERATE_APPLET_CLASS
)
target_link_libraries(local.twodpager PRIVATE pagercore Qt6::Quick)
target_include_directories(local.twodpager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
