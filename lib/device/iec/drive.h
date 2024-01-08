
#include <stdint.h>
#include <string>
#include <unordered_map>

#include "meat_io.h"
#include "meat_stream.h"


class IECData
{
public:
    /**
     * @brief the primary command byte
     */
    uint8_t primary = 0;
    /**
     * @brief the primary device number
     */
    uint8_t device = 0;
    /**
     * @brief the secondary command byte
     */
    uint8_t secondary = 0;
    /**
     * @brief the secondary command channel
     */
    uint8_t channel = 0;
    /**
     * @brief the device command
     */
    std::string payload = "";

    /**
     * @brief clear and initialize IEC command data
     */
    void init(void)
    {
        primary = 0;
        device = 0;
        secondary = 0;
        channel = 0;
        payload = "";
    }
};

class iecDrive
{
protected:
    std::unique_ptr<MFile> _base;   // Always points to current directory/image
    std::string _last_file;         // Always points to last loaded file

private:

    IECData commanddata;

    // Named Channel functions
    //std::shared_ptr<MStream> currentStream;
    bool registerStream (uint8_t channel);
    std::shared_ptr<MStream> retrieveStream ( uint8_t channel );
    bool closeStream ( uint8_t channel, bool close_all = false );
    uint16_t retrieveLastByte ( uint8_t channel );
    void storeLastByte( uint8_t channel, char last);
    void flushLastByte( uint8_t channel );

    // send single basic line, including heading basic pointer and terminating zero.
    uint16_t sendLine(uint16_t blocks, const char *format, ...);
    uint16_t sendLine(uint16_t blocks, char *text);
    uint16_t sendHeader(std::string header, std::string id);
    uint16_t sendFooter();

    std::unordered_map<uint16_t, std::shared_ptr<MStream>> streams;
    std::unordered_map<uint16_t, uint16_t> streamLastByte;

public:
    void sendListing();
    bool sendFile();
    bool saveFile();
};