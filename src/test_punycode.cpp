#include "U8Char.h"
#include "punycode.h"
#include <stdio.h>
#include "archive.h"

void test_punycode()
{
    //std::string chinese = "文件档案名";
    //                                   0x6587 0x4EF6 0x6863 0x6848 0x540D
    //                         E6 96 87 E4 BB B6 E6 A1 A3 E6 A1 88 E5 90 8D
    const uint32_t chineseAsUnicode[] = {0x6587, 0x4ef6, 0x6843, 0x684c, 0x540d};
    char asPunycode[256];
    size_t dstlen = sizeof asPunycode;
    // size_t punycode_encode(const uint32_t *const src, const size_t srclen, char *const dst, size_t *const dstlen)
    punycode_encode(chineseAsUnicode, 5, asPunycode, &dstlen);
    std::string punycode(asPunycode, dstlen);
    printf("Chinese U32 as punycode:'%s' (should be 5nqx7jp0rbsckb)\n", punycode.c_str());
    
    std::string asUnicode = U8Char::fromPunycode("5nqx7jp0rbsckb");
    printf("Chinese UTF8 from the above punycode:'%s (should be 文件档案名)'\n", asUnicode.c_str());

     std::string punycode2 = U8Char::toPunycode(asUnicode);
     printf("Chinese text as from punycode again:'%s'\n", punycode2.c_str());


    // uint32_t asU32[1024];
    //char asPunycode[1024];
    //dstlen = sizeof asPunycode;
    // size_t n_converted;
    // U8Char temp(' ');

    // Debug_printv("Calling toUnicode32\n");
    // size_t conv_len = temp.toUnicode32(asUnicode, asU32, sizeof asU32);
    // Debug_printv("Conv len=%d, encoding now...\n", conv_len);
    // n_converted = punycode_encode(asU32, conv_len, asPunycode, &dstlen);    

}
