#include "test_parsers.h"

#include "EdUrlParser.h"
#include "peoples_url_parser.h"

void test_parsers(std::string path)
{
    test_EdUrlParser(path);
    test_PeoplesUrlParser(path);
    printf("==================================\r\n\r\n");
}

void test_EdUrlParser(std::string path)
{
    EdUrlParser *url = EdUrlParser::parseUrl( path );


    printf("EdUP [%s]\r\n", url->mRawUrl.c_str());
    printf("Scheme: [%s]\r\n", url->scheme.c_str());
    printf("Host: [%s]\r\n", url->hostName.c_str());
    printf("Port: [%s]\r\n", url->port.c_str());    
    printf("Path: [%s]\r\n", url->path.c_str());
    printf("Query: [%s]\r\n", url->query.c_str());
    printf("Fragment: [%s]\r\n", url->fragment.c_str());
}

void test_PeoplesUrlParser(std::string path)
{
    PeoplesUrlParser *url = PeoplesUrlParser::parseURL( path );

    printf("PUP [%s]\r\n", url->url.c_str());
    printf("Scheme: [%s]\r\n", url->scheme.c_str());
    printf("User: [%s]\r\n", url->user.c_str());
    printf("Password: [%s]\r\n", url->password.c_str());
    printf("Host: [%s]\r\n", url->host.c_str());
    printf("Port: [%s]\r\n", url->port.c_str());    
    printf("Path: [%s]\r\n-\r\n", url->path.c_str());
    printf("Name: [%s]\r\n", url->name.c_str());
    printf("Base Name: [%s]\r\n", url->base_name.c_str());
    printf("Extension: [%s]\r\n", url->extension.c_str());
    printf("Query: [%s]\r\n", url->query.c_str());
    printf("Fragment: [%s]\r\n", url->fragment.c_str());
    printf("----------------------------------\r\n");
}
