
#include "drive.h"

#include <unordered_map>

#include "../../include/debug.h"
#include "../../include/cbm_defines.h"

#include "bus.h"

#include "string_utils.h"



// used to start working with a stream, registering it as underlying stream of some
// IEC channel on some IEC device
bool iecDrive::registerStream ( uint8_t channel )
{
    // Debug_printv("dc_basepath[%s]",  device_config.basepath().c_str());
    // Debug_printv("_file[%s]", _file.c_str());

    // TODO: Determine mode and create the proper stream
    std::ios_base::openmode mode = std::ios_base::in;

    Debug_printv("_base[%s]", _base->url.c_str());
    _base.reset( MFSOwner::File( _base->url ) );

    std::shared_ptr<MStream> new_stream;

    // LOAD / GET / INPUT
    if ( channel == CHANNEL_LOAD )
    {
        if ( !_base->exists() )
            return false;

        Debug_printv("LOAD \"%s\"", _base->url.c_str());
        new_stream = std::shared_ptr<MStream>(_base->meatStream());
    }

    // SAVE / PUT / PRINT / WRITE
    else if ( channel == CHANNEL_SAVE )
    {
        Debug_printv("SAVE \"%s\"", _base->url.c_str());
        // CREATE STREAM HERE FOR OUTPUT
        new_stream = std::shared_ptr<MStream>(_base->meatStream());
        new_stream->open();
    }
    else
    {
        Debug_printv("OTHER \"%s\"", _base->url.c_str());
        new_stream = std::shared_ptr<MStream>(_base->meatStream());
    }


    if ( new_stream == nullptr )
    {
        return false;
    }

    if( !new_stream->isOpen() )
    {
        Debug_printv("Error creating stream");
        return false;
    }
    else
    {
        // Close the stream if it is already open
        closeStream( channel );
    }


    //size_t key = ( IEC.data.device * 100 ) + IEC.data.channel;

    // // Check to see if a stream is open on this device/channel already
    // auto found = streams.find(key);
    // if ( found != streams.end() )
    // {
    //     Debug_printv( "Stream already registered on this device/channel!" );
    //     return false;
    // }

    // Add stream to streams 
    auto newPair = std::make_pair ( channel, new_stream );
    streams.insert ( newPair );

    Debug_printv("Stream created. key[%d]", channel);
    return true;
}

std::shared_ptr<MStream> iecDrive::retrieveStream ( uint8_t channel )
{
    Debug_printv("Stream key[%d]", channel);

    if ( streams.find ( channel ) != streams.end() )
    {
        Debug_printv("Stream retrieved. key[%d]", channel);
        return streams.at ( channel );
    }
    else
    {
        Debug_printv("Error! Trying to recall not-registered stream!");
        return nullptr;
    }
}

bool iecDrive::closeStream ( uint8_t channel, bool close_all )
{
    auto found = streams.find(channel);

    if ( found != streams.end() )
    {
        //Debug_printv("Stream closed. key[%d]", key);
        auto closingStream = (*found).second;
        closingStream->close();
        return streams.erase ( channel );
    }

    return false;
}

uint16_t iecDrive::retrieveLastByte ( uint8_t channel )
{
    if ( streamLastByte.find ( channel ) != streamLastByte.end() )
    {
        return streamLastByte.at ( channel );
    }
    else
    {
        return 999;
    }
}

void iecDrive::storeLastByte( uint8_t channel, char last)
{
    auto newPair = std::make_pair ( channel, (uint16_t)last );
    streamLastByte.insert ( newPair );
}

void iecDrive::flushLastByte( uint8_t channel )
{
    auto newPair = std::make_pair ( channel, (uint16_t)999 );
    streamLastByte.insert ( newPair );
}



