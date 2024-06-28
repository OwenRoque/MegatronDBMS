qt_add_library(memory STATIC
    buffermanager.h buffermanager.cpp

)

target_link_libraries(memory PRIVATE
    core
    storage
    Qt${QT_VERSION_MAJOR}::Core
)

target_include_directories(memory INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
