#include "gw2dattools/compression/inflateDatFileBuffer.h"

#include <cstdlib>
#include <memory.h>
#include <iostream>

#include "gw2dattools/exception/Exception.h"

#include "HuffmanTree.h"
#include "../utils/BitArray.h"
#include <vector>

namespace gw2dt
{
    namespace compression
    {
        namespace dat
        {
            // Constants for Huffman decoding
            const uint32_t maxBitsForHash = 8;
            const uint32_t maxCodeBitsLength = 32;
            const uint32_t maxSymbolValue = 285;

            // Type definitions for bit arrays and Huffman trees
            typedef utils::BitArray<uint32_t> DatFileBitArray;
            using DatFileHuffmanTree = HuffmanTree<uint16_t, maxBitsForHash, maxCodeBitsLength, maxSymbolValue>;
            using DatFileHuffmanTreeBuilder = HuffmanTreeBuilder<uint16_t, maxCodeBitsLength, maxSymbolValue>;

            // Static Huffman tree dictionary
            static DatFileHuffmanTree huffmanTreeDictionary;

            // Parse and build a Huffman tree from input data
            bool parseHuffmanTree(DatFileBitArray &inputBitArray, DatFileHuffmanTree &huffmanTree, DatFileHuffmanTreeBuilder &huffmanTreeBuilder)
            {
                // Read the number of symbols
                uint16_t numberOfSymbols;
                inputBitArray.read(numberOfSymbols);
                inputBitArray.drop<uint16_t>();

                if (numberOfSymbols > maxSymbolValue)
                {
                    throw exception::Exception("Too many symbols to decode.");
                }

                huffmanTreeBuilder.clear();
                int16_t remainingSymbols = numberOfSymbols - 1;

                // Decode symbols from the bit array
                while (remainingSymbols >= 0)
                {
                    uint16_t code;
                    huffmanTreeDictionary.readCode(inputBitArray, code);

                    uint8_t codeBits = code & 0x1F;
                    uint16_t numSymbols = (code >> 5) + 1;

                    if (codeBits == 0)
                    {
                        remainingSymbols -= numSymbols; // No bits, so skip these symbols
                    }
                    else
                    {
                        while (numSymbols > 0)
                        {
                            huffmanTreeBuilder.addSymbol(remainingSymbols--, codeBits);
                            --numSymbols;
                        }
                    }
                }

                return huffmanTreeBuilder.buildHuffmanTree(huffmanTree);
            }

