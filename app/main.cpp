#include <iostream>
#include <fstream>
#include <cstdint>
#include <array>
#include <bitset>
#include <getopt.h>

std::array<uint16_t, 8> encodingMatrix = {
        uint16_t(0xF080), // {1, 1, 1, 1, 0, 0, 0, 0,   1, 0, 0, 0, 0, 0, 0, 0}
        uint16_t(0xCC40), // {1, 1, 0, 0, 1, 1, 0, 0,   0, 1, 0, 0, 0, 0, 0, 0}
        uint16_t(0xAA20), // {1, 0, 1, 0, 1, 0, 1, 0,   0, 0, 1, 0, 0, 0, 0, 0}
        uint16_t(0x5610), // {0, 1, 0, 1, 0, 1, 1, 0,   0, 0, 0, 1, 0, 0, 0, 0}
        uint16_t(0xE908), // {1, 1, 1, 0, 1, 0, 0, 1,   0, 0, 0, 0, 1, 0, 0, 0}
        uint16_t(0x9504), // {1, 0, 0, 1, 0, 1, 0, 1,   0, 0, 0, 0, 0, 1, 0, 0}
        uint16_t(0x7B02), // {0, 1, 1, 1, 1, 0, 1, 1,   0, 0, 0, 0, 0, 0, 1, 0}
        uint16_t(0xE701)  // {1, 1, 1, 0, 0, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 1}
};

std::array<uint8_t, 16> decodingMatrix = {
        uint8_t(0xED), // {1, 1, 1, 0, 1, 1, 0, 1}
        uint8_t(0xDB), // {1, 1, 0, 1, 1, 0, 1, 1}
        uint8_t(0xAB), // {1, 0, 1, 0, 1, 0, 1, 1}
        uint8_t(0x96), // {1, 0, 0, 1, 0, 1, 1, 0}
        uint8_t(0x6A), // {0, 1, 1, 0, 1, 0, 1, 0}
        uint8_t(0x55), // {0, 1, 0, 1, 0, 1, 0, 1}
        uint8_t(0x33), // {0, 0, 1, 1, 0, 0, 1, 1}
        uint8_t(0xF),  // {0, 0, 0, 0, 1, 1, 1, 1}
        uint8_t(0x80), // {1, 0, 0, 0, 0, 0, 0, 0}
        uint8_t(0x40), // {0, 1, 0, 0, 0, 0, 0, 0}
        uint8_t(0x20), // {0, 0, 1, 0, 0, 0, 0, 0}
        uint8_t(0x10), // {0, 0, 0, 1, 0, 0, 0, 0}
        uint8_t(0x8),  // {0, 0, 0, 0, 1, 0, 0, 0}
        uint8_t(0x4),  // {0, 0, 0, 0, 0, 1, 0, 0}
        uint8_t(0x2),  // {0, 0, 0, 0, 0, 0, 1, 0}
        uint8_t(0x1),  // {0, 0, 0, 0, 0, 0, 0, 1}
};

uint8_t encodeByte(uint8_t byte) {
    uint8_t result = 0;

    for (int i = 0; i < 8; i++) {
        auto mtx = static_cast<uint8_t>(encodingMatrix[i] >> 8);
        uint8_t andOperation = mtx & byte;
        std::bitset<8> numberOfBits(andOperation);
        if (numberOfBits.count() % 2 != 0) {
            result |= (1 << (7 - i));
        }
    }
    return result;
}

uint8_t errorCorrection(uint8_t data, uint8_t errors) {
    for (int i = 0; i < 8; i++) {
        if (errors == decodingMatrix[i]) {
            data ^= 1 << (7 - i);
            return data;
        }
    }

    for (int i = 0; i < 16; i++) {
        for (int j = i; j < 16; j++) {
            uint8_t mistakeMask = decodingMatrix[i] ^ decodingMatrix[j];
            if (mistakeMask == errors) {
                data ^= 0x1 << (7 - i);
                if (i != j) {
                    data ^= 0x1 << (7 - j);
                }
            }
        }
    }
    return data;
}

uint8_t findErrors(uint16_t code) {
    uint8_t errors = 0;

    for (int i = 0; i < 8; i++) {
        std::bitset<16> bitset(code & encodingMatrix[i]);
        if ((bitset.count() % 2) != 0) {
            errors |= (1 << (7 - i));
        }
    }
    return errors;
}

uint8_t decodeByte(uint16_t code) {
    auto data = static_cast<uint8_t>(code >> 8);
    uint8_t errors = findErrors(code);

    if (errors == 0) {
        return data;
    }

    return errorCorrection(data, errors);
}

int main(int argc, char* argv[]) {
    bool encode = false;
    bool decode = false;
    bool file = false;

    int opt;
    std::string filename;
    while ((opt = getopt(argc, argv, "f:ed")) != -1) {
        switch (opt) {
            case 'f':
                file = true;
                filename = optarg;
                break;
            case 'e':
                encode = true;
                break;
            case 'd':
                decode = true;
                break;
            default:
                std::cerr << "Unknown option: " << opt << "\n";
                return 1;
        }
    }

    if (filename.empty() || !file) {
        std::cerr << "-f arg and file name required.\n";
        return 1;
    }

    if (!encode && !decode) {
        std::cerr << "Option -e or -d required.\n";
        return 1;
    }

    if (encode) {
        std::ifstream inputFile(filename, std::ios::binary);
        if (!inputFile) {
            std::cerr << "Error while opening the input file\n";
            return 1;
        }

        std::ofstream outputFile("e_" + filename, std::ios::binary);
        if (!outputFile) {
            std::cerr << "Error while opening the output file\n";
            return 1;
        }

        uint8_t byte;

        while (inputFile.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
            uint8_t encoded = encodeByte(byte);
            outputFile.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
            outputFile.write(reinterpret_cast<const char*>(&encoded), sizeof(encoded));
        }

        inputFile.close();
        outputFile.close();
    }

    if (decode) {
        std::ifstream inputFile(filename, std::ios::binary);
        if (!inputFile) {
            std::cerr << "Error while opening the input file\n";
            return 1;
        }

        std::ofstream outputFile("d_" + filename, std::ios::binary);
        if (!outputFile) {
            std::cerr << "Error while opening the output file\n";
            return 1;
        }

        uint16_t byte;
        uint16_t swap;

        while (inputFile.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
            swap = ((byte & 0xFF00) >> 8) | ((byte & 0x00FF) << 8);
            uint8_t decoded = decodeByte(swap);
            outputFile.write(reinterpret_cast<const char*>(&decoded), sizeof(decoded));
        }

        inputFile.close();
        outputFile.close();
    }
    return 0;
}
