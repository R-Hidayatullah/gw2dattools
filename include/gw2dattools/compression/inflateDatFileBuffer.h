#ifndef GW2DATTOOLS_COMPRESSION_INFLATEDATFILEBUFFER_H
#define GW2DATTOOLS_COMPRESSION_INFLATEDATFILEBUFFER_H

#include <cstdint>
#include <string>
#include "gw2dattools/dllMacros.h"

namespace gw2dt
{
    namespace compression
    {

        /**
         * @brief Inflates a compressed data buffer.
         *
         * This function decompresses data from a compressed input buffer and writes it to an output buffer.
         * If the provided output buffer is null or insufficiently sized, the function will allocate the
         * necessary memory internally. The caller is responsible for freeing this memory if it is allocated
         * internally.
         *
         * @param iInputSize   Size of the input buffer in bytes.
         * @param iInputTab    Pointer to the compressed input buffer.
         * @param ioOutputSize Reference to the size of the output buffer. On input, this specifies the maximum
         *                     number of bytes to decode if non-zero. On output, this will contain the actual
         *                     size of the decompressed data.
         * @param ioOutputTab  Optional pointer to an output buffer. If provided, this buffer must be large enough
         *                     to hold at least `*ioOutputSize` bytes of decompressed data. If null, the function
         *                     will allocate memory internally.
         * @return uint8_t*    Pointer to the output buffer, which may be different from `ioOutputTab` if memory
         *                     was allocated internally. Returns nullptr if decompression fails.
         * @throws std::exception If decompression fails due to invalid parameters or data.
         */
        GW2DATTOOLS_API uint8_t *GW2DATTOOLS_APIENTRY inflateDatFileBuffer(
            uint32_t iInputSize,
            const uint8_t *iInputTab,
            uint32_t &ioOutputSize,
            uint8_t *ioOutputTab = nullptr);

    } // namespace compression
} // namespace gw2dt

#endif // GW2DATTOOLS_COMPRESSION_INFLATEDATFILEBUFFER_H
