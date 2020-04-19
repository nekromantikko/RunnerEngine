#include <vector>
#include <zlib.h>

#include "compression.h"

//Header section added to the beginning of the output vector when data is
//..compressed using compress() and removed during extraction
struct DataHeader
{
    u32 extractedSize;
    u32 compressedSize;
};

///////////////////////////////////////////////////////////////////////////////
// Compress a vector of arbitrary lenght using deflate compression from the
// zlib general purpose compression library (v 1.2.8)
//=============================================================================
// The output vector contains a small header section that stores the size of
// the original data and the amount of compressed bytes that follow.
// In case of failure, this function returns an empty vector.
///////////////////////////////////////////////////////////////////////////////
std::vector<u8> Compression::compress(const std::vector<u8> &source)
{
    //the output vector
    std::vector<u8> compressed;

    //initialize the header and store the size of the source data
    DataHeader header;
    header.extractedSize = source.size();

    //Initialize the zlib stream struct
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    //set the stream input size as the size of the uncompressed data vector
    //.."source", and give a pointer to the first byte of the vector.
    strm.next_in = (u8 *)source.data();
    strm.avail_in = source.size();

    //Initialize the stream state for deflation
    s32 retval = deflateInit(&strm, Z_BEST_COMPRESSION);
    if(retval != Z_OK)
    {
        deflateEnd(&strm);
        compressed.clear();
        return compressed;
    }

    //estimate the maximum size of the compressed data output using the
    //..function "deflateBound()", resize the output vector "compressed"
    //..to that size plus a few bytes for the header, and give a pointer to
    //..the first byte after the header data.
    memory_index maxExtractedSize = deflateBound(&strm, source.size());
    compressed.resize(maxExtractedSize + sizeof(header));
    strm.next_out = (u8 *)(compressed.data() + sizeof(header));
    strm.avail_out = maxExtractedSize;

    //Compress the data
    retval = deflate(&strm, Z_FINISH);

    //Write the size of the compressed output to the header
    header.compressedSize = strm.total_out;

    //shut down deflate
    deflateEnd(&strm);

    //if the compression was unsuccessfull, return an empty vector.
    if(retval != Z_STREAM_END)
    {
        compressed.clear();
        return compressed;
    }

    //Resize the output vector to match the compressed data size
    compressed.resize(compressed.size() - strm.avail_out);

    //Add the header to the beginning of the vector and return the data
    //..vector by value
    for(memory_index n = 0; n < sizeof(header); n++)
        compressed[n] = *((u8 *)&header + n);
    return compressed;
}

///////////////////////////////////////////////////////////////////////////////
// Extract a vector of data previously compressed using the compress() utility,
// using the zlib general purpose compression library (v 1.2.8).
//=============================================================================
// The source vector must be larger or equal in size to the amount of
// compressed data originally compressed.
// In case of failure, this function returns an empty vector.
///////////////////////////////////////////////////////////////////////////////
std::vector<u8> Compression::extract(const std::vector<u8> &source)
{
    std::vector<u8> extracted;

    //extract the header from the compressed data
    DataHeader header;
    for(memory_index n = 0; n < sizeof(header); n++)
        *((u8 *)&header + n) = source[n];

    //Check if the source vector is smaller than the amount of compressed
    //..bytes it is supposed to contain and quit if so
    if(source.size() < header.compressedSize + sizeof(header))
    {
        return extracted;
    }

    //Initialize the zlib stream struct
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    //set the stream input size as the size of the compressed data vector
    //.."source", and give a pointer to the first byte of the vector.
    strm.next_in = (u8 *)(source.data() + sizeof(header));
    strm.avail_in = header.compressedSize;

    //Set the stream output size to equal the size of the extracted data
    //..(the value was written to the header during compression).
    extracted.resize(header.extractedSize);
    strm.next_out = (u8 *)extracted.data();
    strm.avail_out = header.extractedSize;

    //Initialize the stream state for inflation
    s32 retval = inflateInit (&strm);
    if(retval != Z_OK)
    {
        inflateEnd(&strm);
        extracted.clear();
        return extracted;
    }

    //Compress the data
    retval = inflate(&strm, Z_FINISH);

    //shut down the inflate stream struct
    inflateEnd(&strm);

    //if the compression was unsuccessfull, return an empty vector.
    if(retval != Z_STREAM_END)
    {
        extracted.clear();
        return extracted;
    }

    //return the extracted data vector by value
    return extracted;
}

///////////////////////////////////////////////////////////////////////////////
// Return the uncompressed size of a vector containing data previously
// compressed using the compress() utility.
//=============================================================================
// In case of failure, this function returns 0;
///////////////////////////////////////////////////////////////////////////////
memory_index Compression::getExtractedSize(const std::vector<u8> &source)
{
    DataHeader header;
    for(memory_index n = 0; n < sizeof(header); n++)
        *((u8 *)&header + n) = source[n];
    return header.extractedSize;
}
