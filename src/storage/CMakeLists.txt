qt_add_library(storage STATIC
    disk.h disk.cpp
    diskcontroller.h diskcontroller.cpp
    platter.h platter.cpp
    utility.h
    surface.h surface.cpp
    sector.h sector.cpp
    track.h track.cpp
    armassembly.h armassembly.cpp
    rwhead.h rwhead.cpp
    block.h block.cpp
)

target_link_libraries(storage PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_include_directories(storage INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${Qt6Widgets_INCLUDE_DIRS})
