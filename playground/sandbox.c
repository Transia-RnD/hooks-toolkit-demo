#include <iostream>
#include <cstdint>
#include <vector>

std::vector<std::vector<uint8_t> > processBuffer(uint8_t* buffer, size_t bufferSize) {
    // Get the length of the index
    uint8_t indexLength = 20;

    std::cout << "indexLength: " << static_cast<int>(indexLength) << "\n";

    // Calculate the number of indexes in the buffer
    size_t numIndexes = bufferSize / indexLength;

    std::cout << "numIndexes: " << numIndexes << "\n";

    // Create a vector to store the index buffers
    std::vector<std::vector<uint8_t> > indexBuffers;

    // Loop over the indexes
    for (size_t i = 0; i < numIndexes; i++) {
        // Calculate the start position of the current index
        size_t startIndex = i * indexLength;

        // Create a new index buffer
        std::vector<uint8_t> indexBuffer;

        // Copy the bytes from the buffer to the index buffer (excluding the first byte)
        for (size_t j = 1; j < indexLength; j++) {
            indexBuffer.push_back(buffer[startIndex + j]);
        }

        std::cout << "indexBuffer: " << indexBuffer.size() << "\n";

        // Add the index buffer to the vector
        indexBuffers.push_back(indexBuffer);
    }

    // Return the vector of index buffers
    return indexBuffers;
}

int main() {
    uint8_t buffer[21] = {0x20, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13};
    size_t bufferSize = sizeof(buffer);
    std::cout << "bufferSize " << bufferSize << "\n";

    std::vector<std::vector<uint8_t> > indexBuffers = processBuffer(buffer, bufferSize);

    std::cout << "Indexs " << indexBuffers.size() << "\n";

    // Print the index buffers
    for (size_t i = 0; i < indexBuffers.size(); i++) {
        std::cout << "Index " << i + 1 << ": ";
        for (size_t j = 0; j < indexBuffers[i].size(); j++) {
            std::cout << std::hex << static_cast<int>(indexBuffers[i][j]) << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}