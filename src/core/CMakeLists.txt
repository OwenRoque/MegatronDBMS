qt_add_library(core STATIC
    systemcatalog.h systemcatalog.cpp
    megatron_types.h
)

target_link_libraries(core PRIVATE
    storage
    Qt${QT_VERSION_MAJOR}::Core
)

target_include_directories(core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
