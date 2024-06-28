
qt_add_resources(QRC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../resources/resources.qrc
)


qt_add_library(interface STATIC
    ${QRC_SOURCES}
    diskinit.ui diskinit.h diskinit.cpp
    newrelation.ui newrelation.h newrelation.cpp
    queryform.ui queryform.h queryform.cpp
    attributetablewidget.h attributetablewidget.cpp

    lineeditdelegate.h
    comboboxdelegate.h
    texteditdelegate.h
    checkboxdelegate.h
    customcomboboxdelegate.h
    customindexcombobox.h
    indexform.h indexform.cpp indexform.ui


)

target_link_libraries(interface PRIVATE
    core
    storage
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_include_directories(interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${Qt6Widgets_INCLUDE_DIRS})