            // Inflate data from a compressed bit array into an output buffer
            void inflateData(DatFileBitArray &inputBitArray, uint32_t outputSize, uint8_t *outputBuffer)
            {
                uint32_t outputPos = 0;

                // Skip some initial bits and read a constant addition for write size
                inputBitArray.drop<4>();
                uint16_t writeSizeConstAdd;
                inputBitArray.read<4>(writeSizeConstAdd);
                writeSizeConstAdd += 1;
                inputBitArray.drop<4>();

                // Huffman trees for symbols and copy operations
                DatFileHuffmanTree huffmanTreeSymbol, huffmanTreeCopy;
                DatFileHuffmanTreeBuilder huffmanTreeBuilder;

                while (outputPos < outputSize)
                {
                    // Parse both Huffman trees
                    if (!parseHuffmanTree(inputBitArray, huffmanTreeSymbol, huffmanTreeBuilder) ||
                        !parseHuffmanTree(inputBitArray, huffmanTreeCopy, huffmanTreeBuilder))
                    {
                        break;
                    }

                    // Read the max count of codes
                    uint32_t maxCount;
                    inputBitArray.read<4>(maxCount);
                    maxCount = (maxCount + 1) << 12;
                    inputBitArray.drop<4>();

                    uint32_t codeReadCount = 0;

                    while (codeReadCount < maxCount && outputPos < outputSize)
                    {
                        ++codeReadCount;

                        // Read the next symbol code
                        uint16_t symbol = 0;
                        huffmanTreeSymbol.readCode(inputBitArray, symbol);

                        if (symbol < 0x100)
                        {
                            // Directly write the symbol as a byte
                            outputBuffer[outputPos++] = static_cast<uint8_t>(symbol);
                            continue;
                        }

                        // Handle copy mode for repeated data
                        symbol -= 0x100;

                        // Calculate write size
                        div_t symbolDiv4 = div(symbol, 4);
                        uint32_t writeSize = (symbolDiv4.quot == 0) ? symbol
                                                                    : (symbolDiv4.quot < 7 ? ((1 << (symbolDiv4.quot - 1)) * (4 + symbolDiv4.rem))
                                                                                           : (symbol == 28 ? 0xFF : throw exception::Exception("Invalid write size code.")));

                        // Handle additional bits for write size
                        if (symbolDiv4.quot > 1 && symbol != 28)
                        {
                            uint8_t additionalBits = static_cast<uint8_t>(symbolDiv4.quot - 1);
                            uint32_t additionalSize;
                            inputBitArray.read(additionalBits, additionalSize);
                            writeSize |= additionalSize;
                            inputBitArray.drop(additionalBits);
                        }
                        writeSize += writeSizeConstAdd;

                        // Calculate write offset
                        huffmanTreeCopy.readCode(inputBitArray, symbol);
                        div_t symbolDiv2 = div(symbol, 2);
                        uint32_t writeOffset = (symbolDiv2.quot == 0) ? symbol
                                                                      : (symbolDiv2.quot < 17 ? ((1 << (symbolDiv2.quot - 1)) * (2 + symbolDiv2.rem))
                                                                                              : throw exception::Exception("Invalid write offset code."));

                        // Handle additional bits for write offset
                        if (symbolDiv2.quot > 1)
                        {
                            uint8_t offsetAddBits = static_cast<uint8_t>(symbolDiv2.quot - 1);
                            uint32_t offsetAdd;
                            inputBitArray.read(offsetAddBits, offsetAdd);
                            writeOffset |= offsetAdd;
                            inputBitArray.drop(offsetAddBits);
                        }
                        writeOffset += 1;

                        // Copy the data to the output buffer
                        for (uint32_t i = 0; i < writeSize && outputPos < outputSize; ++i)
                        {
                            outputBuffer[outputPos] = outputBuffer[outputPos - writeOffset];
                            ++outputPos;
                        }
                    }
                }
            }

        }

        GW2DATTOOLS_API uint8_t *GW2DATTOOLS_APIENTRY inflateDatFileBuffer(
            uint32_t inputSize,
            const uint8_t *inputBuffer,
            uint32_t &outputSize,
            uint8_t *outputBuffer)
        {
            if (inputBuffer == nullptr)
            {
                throw exception::Exception("Input buffer is null.");
            }

            if (outputBuffer != nullptr && outputSize == 0)
            {
                throw exception::Exception("Output buffer is not null, but output size is undefined.");
            }

            uint8_t *finalOutputBuffer = nullptr;
            bool ownsBuffer = true; // Flag to track if memory needs to be freed

            try
            {
                dat::DatFileBitArray inputBitArray(inputBuffer, inputSize, 16384);
                inputBitArray.drop<uint32_t>(); // Skip header
                uint32_t uncompressedSize;
                inputBitArray.read(uncompressedSize);
                std::cout << "\nUncompressed Size: " << uncompressedSize << "\n\n"
                          << std::endl;

                inputBitArray.drop<uint32_t>(); // Skip another header part

                if (outputSize != 0)
                {
                    uncompressedSize = std::min(uncompressedSize, outputSize);
                }

                outputSize = uncompressedSize; // Update output size

                if (outputBuffer == nullptr)
                {
                    finalOutputBuffer = static_cast<uint8_t *>(malloc(uncompressedSize));
                    if (finalOutputBuffer == nullptr)
                    {
                        throw std::bad_alloc();
                    }
                }
                else
                {
                    ownsBuffer = false;
                    finalOutputBuffer = outputBuffer;
                }

                dat::inflateData(inputBitArray, uncompressedSize, finalOutputBuffer);

                return finalOutputBuffer;
            }
            catch (...)
            {
                if (ownsBuffer && finalOutputBuffer != nullptr)
                {
                    free(finalOutputBuffer);
                }
                throw; // Rethrow any exception
            }
        }

        class DatFileHuffmanTreeDictStaticInitializer
        {
        public:
            DatFileHuffmanTreeDictStaticInitializer(dat::DatFileHuffmanTree &ioHuffmanTree);
        };

