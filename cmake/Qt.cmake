# detect qt version
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
        Core
        Widgets
        Multimedia
)

# really load qt modules
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
        Core
        Widgets
        Multimedia
)