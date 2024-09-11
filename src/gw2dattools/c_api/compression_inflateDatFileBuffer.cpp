// Including necessary headers for buffer inflation
#include "gw2dattools/c_api/compression_inflateDatFileBuffer.h"
#include <cstdio>
#include <exception>
#include "gw2dattools/compression/inflateDatFileBuffer.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief C API function to inflate a DAT file buffer.
     *
     * @param iInputSize   Size of the input buffer.
     * @param iInputTab    Pointer to the input buffer to inflate.
     * @param ioOutputSize Pointer to the size of the output buffer.
     * @param ioOutputTab  Optional output buffer. If provided, ioOutputSize must be <= size of this buffer.
     * @return uint8_t*    Pointer to the inflated output buffer; returns NULL if decompression fails.
     */
    GW2DATTOOLS_API uint8_t *GW2DATTOOLS_APIENTRY compression_inflateDatFileBuffer(
        const uint32_t iInputSize,
        uint8_t *iInputTab,
        uint32_t *ioOutputSize,
        uint8_t *ioOutputTab)
    {
        // Check if ioOutputSize is valid
        if (ioOutputSize == nullptr)
        {
            printf("GW2DATTOOLS_C_API(compression_inflateDatFileBuffer): ioOutputSize is NULL.\n");
            return NULL;
        }

        try
        {
            // Attempt to inflate the buffer
            return gw2dt::compression::inflateDatFileBuffer(
                iInputSize,
                iInputTab,
                *ioOutputSize,
                ioOutputTab == NULL ? nullptr : ioOutputTab);
        }
        catch (const std::exception &e)
        {
            // Catch and log exceptions
            printf("GW2DATTOOLS_C_API(compression_inflateDatFileBuffer): %s\n", e.what());
            return NULL;
        }
    }

#ifdef __cplusplus
}
#endif