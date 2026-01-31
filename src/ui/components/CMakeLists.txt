file(GLOB COMPONENTS_SRCS "*.cpp")
file(GLOB COMPONENTS_HEADERS "*.h")


add_library(UIComponents STATIC ${COMPONENTS_SRCS} ${COMPONENTS_HEADERS})

target_link_libraries(UIComponents PUBLIC
  Qt6::Widgets 
)