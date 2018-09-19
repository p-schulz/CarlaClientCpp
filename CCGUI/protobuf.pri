##### GOOGLE PROTOBUF #####

unix{
        LIBS +=  -L$$_PRO_FILE_PWD_/../libs/protobuf/lib
        INCLUDEPATH += $$_PRO_FILE_PWD_/../libs/protobuf/include
        LIBS +=  -lprotobuf
}
