add_executable(switchcontrollertest switchcontrollertest.cpp)
target_link_libraries(switchcontrollertest PRIVATE pagercore Qt6::Test)
add_test(NAME switching COMMAND switchcontrollertest)

add_executable(desktopintegrationtest desktopintegrationtest.cpp)
target_link_libraries(desktopintegrationtest PRIVATE pagercore Qt6::Test)
find_program(DBUS_RUN_SESSION dbus-run-session REQUIRED)
add_test(NAME desktop-integration
    COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:desktopintegrationtest>)
set_tests_properties(desktop-integration PROPERTIES TIMEOUT 20)

add_executable(workspaceprobe workspaceprobe.cpp)
target_link_libraries(workspaceprobe PRIVATE pagercore)

find_package(Qt6 REQUIRED COMPONENTS QuickTest)
add_executable(qmltests qmltests.cpp)
target_link_libraries(qmltests PRIVATE Qt6::QuickTest)
target_compile_definitions(qmltests PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml")
add_test(NAME grid COMMAND qmltests -input ${CMAKE_CURRENT_SOURCE_DIR}/qml/tst_grid.qml)
add_test(NAME popup COMMAND qmltests -input ${CMAKE_CURRENT_SOURCE_DIR}/qml/tst_popup.qml)
set_tests_properties(grid popup PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_CONTROLS_STYLE=Basic"
    TIMEOUT 20)