// send single basic line, including heading basic pointer and terminating zero.
uint16_t iecDrive::sendLine(uint16_t blocks, const char *format, ...)
{
    // Debug_printv("bus[%d]", IEC.bus_state);

    // // Exit if ATN is PULLED while sending
    // // Exit if there is an error while sending
    // if ( IEC.bus_state == BUS_ERROR )
    // {
    //     // Save file pointer position
    //     //streamUpdate(basicPtr);
    //     //setDeviceStatus(74);
    //     return 0;
    // }

    // Format our string
    va_list args;
    va_start(args, format);
    char text[vsnprintf(NULL, 0, format, args) + 1];
    vsnprintf(text, sizeof text, format, args);
    va_end(args);

    return sendLine(blocks, text);
}

uint16_t iecDrive::sendLine(uint16_t blocks, char *text)
{
    Debug_printf("%d %s ", blocks, text);

    // // Exit if ATN is PULLED while sending
    // // Exit if there is an error while sending
    // if ( IEC.flags & ERROR ) return 0;

    // Get text length
    uint8_t len = strlen(text);

    // Send that pointer
    // No basic line pointer is used in the directory listing set to 0x0101
    IEC.sendByte(0x01);		// IEC.sendByte(basicPtr bitand 0xFF);
    IEC.sendByte(0x01);		// IEC.sendByte(basicPtr >> 8);

    // Send blocks
    IEC.sendByte(blocks bitand 0xFF);
    IEC.sendByte(blocks >> 8);

    // Send line contents
    for (uint8_t i = 0; i < len; i++)
    {
        if ( !IEC.sendByte(text[i]) ) return 0;
    }

    // Finish line
    IEC.sendByte(0);

    Debug_println("");
    
    return len + 5;
} // sendLine

uint16_t iecDrive::sendHeader(std::string header, std::string id)
{
    uint16_t byte_count = 0;
    bool sent_info = false;

    std::string url = _base->host;
    url = mstr::toPETSCII2(url);
    std::string path = _base->pathToFile();
    path = mstr::toPETSCII2(path);
    std::string archive = _base->media_archive;
    archive = mstr::toPETSCII2(archive);
    std::string image = _base->media_image;
    image = mstr::toPETSCII2(image);
    Debug_printv("path[%s] size[%d]", path.c_str(), path.size());

    // Send List HEADER
    uint8_t space_cnt = 0;
    space_cnt = (16 - header.size()) / 2;
    space_cnt = (space_cnt > 8 ) ? 0 : space_cnt;

    //Debug_printv("header[%s] id[%s] space_cnt[%d]", header.c_str(), id.c_str(), space_cnt);

    byte_count += sendLine(0, CBM_REVERSE_ON "\"%*s%s%*s\" %s", space_cnt, "", header.c_str(), space_cnt, "", id.c_str());
    if ( IEC.flags & ERROR ) return 0;

    //byte_count += sendLine(basicPtr, 0, "\x12\"%*s%s%*s\" %.02d 2A", space_cnt, "", PRODUCT_ID, space_cnt, "", device_config.device());
    //byte_count += sendLine(basicPtr, 0, CBM_REVERSE_ON "%s", header.c_str());

    // Send Extra INFO
    if (url.size())
    {
        byte_count += sendLine(0, "%*s\"%-*s\" NFO", 0, "", 19, "[URL]");
        if ( IEC.flags & ERROR ) return 0;
        byte_count += sendLine(0, "%*s\"%-*s\" NFO", 0, "", 19, url.c_str());
        if ( IEC.flags & ERROR ) return 0;
        sent_info = true;
    }
    if (path.size() > 1)
    {
        byte_count += sendLine(0, "%*s\"%-*s\" NFO", 0, "", 19, "[PATH]");
        if ( IEC.flags & ERROR ) return 0;
        byte_count += sendLine(0, "%*s\"%-*s\" NFO", 0, "", 19, path.c_str());
        if ( IEC.flags & ERROR ) return 0;
        sent_info = true;
    }
    if (archive.size() > 1)
    {
        byte_count += sendLine(0, "%*s\"%-*s\" NFO", 0, "", 19, "[ARCHIVE]");
        if ( IEC.flags & ERROR ) return 0;
        byte_count += sendLine(0, "%*s\"%-*s\" NFO", 0, "", 19, archive.c_str());
        if ( IEC.flags & ERROR ) return 0;
    }
    if (image.size())
    {
        byte_count += sendLine(0, "%*s\"%-*s\" NFO", 0, "", 19, "[IMAGE]");
        if ( IEC.flags & ERROR ) return 0;
        byte_count += sendLine(0, "%*s\"%-*s\" NFO", 0, "", 19, image.c_str());
        if ( IEC.flags & ERROR ) return 0;
        sent_info = true;
    }
    if (sent_info)
    {
        byte_count += sendLine(0, "%*s\"-------------------\" NFO", 0, "");
        if ( IEC.flags & ERROR ) return 0;
    }

    // // If SD Card is available ad we are at the root path show it as a directory at the top
    // if (fnSDFAT.running() && _base->url.size() < 2)
    // {
    //     byte_count += sendLine(0, "%*s\"SD\"               DIR", 3, "");
    //     if ( IEC.flags & ERROR ) return 0;
    // }

    return byte_count;
}