        DatFileHuffmanTreeDictStaticInitializer::DatFileHuffmanTreeDictStaticInitializer(dat::DatFileHuffmanTree &ioHuffmanTree)
        {
            dat::DatFileHuffmanTreeBuilder aDatFileHuffmanTreeBuilder;
            aDatFileHuffmanTreeBuilder.clear();

            // Define symbols and their corresponding bit lengths
            std::vector<int> symbols = {0x0A, 0x09, 0x08, 0x0C, 0x0B, 0x07, 0x00, 0xE0, 0x2A, 0x29, 0x06, 0x4A, 0x40, 0x2C, 0x2B,
                                        0x28, 0x20, 0x05, 0x04, 0x49, 0x48, 0x27, 0x26, 0x25, 0x0D, 0x03, 0x6A, 0x69, 0x4C, 0x4B,
                                        0x47, 0x24, 0xE8, 0xA0, 0x89, 0x88, 0x68, 0x67, 0x63, 0x60, 0x46, 0x23, 0xE9, 0xC9, 0xC0,
                                        0xA9, 0xA8, 0x8A, 0x87, 0x80, 0x66, 0x65, 0x45, 0x44, 0x43, 0x2D, 0x02, 0x01, 0xE5, 0xC8,
                                        0xAA, 0xA5, 0xA4, 0x8B, 0x85, 0x84, 0x6C, 0x6B, 0x64, 0x4D, 0x0E, 0xE7, 0xCA, 0xC7, 0xA7,
                                        0xA6, 0x86, 0x83, 0xE6, 0xE4, 0xC4, 0x8C, 0x2E, 0x22, 0xEC, 0xC6, 0x6D, 0x4E, 0xEA, 0xCC,
                                        0xAC, 0xAB, 0x8D, 0x11, 0x10, 0x0F, 0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0xF7,
                                        0xF6, 0xF5, 0xF4, 0xF3, 0xF2, 0xF1, 0xF0, 0xEF, 0xEE, 0xED, 0xEB, 0xE3, 0xE2, 0xE1, 0xDF,
                                        0xDE, 0xDD, 0xDC, 0xDB, 0xDA, 0xD9, 0xD8, 0xD7, 0xD6, 0xD5, 0xD4, 0xD3, 0xD2, 0xD1, 0xD0,
                                        0xCF, 0xCE, 0xCD, 0xCB, 0xC5, 0xC3, 0xC2, 0xC1, 0xBF, 0xBE, 0xBD, 0xBC, 0xBB, 0xBA, 0xB9,
                                        0xB8, 0xB7, 0xB6, 0xB5, 0xB4, 0xB3, 0xB2, 0xB1, 0xB0, 0xAF, 0xAE, 0xAD, 0xA3, 0xA2, 0xA1,
                                        0x9F, 0x9E, 0x9D, 0x9C, 0x9B, 0x9A, 0x99, 0x98, 0x97, 0x96, 0x95, 0x94, 0x93, 0x92, 0x91,
                                        0x90, 0x8F, 0x8E, 0x82, 0x81, 0x7F, 0x7E, 0x7D, 0x7C, 0x7B, 0x7A, 0x79, 0x78, 0x77, 0x76,
                                        0x75, 0x74, 0x73, 0x72, 0x71, 0x70, 0x6F, 0x6E, 0x62, 0x61, 0x5F, 0x5E, 0x5D, 0x5C, 0x5B,
                                        0x5A, 0x59, 0x58, 0x57, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51, 0x50, 0x4F, 0x42, 0x41, 0x3F,
                                        0x3E, 0x3D, 0x3C, 0x3B, 0x3A, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30,
                                        0x2F, 0x21, 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13,
                                        0x12};
            std::vector<int> bitLengths = {3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8,
                                           8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

            // Add each symbol with its corresponding bit length
            for (size_t i = 0; i < symbols.size(); ++i)
            {
                aDatFileHuffmanTreeBuilder.addSymbol(symbols[i], bitLengths[i]);
            }

            aDatFileHuffmanTreeBuilder.buildHuffmanTree(ioHuffmanTree);
        }

        static DatFileHuffmanTreeDictStaticInitializer aDatFileHuffmanTreeDictStaticInitializer(dat::huffmanTreeDictionary);

    }
}
