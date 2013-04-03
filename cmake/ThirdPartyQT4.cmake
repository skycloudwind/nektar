# QT4 
OPTION(NEKTAR_USE_QT4
    "Use QT4 for graphical tools and utilities." OFF)

IF( NEKTAR_USE_QT4 )
    FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui REQUIRED)
    INCLUDE(${QT_USE_FILE})
    ADD_DEFINITIONS(${QT_DEFINITIONS})
    IF (QT4_FOUND)
        MESSAGE(STATUS "Found QT4: ${QT_INCLUDE_DIR}")
    ENDIF(QT4_FOUND)
ENDIF( NEKTAR_USE_QT4 )