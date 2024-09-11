
#include "huffmanTreeUtils.h"

namespace gw2dt
{
    namespace compression
    {

        void readCode(const HuffmanTree &iHuffmanTree, State &ioState, uint16_t &ioCode)
        {
            if (iHuffmanTree.isEmpty)
            {
                throw exception::Exception("Trying to read code from an empty HuffmanTree.");
            }

            needBits(ioState, MaxNbBitsHash);

            uint32_t hash = readBits(ioState, MaxNbBitsHash);
            if (iHuffmanTree.symbolValueHashTab[hash] != -1)
            {
                ioCode = iHuffmanTree.symbolValueHashTab[hash];
                dropBits(ioState, iHuffmanTree.codeBitsHashTab[hash]);
            }
            else
            {
                uint16_t anIndex = 0;
                while (readBits(ioState, 32) >= iHuffmanTree.codeCompTab[anIndex])
                {
                    ++anIndex;
                }

                uint8_t aNbBits = iHuffmanTree.codeBitsTab[anIndex];
                ioCode = iHuffmanTree.symbolValueTab[iHuffmanTree.symbolValueTabOffsetTab[anIndex] -
                                                     ((readBits(ioState, 32) - iHuffmanTree.codeCompTab[anIndex]) >> (32 - aNbBits))];
                dropBits(ioState, aNbBits);
            }
        }

        void buildHuffmanTree(HuffmanTree &ioHuffmanTree, int16_t *ioWorkingBitTab, int16_t *ioWorkingCodeTab)
        {
            std::memset(&ioHuffmanTree.codeCompTab, 0, sizeof(ioHuffmanTree.codeCompTab));
            std::memset(&ioHuffmanTree.symbolValueTabOffsetTab, 0, sizeof(ioHuffmanTree.symbolValueTabOffsetTab));
            std::memset(&ioHuffmanTree.symbolValueTab, 0, sizeof(ioHuffmanTree.symbolValueTab));
            std::memset(&ioHuffmanTree.codeBitsTab, 0, sizeof(ioHuffmanTree.codeBitsTab));
            std::memset(&ioHuffmanTree.codeBitsHashTab, 0, sizeof(ioHuffmanTree.codeBitsHashTab));
            std::memset(&ioHuffmanTree.symbolValueHashTab, 0xFF, sizeof(ioHuffmanTree.symbolValueHashTab));

            ioHuffmanTree.isEmpty = true;

            uint32_t aCode = 0;
            uint8_t aNbBits = 0;

            while (aNbBits <= MaxNbBitsHash)
            {
                if (ioWorkingBitTab[aNbBits] != -1)
                {
                    ioHuffmanTree.isEmpty = false;

                    int16_t aCurrentSymbol = ioWorkingBitTab[aNbBits];
                    while (aCurrentSymbol != -1)
                    {
                        uint16_t aHashValue = static_cast<uint16_t>(aCode << (MaxNbBitsHash - aNbBits));
                        uint16_t aNextHashValue = static_cast<uint16_t>((aCode + 1) << (MaxNbBitsHash - aNbBits));

                        while (aHashValue < aNextHashValue)
                        {
                            ioHuffmanTree.symbolValueHashTab[aHashValue] = aCurrentSymbol;
                            ioHuffmanTree.codeBitsHashTab[aHashValue] = aNbBits;
                            ++aHashValue;
                        }

                        aCurrentSymbol = ioWorkingCodeTab[aCurrentSymbol];
                        --aCode;
                    }
                }
                aCode = (aCode << 1) + 1;
                ++aNbBits;
            }

            uint16_t aCodeCompTabIndex = 0;
            uint16_t aSymbolOffset = 0;

            while (aNbBits < MaxCodeBitsLength)
            {
                if (ioWorkingBitTab[aNbBits] != -1)
                {
                    ioHuffmanTree.isEmpty = false;

                    int16_t aCurrentSymbol = ioWorkingBitTab[aNbBits];
                    while (aCurrentSymbol != -1)
                    {
                        ioHuffmanTree.symbolValueTab[aSymbolOffset] = aCurrentSymbol;
                        ++aSymbolOffset;
                        aCurrentSymbol = ioWorkingCodeTab[aCurrentSymbol];
                        --aCode;
                    }

                    ioHuffmanTree.codeCompTab[aCodeCompTabIndex] = ((aCode + 1) << (32 - aNbBits));
                    ioHuffmanTree.codeBitsTab[aCodeCompTabIndex] = aNbBits;
                    ioHuffmanTree.symbolValueTabOffsetTab[aCodeCompTabIndex] = aSymbolOffset - 1;

                    ++aCodeCompTabIndex;
                }
                aCode = (aCode << 1) + 1;
                ++aNbBits;
            }
        }

        void fillWorkingTabsHelper(uint8_t iBits, int16_t iSymbol, int16_t *ioWorkingBitTab, int16_t *ioWorkingCodeTab)
        {
            if (iBits > MaxCodeBitsLength)
            {
                throw exception::Exception("Attempted to fill a working tab with more than MaxCodeBitsLength bits.");
            }

            if (iSymbol < 0)
            {
                throw exception::Exception("Attempted to fill working tabs with a negative symbol.");
            }

            if (ioWorkingBitTab[iBits] == -1)
            {
                ioWorkingBitTab[iBits] = iSymbol;
                ioWorkingCodeTab[iSymbol] = -1;
            }
            else
            {
                int16_t aCurrentSymbol = ioWorkingBitTab[iBits];
                while (aCurrentSymbol != -1)
                {
                    aCurrentSymbol = ioWorkingCodeTab[aCurrentSymbol];
                }
                ioWorkingCodeTab[aCurrentSymbol] = iSymbol;
            }
        }
    } // namespace compression
} // namespace gw2dt