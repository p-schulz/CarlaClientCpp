

##### BOOST #####

unix:!mac{

LIBS += -L$$_PRO_FILE_PWD_/../libs/boost/stage/lib
INCLUDEPATH += $$_PRO_FILE_PWD_/../libs/boost/
QMAKE_CXXFLAGS += -isystem $$_PRO_FILE_PWD_/../libs/boost/
LIBS +=    -lboost_system \
           -lboost_filesystem \
           -lboost_program_options \
           -lboost_thread \
           -lboost_chrono \
}

win32{
    Debug{
        contains(QT_ARCH, i386){
            LIBS += -L$$PWD/libs/boost/win32/stage/lib
            INCLUDEPATH += $$PWD/libs/boost
        } else {
            LIBS += -L$$PWD/libs/boost/win64/stage/lib
            INCLUDEPATH += $$PWD/libs/boost/win64
        }
    } 
    Release{
        contains(QT_ARCH, i386){
            LIBS += -L$$PWD/libs/boost/win32/stage/lib
            INCLUDEPATH += $$PWD/libs/boost
        } else {
            LIBS += -L$$PWD/libs/boost/win64/stage/lib
            INCLUDEPATH += $$PWD/libs/boost/win64
        }
    }
}

