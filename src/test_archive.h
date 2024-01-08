#ifndef TEST_ARCHIVE_H
#define TEST_ARCHIVE_H

#include <archive.h>
#include <archive_entry.h>
#include <stdio.h>

void test_archive();

class ArchiveStreamData {
public:
    uint8_t *srcBuffer = nullptr;
    FILE *file = nullptr; // a stream that is able to serve bytes of this archive
};

static ArchiveStreamData streamData;

#endif // TEST_ARCHIVE_H