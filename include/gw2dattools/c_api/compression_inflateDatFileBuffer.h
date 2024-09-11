#ifndef GW2DATTOOLS_CAPI_COMPRESSION_INFLATEBUFFER_H
#define GW2DATTOOLS_CAPI_COMPRESSION_INFLATEBUFFER_H

#include <stdint.h>
#include <stdlib.h>
#include "gw2dattools/dllMacros.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Inflates a compressed buffer.
     *
     * This function decompresses data from a compressed input buffer into an output buffer.
     * If the output buffer is not provided, or if `ioOutputSize` is 0, the function will
     * allocate memory for the output buffer internally. The caller is responsible for freeing
     * this memory if it is allocated.
     *
     * @param iInputSize   Size of the input buffer in bytes.
     * @param iInputTab    Pointer to the compressed input buffer.
     * @param ioOutputSize Pointer to the size of the output buffer. If `*ioOutputSize` is 0,
     *                     the function will decode as much as possible and return the size of
     *                     the decompressed data in `*ioOutputSize`. If `*ioOutputSize` is non-zero,
     *                     the function will decode up to this size.
     * @param ioOutputTab  Optional pointer to an output buffer. If provided, this buffer must
     *                     be large enough to hold at least `*ioOutputSize` bytes of decompressed
     *                     data. If NULL, the function will allocate the output buffer internally.
     *                     If the function allocates memory, the caller must free it.
     * @return uint8_t*    Pointer to the output buffer, which may be different from `ioOutputTab`
     *                     if memory was allocated internally. Returns NULL if decompression fails.
     */
    GW2DATTOOLS_API uint8_t *GW2DATTOOLS_APIENTRY compression_inflateBuffer(
        const uint32_t iInputSize,
        const uint8_t *iInputTab,
        uint32_t *ioOutputSize,
        uint8_t *ioOutputTab = NULL);

#ifdef __cplusplus
}
#endif

#endif // GW2DATTOOLS_CAPI_COMPRESSION_INFLATEBUFFER_H
