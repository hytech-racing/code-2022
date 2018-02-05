
def fletcher16(data):
    index = 0
    c0 = 0
    c1 = 0
    i = 0
    length = len(data)
    while length >= 5802:
        for i in range(5802):
            c0 += data[index]
            c1 += c0
            index += 1
        c0 %= 255
        c1 %= 255
        length -= 5802
    
    index = 0
    for i in range(len(data)):
        c0 += data[index]
        c1 += c0
        index += 1
    c0 %= 255
    c1 %= 255
    return (c1 << 8 | c0)

def cobs_decode(data):
    hex_data = bytearray.fromhex(data)
    output = []
    read_index = 0
    write_index = 0
    code = 0

    while read_index < len(hex_data):
        code = hex_data[read_index]
        if (read_index + code) > len(hex_data) and code != 1:
            print("RI: ", read_index, " code: ", code)
            return output
        read_index += 1
        for i in range(1, code):
            output.append(hex_data[read_index]);
            write_index += 1
            read_index += 1
        if code != 0xFF and read_index != len(hex_data):
            output.append(0)
            write_index += 1
    
    return bytes(output)
