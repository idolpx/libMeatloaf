
#include "test_parsers.h"
#include "test_meatloaf_filesystems.h"

bool pt = false;    // Run Parser Tests
bool mlfs = true;   // Run Meatloaf File System Tests

int main(int argc, char **argv)
{
    // URL Parser
    if ( pt )
    {
        test_parsers("https://user:password@c64.meatloaf.cc:64128/some/long/path/to/file.ext?query#fragment");
        test_parsers("https://meatloaf.cc");
        test_parsers("ftp://user:pass@domain.com/path/file.ext");
        test_parsers("/path/archive.zip/file.ext");
        test_parsers("https://api.meatloaf.cc/?fb64");
        test_parsers("/fb64");
        test_parsers("/path/to/file.d64/file 1 #ipx");
    }

    // Meatloaf File System
    if ( mlfs )
    {
        test_meatloaf_mfile_properties("goonies.d64");
        test_meatloaf_mfile_directory("goonies.d64");
        //test_meatloaf_mfile_directory("ultima iii.d81");
        //test_meatloaf_mfile_properties("ultima iii.d81");
    }
}