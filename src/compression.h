#ifndef COMPRESSION_H
#define COMPRESSION_H
#include <vector>
#include "typedef.h"

namespace Compression
{
    ///////////////////////////////////////////////////////////////////////////////
    // Compress a vector of arbitrary lenght using deflate compression from the
    // zlib general purpose compression library (v 1.2.8)
    //=============================================================================
    // The output vector contains a small header section that stores the size of
    // the original data and the amount of compressed bytes that follow.
    // In case of failure, this function returns an empty vector.
    ///////////////////////////////////////////////////////////////////////////////
    std::vector<u8> compress(const std::vector<u8> &source);

    ///////////////////////////////////////////////////////////////////////////////
    // Extract a vector of data previously compressed using the compress() utility,
    // using the zlib general purpose compression library (v 1.2.8).
    //=============================================================================
    // The source vector must be larger or equal in size to the amount of
    // compressed data originally compressed.
    // In case of failure, this function returns an empty vector.
    ///////////////////////////////////////////////////////////////////////////////
    std::vector<u8> extract(const std::vector<u8> &source);

    ///////////////////////////////////////////////////////////////////////////////
    // Return the uncompressed size of a vector containing data previously
    // compressed using the compress() utility.
    //=============================================================================
    // In case of failure, this function returns 0;
    ///////////////////////////////////////////////////////////////////////////////
    memory_index getExtractedSize(const std::vector<u8> &source);
}

#endif // COMPRESSION_H