uint16_t iecDrive::sendFooter()
{
    uint16_t blocks_free;
    uint16_t byte_count = 0;
    uint64_t bytes_free = _base->getAvailableSpace();

    if ( _base->size() )
    {
        blocks_free = _base->media_blocks_free;
        byte_count = sendLine(blocks_free, "BLOCKS FREE.");
    }
    else
    {
        // We are not in a media file so let's show BYTES FREE instead
        blocks_free = 0;
        byte_count = sendLine(blocks_free, CBM_DELETE CBM_DELETE "%sBYTES FREE.", mstr::formatBytes(bytes_free).c_str() );
    }

    return byte_count;
}

void iecDrive::sendListing()
{
    Debug_printf("sendListing: [%s]\r\n=================================\r\n", _base->url.c_str());

    uint16_t byte_count = 0;
    std::string extension = "dir";

    std::unique_ptr<MFile> entry = std::unique_ptr<MFile>( _base->getNextFileInDir() );

    if(entry == nullptr) {
        closeStream( commanddata.channel );

        bool isOpen = registerStream(commanddata.channel);
        if(isOpen) 
        {
            sendFile();
        }
        else
        {
            IEC.senderTimeout(); // File Not Found
        }
        
        return;
    }

    //fnLedStrip.startRainbow(300);

    // Send load address
    IEC.sendByte(CBM_BASIC_START & 0xff);
    IEC.sendByte((CBM_BASIC_START >> 8) & 0xff);
    byte_count += 2;

    // If there has been a error don't try to send any more bytes
    if ( IEC.flags & ERROR )
    {
        Debug_printv(":(");
        return;
    }

    Debug_println("");

    // Send Listing Header
    if (_base->media_header.size() == 0)
    {
        // Send device default listing header
        char buf[7] = { '\0' };
        sprintf(buf, "%.02d 2A", IEC.data.device);
        byte_count += sendHeader(PRODUCT_ID, buf);
        if ( IEC.flags & ERROR ) return;
    }
    else
    {
        // Send listing header from media file
        byte_count += sendHeader(_base->media_header.c_str(), _base->media_id.c_str());
        if ( IEC.flags & ERROR ) return;
    }

    // Send Directory Items
    while(entry != nullptr)
    {
        if (!entry->isDirectory())
        {
            // Get extension
            if (entry->extension.length())
            {
                extension = entry->extension;
            }
            else
            {
                extension = "prg";
            }
        }
        else
        {
            extension = "dir";
        }

        // Don't show hidden folders or files
        //Debug_printv("size[%d] name[%s]", entry->size(), entry->name.c_str());
        std::string name = entry->name;
        if ( !entry->isPETSCII )
        {
            name = mstr::toPETSCII2( entry->name );
            extension = mstr::toPETSCII2(extension);
        }
        mstr::rtrimA0(name);
        mstr::replaceAll(name, "\\", "/");

        //uint32_t s = entry->size();
        //uint32_t block_cnt = s / _base->media_block_size;
        uint32_t block_cnt = entry->blocks();
        // Debug_printv( "size[%d] blocks[%d] blocksz[%d]", s, block_cnt, _base->media_block_size );
        //if ( s > 0 && s < _base->media_block_size )
        //    block_cnt = 1;

        uint8_t block_spc = 3;
        if (block_cnt > 9)
            block_spc--;
        if (block_cnt > 99)
            block_spc--;
        if (block_cnt > 999)
            block_spc--;

        uint8_t space_cnt = 21 - (name.size() + 5);
        if (space_cnt > 21)
            space_cnt = 0;

        if (name[0]!='.')
        {
            // Exit if ATN is PULLED while sending
            // Exit if there is an error while sending
            if ( IEC.bus_state == BUS_ERROR )
            {
                // Save file pointer position
                // streamUpdate(byte_count);
                //setDeviceStatus(74);
                return;
            }

            byte_count += sendLine(block_cnt, "%*s\"%s\"%*s %s", block_spc, "", name.c_str(), space_cnt, "", extension.c_str());
            if ( IEC.flags & ERROR ) return;
        }

        entry.reset(_base->getNextFileInDir());

        //fnLedManager.toggle(eLed::LED_BUS);
    }

    // Send Listing Footer
    byte_count += sendFooter();
    if ( IEC.flags & ERROR ) return;

    // End program with two zeros after last line. Last zero goes out as EOI.
    IEC.sendByte(0);
    IEC.sendByte(0, true);
    //closeStream();

    Debug_printf("\r\n=================================\r\n%d bytes sent\r\n", byte_count);

    //fnLedManager.set(eLed::LED_BUS, false);
    //fnLedStrip.stopRainbow();
} // sendListing



