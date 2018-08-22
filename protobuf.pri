##### GOOGLE PROTOBUF #####

unix{
        INCLUDEPATH += $$_PRO_FILE_PWD_/../libs/protobuf/include
        LIBS +=  -L$$_PRO_FILE_PWD_/../libs/protobuf/lib
        LIBS +=  -lprotobuf
}
