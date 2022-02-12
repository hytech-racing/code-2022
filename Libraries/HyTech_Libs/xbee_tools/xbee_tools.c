#include "xbee_tools.h"

/*
 * Calculates a 16-bit checksum of the given data using Fletcher's algorithm
 * Source: https://en.wikipedia.org/wiki/Fletcher%27s_checksum
 */
uint16_t fletcher16(const uint8_t *data, size_t len) {
    uint32_t c0, c1;
    unsigned int i;

    for (c0 = c1 = 0; len >= 5802; len -= 5802) {
        for (i = 0; i < 5802; ++i) {
            c0 = c0 + *data++;
            c1 = c1 + c0;
        }
        c0 = c0 % 255;
        c1 = c1 % 255;
    }
    for (i = 0; i < len; ++i) {
        c0 = c0 + *data++;
        c1 = c1 + c0;
    }
    c0 = c0 % 255;
    c1 = c1 % 255;
    return (c1 << 8 | c0);
}

/*
 * Stuffs "length" bytes of data at the location pointed to by
 * "input", writing the output to the location pointed to by
 * "output". Returns the number of bytes written to "output".
 * Source: https://github.com/jacquesf/COBS-Consistent-Overhead-Byte-Stuffing/blob/master/cobs.c
 */
size_t cobs_encode(const uint8_t * input, size_t length, uint8_t * out)
{
    size_t read_index = 0;
    size_t write_index = 1;
    size_t code_index = 0;
    uint8_t code = 1;

    while(read_index < length)
    {
        if(input[read_index] == 0)
        {
            out[code_index] = code;
            code = 1;
            code_index = write_index++;
            read_index++;
        }
        else
        {
            out[write_index++] = input[read_index++];
            code++;
            if(code == 0xFF)
            {
                out[code_index] = code;
                code = 1;
                code_index = write_index++;
            }
        }
    }

    out[code_index] = code;

    return write_index;
}

/*
 * Unstuffs "length" bytes of data at the location pointed to by
 * "input", writing the output * to the location pointed to by
 * "output". Returns the number of bytes written to "output" if
 * "input" was successfully unstuffed, and 0 if there was an
 * error unstuffing "input".
 * Source: https://github.com/jacquesf/COBS-Consistent-Overhead-Byte-Stuffing/blob/master/cobs.c
 */
size_t cobs_decode(const uint8_t * input, size_t length, uint8_t * out)
{
    size_t read_index = 0;
    size_t write_index = 0;
    uint8_t code;
    uint8_t i;

    while(read_index < length)
    {
        code = input[read_index];

        if(read_index + code > length && code != 1)
        {
            return 0;
        }

        read_index++;

        for(i = 1; i < code; i++)
        {
            out[write_index++] = input[read_index++];
        }
        if(code != 0xFF && read_index != length)
        {
            out[write_index++] = '\0';
        }
    }

    return write_index;
}