// bool iecDrive::sendFile()
// {
//     size_t count = 0;
//     bool success_rx = true;
//     bool success_tx = true;

//     uint8_t b;  // byte
//     uint8_t nb; // next byte
//     size_t bi = 0;
//     size_t load_address = 0;
//     size_t sys_address = 0;

// 	//iecStream.open(&IEC);

// #ifdef DATA_STREAM
//     char ba[9];
//     ba[8] = '\0';
// #endif

//     // std::shared_ptr<MStream> istream = std::static_pointer_cast<MStream>(currentStream);
//     auto istream = retrieveStream(commanddata.channel);
//     if ( istream == nullptr )
//     {
//         Debug_printv("Stream not found!");
//         IEC.senderTimeout(); // File Not Found
//         //closeStream(commanddata.channel);
//         _base.reset( MFSOwner::File( _base->base() ) );
//         return false;
//     }

//     if ( !_base->isDirectory() )
//     {
//         if ( istream->has_subdirs )
//         {
//             PeoplesUrlParser u;
//             u.parseUrl( istream->url );
//             Debug_printv( "Subdir Change Directory Here! istream[%s] > base[%s]", istream->url.c_str(), u.base().c_str() );
//             _last_file = u.name;
//             _base.reset( MFSOwner::File( u.base() ) );
//         }
//         else
//         {
//             auto f = MFSOwner::File( istream->url );
//             Debug_printv( "Change Directory Here! istream[%s] > base[%s]", istream->url.c_str(), f->streamFile->url.c_str() );
//             _base.reset( f->streamFile );
//         }

//     }

//     uint32_t len = istream->size();
//     uint32_t avail = istream->available();
//     if ( !len )
//         len = -1;

//     //fnLedStrip.startRainbow(300);

