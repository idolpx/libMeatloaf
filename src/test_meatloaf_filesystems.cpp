#include "test_meatloaf_filesystems.h"
#include <iostream>

#include <dirent.h>
#include <sys/stat.h>

#include "meat_io.h"
#include "meat_stream.h"
#include "meat_buffer.h"


void test_meatloaf_mfile_directory(std::string path)
{
    Debug_printv("%s", path.c_str());
    std::unique_ptr<MFile> file = Meat::New<MFile>( path );
    
    printf("[List Dir]\r\n");
    if(file->isDirectory()) {
        printf("{ENTRY} ");
        std::unique_ptr<MFile> entry(file->getNextFileInDir());

        while(entry != nullptr) {
            if(entry->isDirectory())
            {
                printf("     %s <dir>\r\n", entry->name.c_str());
            }
            else
            {
                printf("%d %s <%s>\r\n", entry->size(), entry->name.c_str(), entry->extension.c_str());
            }

            entry.reset(file->getNextFileInDir());
        }
    }
}


void test_meatloaf_mfile_properties(std::string path)
{
    std::unique_ptr<MFile> testMFile = Meat::New<MFile>( path );

    printf("MFile [%s]\r\n", testMFile->url.c_str());
    printf("Scheme: [%s]\r\n", testMFile->scheme.c_str());
    printf("User: [%s]\r\n", testMFile->user.c_str());
    printf("Password: [%s]\r\n", testMFile->password.c_str());
    printf("Host: [%s]\r\n", testMFile->host.c_str());
    printf("Port: [%s]\r\n", testMFile->port.c_str());    
    printf("Path: [%s]\r\n", testMFile->path.c_str());
    printf("\r\n-\r\n");
    printf("Name: [%s]\r\n", testMFile->name.c_str());
    printf("Base Name: [%s]\r\n", testMFile->base_name.c_str());
    printf("Extension: [%s]\r\n", testMFile->extension.c_str());
    printf("Query: [%s]\r\n", testMFile->query.c_str());
    printf("Fragment: [%s]\r\n", testMFile->fragment.c_str());
    printf("\r\n-\r\n");
    printf("pathInStream: [%s]\r\n", testMFile->pathInStream.c_str());
    if ( testMFile->streamFile )
        printf("streamFile: [%s]\r\n", testMFile->streamFile->url.c_str());
    printf("\r\n-\r\n");
    printf("exists: [%d]\r\n", testMFile->exists());
    printf("size: [%d]\r\n", testMFile->size());
    printf("isText: [%d]\r\n", testMFile->isText());
    printf("isDirectory: [%d]\r\n", testMFile->isDirectory());
    printf("-------------------------------\r\n");
}
