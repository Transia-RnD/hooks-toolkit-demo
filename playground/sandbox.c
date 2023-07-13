#include <iostream>

uint8_t hexCharToUint8(char hexChar) {
    if (hexChar >= '0' && hexChar <= '9') {
        return hexChar - '0';
    }
    if (hexChar >= 'a' && hexChar <= 'f') {
        return hexChar - 'a' + 10;
    }
    if (hexChar >= 'A' && hexChar <= 'F') {
        return hexChar - 'A' + 10;
    }
    return 0;
}

void hexStringToUint8Buffer(const std::string& hexString, uint8_t* buffer) {
    size_t len = hexString.length();
    for (size_t i = 0; i < len; i += 2) {
        uint8_t highNibble = hexCharToUint8(hexString[i]);
        uint8_t lowNibble = hexCharToUint8(hexString[i + 1]);
        buffer[i / 2] = (highNibble << 4) | lowNibble;
    }
}

int main() {
    std::string hexString = "201C00000000F8E511006125000001C2553665";
    uint8_t buffer[hexString.length() / 2];
    hexStringToUint8Buffer(hexString, buffer);

    // Print the buffer to verify
    for (int i = 0; i < hexString.length() / 2; i++) {
        std::cout << std::hex << (int)buffer[i] << " ";
    }
    return 0;
}