//     if( IEC.data.channel == CHANNEL_LOAD )
//     {
//         // Get/Send file load address
//         count = 2;
//         istream->read(&b, 1);
//         success_tx = IEC.sendByte(b);
//         load_address = b & 0x00FF; // low byte
//         istream->read(&b, 1);
//         success_tx = IEC.sendByte(b);
//         load_address = load_address | b << 8;  // high byte
//         sys_address = load_address;
//         Debug_printv( "load_address[$%.4X] sys_address[%d]", load_address, sys_address );

//         // Get SYSLINE
//     }

//     // Read byte
//     success_rx = istream->read(&b, 1);
//     //Debug_printv("b[%02X] success[%d]", b, success_rx);

//     Debug_printf("sendFile: [$%.4X]\r\n=================================\r\n", load_address);
//     while( success_rx && !istream->error() )
//     {
//         // Read next byte
//         success_rx = istream->read(&nb, 1);

//         //Debug_printv("b[%02X] nb[%02X] success_rx[%d] error[%d]", b, nb, success_rx, istream->error());
// #ifdef DATA_STREAM
//         if (bi == 0)
//         {
//             Debug_printf(":%.4X ", load_address);
//             load_address += 8;
//         }
// #endif
//         // Send Byte
//         if ( count + 1 == avail || !success_rx )
//         {
//             //Debug_printv("b[%02X] EOI %i", b, count);
//             success_tx = IEC.sendByte(b, true); // indicate end of file.
//             if ( !success_tx )
//                 Debug_printv("tx fail");

//             break;
//         }
//         else
//         {
//             success_tx = IEC.sendByte(b);
//             if ( !success_tx )
//             {
//                 Debug_printv("tx fail");
//                 //break;
//             }

//         }
//         b = nb; // byte = next byte
//         count++;

// #ifdef DATA_STREAM
//         // Show ASCII Data
//         if (b < 32 || b >= 127)
//             ba[bi++] = 46;
//         else
//             ba[bi++] = b;

//         if(bi == 8)
//         {
//             uint32_t t = (count * 100) / len;
//             Debug_printf(" %s (%d %d%%) [%d]\r\n", ba, count, t, avail);
//             bi = 0;
//         }
// #else
//         uint32_t t = (count * 100) / len;
//         Debug_printf("\rTransferring %d%% [%d, %d]      ", t, count, avail);
// #endif

//         // Exit if ATN is PULLED while sending
//         //if ( IEC.status ( PIN_IEC_ATN ) == PULLED )
//         if ( IEC.flags & ATN_PULLED )
//         {
//             //Debug_printv("ATN pulled while sending. i[%d]", i);

//             // Save file pointer position
//             istream->seek(istream->position() - 2);
//             //success_rx = true;
//             break;
//         }

//         // // Toggle LED
//         // if (i % 50 == 0)
//         // {
//         // 	fnLedManager.toggle(eLed::LED_BUS);
//         // }
//     }

// #ifdef DATA_STREAM
//     if (bi)
//     {
//       uint32_t t = (count * 100) / len;
//       ba[bi] = 0;
//       Debug_printf(" %s (%d %d%%) [%d]\r\n", ba, count, t, avail);
//       bi = 0;
//     }
// #endif

//     Debug_printf("\r\n=================================\r\n%d bytes sent of %d [SYS%d]\r\n", count, avail, sys_address);

//     //Debug_printv("len[%d] avail[%d] success_rx[%d]", len, avail, success_rx);

//     //fnLedManager.set(eLed::LED_BUS, false);
//     //fnLedStrip.stopRainbow();

//     if ( istream->error() )
//     {
//         Debug_println("sendFile: Transfer aborted!");
//         IEC.senderTimeout();
//         closeStream(commanddata.channel);
//     }

//     return success_rx;
// } // sendFile

