qt_add_library(core STATIC
    database.h database.cpp
    megatron_types.h megatron_structs.h
    file.h
    heapfile.h heapfile.cpp
    page.h page.cpp

    diskmanager.h diskmanager.cpp
    systemcatalog.h systemcatalog.cpp
    record.h record.cpp
    pagefactory.h

)

target_link_libraries(core PRIVATE
    storage
    Qt${QT_VERSION_MAJOR}::Core
)

target_include_directories(core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
