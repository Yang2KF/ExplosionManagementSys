file(GLOB UI_SRCS "ui/*.cpp")
file(GLOB UI_HEADERS "ui/*.h")

set(MAIN_SOURCE main.cpp resources.qrc)

add_executable(${CMAKE_PROJECT_NAME} ${MAIN_SOURCE} ${UI_SRCS} ${UI_HEADERS})

add_subdirectory(ui/components)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    UIComponents  # 这是我们在第三层定义的库名
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Svg
)

if(WIN32) 
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE dwmapi)
endif()