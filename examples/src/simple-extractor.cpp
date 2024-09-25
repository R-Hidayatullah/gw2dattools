#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip> // For hex output
#include <cctype>  // For ASCII printing
#include <memory>  // For std::unique_ptr

#include <gw2dattools/interface/ANDatInterface.h>
#include <gw2dattools/compression/inflateDatFileBuffer.h>

// Helper function to print buffer data in separate lines for hex, number, and ASCII
void printBuffer(const uint8_t *buffer, uint32_t size, const std::string &label)
{
    std::cout << label << " (Hex): ";
    for (uint32_t i = 0; i < size && i < 15; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]) << " ";
    }
    std::cout << std::endl;

    std::cout << label << " (Number): ";
    for (uint32_t i = 0; i < size && i < 15; ++i)
    {
        std::cout << std::dec << static_cast<int>(buffer[i]) << " ";
    }
    std::cout << std::endl;

    std::cout << label << " (ASCII): ";
    for (uint32_t i = 0; i < size && i < 15; ++i)
    {
        std::cout << (std::isprint(buffer[i]) ? static_cast<char>(buffer[i]) : '.') << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    const uint32_t aBufferSize = 1024 * 1024 * 30; // Assume no file is bigger than 30 Mb
    auto datfile = "Local.dat";
    std::cout << "Filename: " << datfile << std::endl;

    uint32_t targetFileId = 16; // File ID

    auto pANDatInterface = gw2dt::datfile::createANDatInterface(datfile);
    auto aFileRecordVect = pANDatInterface->getFileRecordVect();
    std::cout << "Record Size: " << aFileRecordVect.size() << std::endl;

    // Use unique_ptr with array specialization to manage memory automatically
    std::unique_ptr<uint8_t[]> pOriBuffer = std::make_unique<uint8_t[]>(aBufferSize);
    std::unique_ptr<uint8_t[]> pInfBuffer = std::make_unique<uint8_t[]>(aBufferSize);

    for (const auto &it : aFileRecordVect)
    {
        if (it.fileId != targetFileId) // Process only the target file ID
        {
            continue;
        }

        uint32_t aOriSize = aBufferSize;
        pANDatInterface->getBuffer(it, aOriSize, pOriBuffer.get()); // Use get() to access the raw pointer

        std::cout << "Processing File: " << it.fileId << "\tFile Size: " << it.size << std::endl;

        // Print first 15 bytes of the original (possibly compressed) data
        printBuffer(pOriBuffer.get(), aOriSize, "Original Data");

        if (aOriSize == aBufferSize)
        {
            std::cout << "File " << it.fileId << " has a size greater than (or equal to) 30Mb." << std::endl;
        }

        if (it.isCompressed)
        {
            uint32_t aInfSize = aBufferSize;

            try
            {
                gw2dt::compression::inflateDatFileBuffer(aOriSize, pOriBuffer.get(), aInfSize, pInfBuffer.get());

                // Print first 15 bytes of the decompressed data
                printBuffer(pInfBuffer.get(), aInfSize, "Decompressed Data");
            }
            catch (std::exception &iException)
            {
                std::cout << "File " << it.fileId << " failed to decompress: " << std::string(iException.what()) << std::endl;
            }
        }

        break; // Exit after processing the specified file ID
    }

    return 0;
}