bool iecDrive::sendFile()
{
    size_t count = 0;
    bool success_rx = true;
    bool success_tx = true;

    uint8_t b;  // byte
    uint8_t nb; // next byte
    size_t bi = 0;
    size_t load_address = 0;
    size_t sys_address = 0;

	//iecStream.open(&IEC);

#ifdef DATA_STREAM
    char ba[9];
    ba[8] = '\0';
#endif

    // std::shared_ptr<MStream> istream = std::static_pointer_cast<MStream>(currentStream);
    auto istream = retrieveStream(commanddata.channel);
    if ( istream == nullptr )
    {
        Debug_printv("Stream not found!");
        IEC.senderTimeout(); // File Not Found
        _last_file = "";
        _base.reset( MFSOwner::File( _base->base() ) );
        return false;
    }

    if ( !_base->isDirectory() )
    {
        if ( istream->has_subdirs )
        {
            PeoplesUrlParser *u = PeoplesUrlParser::parseUrl( istream->url );
            Debug_printv( "Subdir Change Directory Here! istream[%s] > base[%s]", istream->url.c_str(), u->base().c_str() );
            _last_file = u->name;
            _base.reset( MFSOwner::File( u->base() ) );
        }
        else
        {
            auto f = MFSOwner::File( istream->url );
            Debug_printv( "Change Directory Here! istream[%s] > base[%s]", istream->url.c_str(), f->streamFile->url.c_str() );
            _base.reset( f->streamFile );
        }
    }

    bool eoi = false;
    uint32_t len = istream->size();
    uint32_t avail = istream->available();

    //fnLedStrip.startRainbow(300);
    Debug_printv("len[%d] avail[%d]", len, avail);

    if( commanddata.channel == CHANNEL_LOAD )
    {
        // Get/Send file load address
        count = 2;
        istream->read(&b, 1);
        success_tx = IEC.sendByte(b);
        load_address = b & 0x00FF; // low byte
        istream->read(&b, 1);
        success_tx = IEC.sendByte(b);
        load_address = load_address | b << 8;  // high byte
        sys_address = load_address;
        Debug_printv( "load_address[$%.4X] sys_address[%d]", load_address, sys_address );

        // Get SYSLINE
    }

    // Read byte
    success_rx = istream->read(&b, 1);
    //Debug_printv("b[%02X] success[%d]", b, success_rx);

    Debug_printf("sendFile: [$%.4X]\r\n=================================\r\n", load_address);
    while( success_rx && !istream->error() )
    {
        count = istream->position();
        avail = istream->available();

        //Debug_printv("b[%02X] nb[%02X] success_rx[%d] error[%d]", b, nb, success_rx, istream->error());
#ifdef DATA_STREAM
        if (bi == 0)
        {
            Debug_printf(":%.4X ", load_address);
            load_address += 8;
        }
#endif

        // Send Byte
        //IEC.pull(PIN_IEC_SRQ);
        success_tx = IEC.sendByte(b, eoi);
        if ( !success_tx )
        {
            Debug_printv("tx fail");
            //IEC.release(PIN_IEC_SRQ);
            return false;
        }
        //IEC.release(PIN_IEC_SRQ);

        // Read next byte
        success_rx = istream->read(&nb, 1);

        // Is this the last byte in the stream?
        if ( istream->eos() )
            eoi = true;

        b = nb; // byte = next byte

        uint32_t t = (count * 100) / len;
#ifdef DATA_STREAM
        // Show ASCII Data
        if (b < 32 || b >= 127)
            ba[bi++] = 46;
        else
            ba[bi++] = b;

        if(bi == 8)
        {
            Debug_printf(" %s (%d %d%%) [%d]\r\n", ba, count, t, avail);
            bi = 0;
        }
#else
        Debug_printf("\rTransferring %d%% [%d, %d]      ", t, count, avail);
#endif

        // Exit if ATN is PULLED while sending
        //if ( IEC.status ( PIN_IEC_ATN ) == PULLED )
        if ( IEC.flags & ATN_PULLED || istream->error() )
        {
            //Debug_printv("ATN pulled while sending. i[%d]", i);

            // Save file pointer position
            istream->seek(istream->position() - 2);
            //success_rx = true;
            break;
        }

        // // Toggle LED
        // if (i % 50 == 0)
        // {
        // 	fnLedManager.toggle(eLed::LED_BUS);
        // }
    }

#ifdef DATA_STREAM
    uint32_t t = (count * 100) / len;
    ba[bi++] = 0;
    Debug_printf(" %s (%d %d%%) [%d]\r\n", ba, count, t, avail);
#endif

    Debug_printf("\r\n=================================\r\n%d bytes sent of %d [SYS%d]\r\n", count, avail, sys_address);

    //Debug_printv("len[%d] avail[%d] success_rx[%d]", len, avail, success_rx);

    //fnLedManager.set(eLed::LED_BUS, false);
    //fnLedStrip.stopRainbow();

    if ( istream->error() )
    {
        Debug_println("sendFile: Transfer aborted!");
        IEC.senderTimeout();
        closeStream(commanddata.channel);
    }

    return success_rx;
} // sendFile


