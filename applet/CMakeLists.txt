plasma_add_applet(local.twodpager
    QML_SOURCES qml/main.qml qml/PagerGrid.qml qml/PagerPopup.qml qml/PanelButton.qml
    CPP_SOURCES pagerbackend.h
    GENERATE_APPLET_CLASS
)
ecm_target_qml_sources(local.twodpager PATH icons SOURCES qml/icons/local-twodpager.svg)
target_link_libraries(local.twodpager PRIVATE pagercore Qt6::Quick)
target_include_directories(local.twodpager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

install(FILES qml/icons/local-twodpager.svg
    DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps)
