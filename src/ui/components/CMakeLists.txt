file(GLOB COMPONENTS_SRCS CONFIGURE_DEPENDS "*.cpp")
file(GLOB COMPONENTS_HEADERS CONFIGURE_DEPENDS "*.h")

add_library(UIComponents STATIC 
    ${COMPONENTS_SRCS} 
    ${COMPONENTS_HEADERS}
)

target_link_libraries(UIComponents PUBLIC
    DataLayer
    Qt6::Widgets
)