bool iecDrive::saveFile()
{
    size_t i = 0;
    bool success = true;
    bool done = false;

    size_t bi = 0;
    size_t load_address = 0;
    size_t b_len = 1;
    uint8_t b[b_len];
    uint8_t ll[b_len];
    uint8_t lh[b_len];

#ifdef DATA_STREAM
    char ba[9];
    ba[8] = '\0';
#endif

    auto ostream = retrieveStream(commanddata.channel);

    if ( ostream == nullptr ) {
        Debug_printv("couldn't open a stream for writing");
        IEC.senderTimeout(); // File Not Found
        return false;
    }
    else
    {
         // Stream is open!  Let's save this!

        // wait - what??? If stream position == x you don't have to seek(x)!!!
        // if ( ostream->position() > 0 )
        // {
        // 	// // Position file pointer
        // 	// ostream->seek(currentStream.cursor);
        // }
        // else
        //fnLedStrip.startRainbow(300);
        {
            // Get file load address
            ll[0] = IEC.receiveByte();
            load_address = *ll & 0x00FF; // low byte
            lh[0] = IEC.receiveByte();
            load_address = load_address | *lh << 8;  // high byte
        }


        Debug_printv("saveFile: [$%.4X]\r\n=================================\r\n", load_address);

        // Recieve bytes until a EOI is detected
        do
        {
            // Save Load Address
            if (i == 0)
            {
                Debug_print("[");
                ostream->write(ll, b_len);
                ostream->write(lh, b_len);
                i += 2;
                Debug_println("]");
            }

#ifdef DATA_STREAM
            if (bi == 0)
            {
                Debug_printf(":%.4X ", load_address);
                load_address += 8;
            }
#endif

            b[0] = IEC.receiveByte();
            // if(ostream->isText())
            // 	ostream->putPetsciiAsUtf8(b[0]);
            // else
                ostream->write(b, b_len);
            i++;

            uint16_t f = IEC.flags;
            done = (f & EOI_RECVD) or (f & ERROR);

            // Exit if ATN is PULLED while sending
            if ( f & ATN_PULLED )
            {
                // Save file pointer position
                // streamUpdate(ostream->position());
                //setDeviceStatus(74);
                break;
            }

#ifdef DATA_STREAM
            // Show ASCII Data
            if (b[0] < 32 || b[0] >= 127)
                ba[bi++] = 46;
            else
                ba[bi++] = b[0];

            if(bi == 8)
            {
                Debug_printf(" %s (%d)\r\n", ba, i);
                bi = 0;
            }
#endif
            // // Toggle LED
            // if (0 == i % 50)
            // {
            // 	fnLedManager.toggle(eLed::LED_BUS);
            // }
        } while (not done);
    }
    // ostream->close(); // nor required, closes automagically

    Debug_printf("=================================\r\n%d bytes saved\r\n", i);
    //fnLedManager.set(eLed::LED_BUS, false);
    //fnLedStrip.stopRainbow();

    // TODO: Handle errorFlag

    return success;
} // saveFile
