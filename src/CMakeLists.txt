file(GLOB_RECURSE MODEL_SRCS
    "model/*.cpp" 
    "db/*.cpp"
)
file(GLOB_RECURSE MODEL_HEADERS 
    "model/*.h" 
    "db/*.h"       
    "entities/*.h"
)

add_library(DataLayer STATIC ${MODEL_SRCS} ${MODEL_HEADERS})
target_link_libraries(DataLayer PUBLIC 
    Qt6::Core 
    Qt6::Sql
    Qt6::Gui
)
target_include_directories(DataLayer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(ui/components)

set(MAIN_FILES main.cpp resources.qrc ui/main_window.cpp ui/main_window.h)

add_executable(${CMAKE_PROJECT_NAME} ${MAIN_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    UIComponents  
    Qt6::Svg
)

if(WIN32) 
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE dwmapi)
endif()