#include "test_archive.h"
#include <iostream>

struct archive *a;

/* Returns pointer and size of next block of data from archive. */
// The read callback returns the number of bytes read, zero for end-of-file, or a negative failure code as above.
// It also returns a pointer to the block of data read.
// https://github.com/libarchive/libarchive/wiki/LibarchiveIO
//
// This callback is just a way to get bytes from srcStream into libarchive for processing
ssize_t cb_read(struct archive *a, void *userData, const void **buff)
{
    // ArchiveStreamData *streamData = (ArchiveStreamData *)userData;
    // // 1. we have to call srcStr.read(...)
    // ssize_t bc = streamData->srcStream->read(streamData->srcBuffer, ArchiveStream::buffSize);
    // //std::string dump((char*)streamData->srcBuffer, bc);
    // //Debug_printv("Libarch pulling data from src MStream, got bytes:%d", bc);
    // //Debug_printv("Dumping bytes: %s", dump.c_str());
    // // 2. set *buff to the bufer read in 1.
    // *buff = streamData->srcBuffer;
    // // 3. return read bytes count
    // return bc;

    FILE* file = ((ArchiveStreamData*)userData)->file;

    char buffer[4096]; // Assuming a buffer size of 100 bytes
    size_t bytesRead = fread(buffer, 1, sizeof(buffer), file);

    // Check if reading was successful
    if (bytesRead > 0) {
        *buff = buffer;
        printf("cb_read: ZIPFILE->LIBARCH %zu bytes from the file\n", bytesRead);
    } else if(bytesRead == 0) {
        printf("cb_read: End of file reached\n");
    } else {
        printf("cb_read: Error reading from the file %zu\n", bytesRead);
    }

    return bytesRead;
}

/*
It must return the number of bytes actually skipped, or a negative failure code if skipping cannot be done.
It can skip fewer bytes than requested but must never skip more.
Only positive/forward skips will ever be requested.
If skipping is not provided or fails, libarchive will call the read() function and simply ignore any data that it does not need.

* Skips at most request bytes from archive and returns the skipped amount.
* This may skip fewer bytes than requested; it may even skip zero bytes.
* If you do skip fewer bytes than requested, libarchive will invoke your
* read callback and discard data as necessary to make up the full skip.
*/
// https://github.com/libarchive/libarchive/wiki/LibarchiveIO
int64_t cb_skip(struct archive *a, void *userData, int64_t request)
{
    std::cout << "cb_skip called" << request << "\n";
    // Debug_printv("bytes[%d]", request);
    // ArchiveStreamData *streamData = (ArchiveStreamData *)userData;

    // if (streamData->srcStream->isOpen())
    // {
    //     bool rc = streamData->srcStream->seek(request, SEEK_CUR);
    //     return (rc) ? request : ARCHIVE_WARN;
    // }
    // else
    // {
    //     Debug_printv("ERROR! skip failed");
    //     return ARCHIVE_FATAL;
    // }
    return ARCHIVE_FATAL;
}

int64_t cb_seek(struct archive *a, void *userData, int64_t offset, int whence)
{
    // Debug_printv("offset[%d] whence[%d] (0=begin, 1=curr, 2=end)", offset, whence);
    // ArchiveStreamData *streamData = (ArchiveStreamData *)userData;

    // if (streamData->srcStream->isOpen())
    // {
    //     bool rc = streamData->srcStream->seek(offset, whence);
    //     return (rc) ? offset : ARCHIVE_WARN;
    // }
    // else
    // {
    //     Debug_printv("ERROR! seek failed");
    //     return ARCHIVE_FATAL;
    // }

    FILE* file = ((ArchiveStreamData*)userData)->file;
    auto rc = fseek(file, offset, whence);

    std::cout << "cb_seek called for:" << offset << "," << whence <<
        " rc:" << rc << "\n";

    return (rc==0) ? ARCHIVE_OK : ARCHIVE_FATAL;
}

int cb_close(struct archive *a, void *userData)
{
    std::cout << "cb_close called\n";
    // ArchiveStreamData *src_str = (ArchiveStreamData *)userData;
    
    // Debug_printv("Libarch wants to close, but we do nothing here...");

    // // do we want to close srcStream here???
    FILE* file = ((ArchiveStreamData*)userData)->file;

    if(file)
        fclose(file);
    return (ARCHIVE_OK);
}

int cb_open(struct archive *arch, void *userData)
{
    std::cout << "cb_open called\n";
    // maybe we can use open for something? Check if stream is open?

    const char *filePath = "message2.zip";
    //const char *filePath = "_arch.7z";

    // Open the file in binary mode for reading
    ((ArchiveStreamData*)userData)->file = fopen(filePath, "rb");
    std::cout << "end of cb_open call\n";

    return (ARCHIVE_OK);
}


void read_dir()
{
    struct archive_entry *entry;
    char buffer[4096]; // Assuming a buffer size of 100 bytes

    while(archive_read_next_header(a, &entry) == ARCHIVE_OK)
    {
        std::cout << "entry name: " << archive_entry_pathname(entry) << std::endl;
    }
}

// https://github.com/mpv-player/mpv/blob/d8c2e33a5d3840045a84cd5fa22c9c601fb1a0ae/stream/stream_libarchive.c#L315
// funkcja: 
// archive_entry_open
// -> reopen_archive otwiera stumień i potem
// -> mp_archive_next_entry
// -> iteruje archive_read_next_header
// wszystko, co ciekawe musi dziać się w mp_archive_new_raw

void read_zipped()
{
    struct archive_entry *entry;
    char buffer[4096]; // Assuming a buffer size of 100 bytes

    while(archive_read_next_header(a, &entry) == ARCHIVE_OK)
    {
        std::cout << "=== FOUND FILE: " << archive_entry_pathname(entry) << std::endl;
        // archive_read_data_skip(a);
        while(int r = archive_read_data(a, buffer, 4096)) 
        {
            if(r < 0) {  
                // -10 retry -20 warn -25 failed -30 fatal      
                std::cout << "archive_read_data failed with: " << r << std::endl;
                std::cout << "error: " << archive_error_string(a) << "\n";
    
                break;
            }
            else
            {
                std::string data(buffer, r);
                std::cout << "LIBARCH->UNZIPPED data count: " << r << "\n";
                 // "\n>>>>>>>>>>>>>>>>\n" << data << "\n<<<<<<<<<<<<<<<<<<<<" << std::endl;
            }
        }
        std::cout << "attempting to get the next header\n";
    }
}



void testArchive()
{
    std::cout << "archive_reaa_new\n";
    a = archive_read_new();
    std::cout << "archive filters\n";
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    archive_read_set_read_callback(a, cb_read);
    // archive_read_set_skip_callback(a, cb_skip);
    archive_read_set_seek_callback(a, cb_seek);
    archive_read_set_close_callback(a, cb_close);
    archive_read_set_open_callback(a, cb_open);
    archive_read_set_callback_data(a, &streamData);
    std::cout << "archive_read_open\n";
    int r =  archive_read_open1(a);
    std::cout << "let's read the zip!\n";
    read_zipped();
    archive_read_close(a);
    archive_read_free(a);
    //std::cout << "Archive created: " << archive_path << std::endl;
}