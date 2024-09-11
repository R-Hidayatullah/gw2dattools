#ifndef GW2DATTOOLS_COMPRESSION_HUFFMANTREEUTILS_H
#define GW2DATTOOLS_COMPRESSION_HUFFMANTREEUTILS_H

#include "gw2dattools/exception/Exception.h"
#include <cstdint>
#include <cstring> // For memset

namespace gw2dt
{
    namespace compression
    {

        static constexpr uint32_t MaxCodeBitsLength = 32; // Max number of bits per code
        static constexpr uint32_t MaxSymbolValue = 285;   // Max value for a symbol
        static constexpr uint32_t MaxNbBitsHash = 8;

        struct HuffmanTree
        {
            uint32_t codeCompTab[MaxCodeBitsLength] = {0};
            uint16_t symbolValueTabOffsetTab[MaxCodeBitsLength] = {0};
            uint16_t symbolValueTab[MaxSymbolValue] = {0};
            uint8_t codeBitsTab[MaxCodeBitsLength] = {0};

            int16_t symbolValueHashTab[1 << MaxNbBitsHash] = {-1};
            uint8_t codeBitsHashTab[1 << MaxNbBitsHash] = {0};

            bool isEmpty = true;
        };

        struct State
        {
            const uint32_t *input = nullptr;
            uint32_t inputSize = 0;
            uint32_t inputPos = 0;

            uint32_t head = 0;
            uint32_t buffer = 0;
            uint8_t bits = 0;

            bool isEmpty = false;
        };

        void buildHuffmanTree(HuffmanTree &ioHuffmanTree, int16_t *ioWorkingBitTab, int16_t *ioWorkingCodeTab);
        void fillWorkingTabsHelper(uint8_t iBits, int16_t iSymbol, int16_t *ioWorkingBitTab, int16_t *ioWorkingCodeTab);

        void readCode(const HuffmanTree &iHuffmanTree, State &ioState, uint16_t &ioCode);

        inline void pullByte(State &ioState)
        {
            if (ioState.bits >= 32)
            {
                throw exception::Exception("Tried to pull a value while we still have 32 bits available.");
            }

            if ((ioState.inputPos + 1) % 0x4000 == 0)
            {
                ++(ioState.inputPos);
            }

            uint32_t aValue = 0;

            if (ioState.inputPos >= ioState.inputSize)
            {
                if (ioState.isEmpty)
                {
                    throw exception::Exception("Reached end of input while trying to fetch a new byte.");
                }
                ioState.isEmpty = true;
            }
            else
            {
                aValue = ioState.input[ioState.inputPos];
            }

            if (ioState.bits == 0)
            {
                ioState.head = aValue;
                ioState.buffer = 0;
            }
            else
            {
                ioState.head |= (aValue >> ioState.bits);
                ioState.buffer = (aValue << (32 - ioState.bits));
            }

            ioState.bits += 32;
            ++(ioState.inputPos);
        }

        inline void needBits(State &ioState, uint8_t iBits)
        {
            if (iBits > 32)
            {
                throw exception::Exception("Tried to need more than 32 bits.");
            }

            if (ioState.bits < iBits)
            {
                pullByte(ioState);
            }
        }

        inline void dropBits(State &ioState, uint8_t iBits)
        {
            if (iBits > 32)
            {
                throw exception::Exception("Tried to drop more than 32 bits.");
            }

            if (iBits > ioState.bits)
            {
                throw exception::Exception("Tried to drop more bits than we have.");
            }

            if (iBits == 32)
            {
                ioState.head = ioState.buffer;
                ioState.buffer = 0;
            }
            else
            {
                ioState.head <<= iBits;
                ioState.head |= (ioState.buffer) >> (32 - iBits);
                ioState.buffer <<= iBits;
            }

            ioState.bits -= iBits;
        }

        inline uint32_t readBits(const State &iState, uint8_t iBits)
        {
            return (iState.head) >> (32 - iBits);
        }

    } // namespace compression
} // namespace gw2dt

#endif // GW2DATTOOLS_COMPRESSION_HUFFMANTREEUTILS_H