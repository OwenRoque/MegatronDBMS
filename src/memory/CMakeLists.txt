qt_add_library(memory STATIC
    buffermanager.h buffermanager.cpp
    replacer.h
    frame.h frame.cpp
    lrureplacer.h lrureplacer.cpp

)

target_link_libraries(memory PRIVATE
    storage
    Qt${QT_VERSION_MAJOR}::Core
    PUBLIC core
)

target_include_directories(memory INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
