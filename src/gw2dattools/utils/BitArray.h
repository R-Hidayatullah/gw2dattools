#ifndef GW2DATTOOLS_UTILS_BITARRAY_H
#define GW2DATTOOLS_UTILS_BITARRAY_H

#include <cstdint>
#include <cassert>

namespace gw2dt
{
    namespace utils
    {

        /**
         * @brief A class template for managing a sequence of bits in a buffer.
         * @tparam IntType The type used to store the bits.
         */
        template <typename IntType>
        class BitArray
        {
        public:
            /**
             * @brief Constructs a BitArray from a given buffer.
             * @param ipBuffer Pointer to the buffer containing the bit data.
             * @param iSize The size of the buffer in bytes.
             * @param iSkippedBytes The number of bytes to skip at the beginning of the buffer.
             */
            BitArray(const uint8_t *ipBuffer, uint32_t iSize, uint32_t iSkippedBytes = 0);

            /**
             * @brief Reads a value lazily from the bit array.
             * @param iBitNumber The number of bits to read.
             * @param oValue The output value.
             */
            template <typename OutputType>
            void readLazy(uint8_t iBitNumber, OutputType &oValue) const;

            /**
             * @brief Reads a value lazily from the bit array using a compile-time constant bit number.
             * @tparam isBitNumber The number of bits to read.
             * @param oValue The output value.
             */
            template <uint8_t isBitNumber, typename OutputType>
            void readLazy(OutputType &oValue) const;

            /**
             * @brief Reads a value lazily from the bit array.
             * @param oValue The output value.
             */
            template <typename OutputType>
            void readLazy(OutputType &oValue) const;

            /**
             * @brief Reads a value from the bit array.
             * @param iBitNumber The number of bits to read.
             * @param oValue The output value.
             */
            template <typename OutputType>
            void read(uint8_t iBitNumber, OutputType &oValue) const;

            /**
             * @brief Reads a value from the bit array using a compile-time constant bit number.
             * @tparam isBitNumber The number of bits to read.
             * @param oValue The output value.
             */
            template <uint8_t isBitNumber, typename OutputType>
            void read(OutputType &oValue) const;

            /**
             * @brief Reads a value from the bit array.
             * @param oValue The output value.
             */
            template <typename OutputType>
            void read(OutputType &oValue) const;

            /**
             * @brief Drops a number of bits from the beginning of the bit array.
             * @param iBitNumber The number of bits to drop.
             */
            void drop(uint8_t iBitNumber);

            /**
             * @brief Drops a number of bits from the beginning of the bit array using a compile-time constant bit number.
             * @tparam isBitNumber The number of bits to drop.
             */
            template <uint8_t isBitNumber>
            void drop();

            /**
             * @brief Drops a number of bits from the beginning of the bit array.
             * @tparam OutputType The type of the value to be dropped.
             */
            template <typename OutputType>
            void drop();

        private:
            /**
             * @brief Implementation of the read operation.
             * @param iBitNumber The number of bits to read.
             * @param oValue The output value.
             */
            template <typename OutputType>
            void readImpl(uint8_t iBitNumber, OutputType &oValue) const;

            /**
             * @brief Implementation of the drop operation.
             * @param iBitNumber The number of bits to drop.
             */
            void dropImpl(uint8_t iBitNumber);

            /**
             * @brief Pulls a value from the buffer.
             * @param oValue The output value.
             * @param oNbPulledBits The number of bits pulled.
             */
            void pull(IntType &oValue, uint8_t &oNbPulledBits);

            const uint8_t *const _pBufferStartPos; ///< Pointer to the start of the buffer.
            const uint8_t *_pBufferPos;            ///< Current position in the buffer.
            uint32_t _bytesAvail;                  ///< Number of bytes available for reading.

            uint32_t _skippedBytes; ///< Number of skipped bytes.

            IntType _head;      ///< The head of the buffer.
            IntType _buffer;    ///< The buffer for bit operations.
            uint8_t _bitsAvail; ///< Number of bits available in the buffer.
        };

    }
}

#include "BitArray.i"

#endif // GW2DATTOOLS_UTILS_BITARRAY_H
