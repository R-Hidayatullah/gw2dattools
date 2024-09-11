#ifndef GW2DATTOOLS_COMPRESSION_HUFFMANTREE_H
#define GW2DATTOOLS_COMPRESSION_HUFFMANTREE_H

#include <array>
#include <cstdint>
#include "../utils/BitArray.h"

namespace gw2dt
{
    namespace compression
    {

        template <typename SymbolType,
                  uint8_t sMaxCodeBitsLength,
                  uint16_t sMaxSymbolValue>
        class HuffmanTreeBuilder;

        /**
         * Represents a Huffman Tree.
         * @tparam SymbolType The type of symbol stored in the Huffman Tree.
         * @tparam sNbBitsHash Number of bits used for the hash table.
         * @tparam sMaxCodeBitsLength Maximum length of the Huffman code in bits.
         * @tparam sMaxSymbolValue Maximum value of the symbol.
         */
        template <typename SymbolType,
                  uint8_t sNbBitsHash,
                  uint8_t sMaxCodeBitsLength,
                  uint16_t sMaxSymbolValue>
        class HuffmanTree
        {
        public:
            friend class HuffmanTreeBuilder<SymbolType, sMaxCodeBitsLength, sMaxSymbolValue>;

            /**
             * Reads a Huffman code from the bit array and decodes it to a symbol.
             * @param iBitArray Bit array from which the Huffman code is read.
             * @param oSymbol Decoded symbol.
             */
            template <typename IntType>
            void readCode(utils::BitArray<IntType> &iBitArray, SymbolType &oSymbol) const;

        private:
            void clear();

            std::array<uint32_t, sMaxCodeBitsLength> _codeComparisonArray;
            std::array<uint16_t, sMaxCodeBitsLength> _symbolValueArrayOffsetArray;
            std::array<SymbolType, sMaxSymbolValue> _symbolValueArray;
            std::array<uint8_t, sMaxCodeBitsLength> _codeBitsArray;

            std::array<bool, (1 << sNbBitsHash)> _symbolValueHashExistenceArray;
            std::array<SymbolType, (1 << sNbBitsHash)> _symbolValueHashArray;
            std::array<uint8_t, (1 << sNbBitsHash)> _codeBitsHashArray;
        };

        /**
         * Builds a Huffman Tree.
         * @tparam SymbolType The type of symbol stored in the Huffman Tree.
         * @tparam sMaxCodeBitsLength Maximum length of the Huffman code in bits.
         * @tparam sMaxSymbolValue Maximum value of the symbol.
         */
        template <typename SymbolType,
                  uint8_t sMaxCodeBitsLength,
                  uint16_t sMaxSymbolValue>
        class HuffmanTreeBuilder
        {
        public:
            void clear();

            /**
             * Adds a symbol to the Huffman Tree.
             * @param iSymbol Symbol to be added.
             * @param iNbBits Number of bits for the Huffman code.
             */
            void addSymbol(SymbolType iSymbol, uint8_t iNbBits);

            /**
             * Builds a Huffman Tree from the accumulated symbols.
             * @tparam sNbBitsHash Number of bits used for the hash table.
             * @param oHuffmanTree The resulting Huffman Tree.
             * @return True if the Huffman Tree was built successfully, otherwise false.
             */
            template <uint8_t sNbBitsHash>
            bool buildHuffmanTree(HuffmanTree<SymbolType, sNbBitsHash, sMaxCodeBitsLength, sMaxSymbolValue> &oHuffmanTree);

        private:
            bool empty() const;

            std::array<bool, sMaxCodeBitsLength> _symbolListByBitsHeadExistenceArray;
            std::array<SymbolType, sMaxCodeBitsLength> _symbolListByBitsHeadArray;

            std::array<bool, sMaxSymbolValue> _symbolListByBitsBodyExistenceArray;
            std::array<SymbolType, sMaxSymbolValue> _symbolListByBitsBodyArray;
        };

    } // namespace compression
} // namespace gw2dt

#include "HuffmanTree.i"

#endif // GW2DATTOOLS_COMPRESSION_HUFFMANTREE_H
