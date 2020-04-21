#ifndef BSP_H
#define BSP_H

#include "file_formats.h"

struct TileBSPNode
{
    TileBSPNode *parent;
    TileBSPNode *children;
    Rectangle2 bounds;
    u32 *chunk;
    int level;

    TileBSPNode() : parent(nullptr), children(nullptr), bounds({0,0,0,0}), chunk(nullptr), level(0)
    {
    }

    TileBSPNode(u32 width, u32 height) : parent(nullptr), children(nullptr), level(0)
    {
        bounds = {0,width,0,height};
        int chunkSize = width*height;
        chunk = new u32[chunkSize];
    }

    ~TileBSPNode()
    {
        if (children)
        {
            delete[] children;
        }
        if (chunk)
        {
            delete[] chunk;
        }
    }

    inline TileBSPNode *get_sibling() const
    {
        TileBSPNode *result = nullptr;
        if (parent)
        {
            if (&parent->children[0] == this)
                result = &parent->children[1];
            else result = &parent->children[0];
        }
        return result;
    }

    inline r32 get_width()
    {
        r32 result;
        result = bounds.x2 - bounds.x1;
        return result;
    }

    inline r32 get_height()
    {
        r32 result;
        result = bounds.y2 - bounds.y1;
        return result;
    }

    u32 get_size()
    {
        u32 nodeCount = 1;
        if (children)
        {
            nodeCount += children[0].get_size();
            nodeCount += children[1].get_size();
        }
        return nodeCount;
    }

    u32 get_depth()
    {
        u32 depth;
        if (children)
        {
            depth = children[0].get_depth();
            depth = std::max(depth, children[1].get_depth());
        }
        else depth = level;
        return depth;
    }

    u32 get_max_size()
    {
        int depth = get_depth();
        u32 size = (1<<depth) - 1;

        return size;
    }

    void to_vector(std::vector<TileBSPNode*> &vec)
    {
        vec.push_back(this);

        if(children)
        {
            children[0].to_vector(vec);
            children[1].to_vector(vec);
        }
    }

    int populate_arrays(int *indices, std::vector<TileBSPNodeInfo> &nodeInfos, std::vector<std::vector<u32>> &chunkData, int pos, int skippedChunks)
    {
        r32 width = get_width();
        r32 height = get_height();

        int chunkSize;
        if (!chunk || is_empty())
        {
            chunkSize = 0;
            skippedChunks++;
        }
        else chunkSize = width*height;

        indices[pos] = nodeInfos.size();

        TileBSPNodeInfo info;
        info.bounds = bounds;
        info.tileCount = chunkSize;
        info.offset = skippedChunks;

        nodeInfos.push_back(info);
        if (chunkSize != 0)
            chunkData.push_back(std::vector<u32>(chunk, chunk+chunkSize));

        if (children)
        {
            skippedChunks = children[0].populate_arrays(indices, nodeInfos, chunkData, 2*pos + 1, skippedChunks);
            skippedChunks = children[1].populate_arrays(indices, nodeInfos, chunkData, 2*pos + 2, skippedChunks);
        }

        return skippedChunks;

    }

    void populate_from_arrays(int *indices, std::vector<TileBSPNodeInfo> &nodeInfos, std::vector<std::vector<u32>> &chunkData, int pos, int n)
    {
        int newPos = 2*pos+1;

        if (newPos < n && indices[newPos] != -1)
        {
            int firstChildIndex = indices[newPos];
            int secondChildIndex = indices[newPos+1];

            children = new TileBSPNode[2];

            children[0].parent = this;
            children[1].parent = this;

            children[0].level = level + 1;
            children[1].level = level + 1;

            //std::cout << firstChildIndex << ", " << secondChildIndex << std::endl;

            TileBSPNodeInfo nodeInfo1 = nodeInfos.at(firstChildIndex);
            children[0].bounds = nodeInfo1.bounds;

            if (nodeInfo1.tileCount > 0)
            {
                std::vector<u32> &tileData1 = chunkData.at(firstChildIndex - nodeInfo1.offset);
                children[0].chunk = new u32[nodeInfo1.tileCount];
                memcpy(children[0].chunk, tileData1.data(), nodeInfo1.tileCount * sizeof(u32));
            }

            children[0].populate_from_arrays(indices, nodeInfos, chunkData, newPos, n);

            TileBSPNodeInfo nodeInfo2 = nodeInfos.at(secondChildIndex);
            children[1].bounds = nodeInfo2.bounds;

            if (nodeInfo2.tileCount > 0)
            {
                std::vector<u32> &tileData2 = chunkData.at(secondChildIndex - nodeInfo2.offset);
                children[1].chunk = new u32[nodeInfo2.tileCount];
                memcpy(children[1].chunk, tileData2.data(), nodeInfo2.tileCount * sizeof(u32));
            }

            children[1].populate_from_arrays(indices, nodeInfos, chunkData, newPos+1, n);
        }
    }

    bool is_empty()
    {
        u32 width = get_width();
        u32 height = get_height();

        bool isEmpty = true;

        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                if (get_tile(x,y) != 0)
                    isEmpty = false;
            }
        }
        return isEmpty;
    }

    inline u32 get_tile(u32 x, u32 y)
    {
        u32 result = 0xFFFFFFFF;

        r32 width = get_width();
        r32 height = get_height();

        if (x < width && y < height)
        {
            u32 index = y * width + x;
            result = chunk[index];
        }

        return result;
    }

    bool split_vertically(u32 x)
    {
        bool result = false;

        r32 width = get_width();
        r32 height = get_height();

        if (x < width && height > 0)
        {
            children = new TileBSPNode[2];

            Rectangle2 leftBounds = bounds;
            Rectangle2 rightBounds = bounds;

            leftBounds.x2 -= (width - x);
            rightBounds.x1 += x;

            children[0].bounds = leftBounds;
            children[1].bounds = rightBounds;

            children[0].parent = this;
            children[1].parent = this;

            children[0].level = level + 1;
            children[1].level = level + 1;

            if (chunk)
            {
                u32 *leftChunk;
                u32 *rightChunk;

                u32 leftChunkSize = x*height;
                u32 rightChunkSize = (width-x)*height;
                leftChunk = new u32[leftChunkSize];
                rightChunk = new u32[rightChunkSize];

                //copy tiles from original chunk to left chunk
                for (int i = 0; i < leftChunkSize; i++)
                {
                    u32 index = i + ((width-x) * (i/x));
                    leftChunk[i] = chunk[index];
                }
                //copy tiles from original chunk to right chunk
                for (int i = 0; i < rightChunkSize; i++)
                {
                    u32 diff = width - x;
                    u32 index = ((i / diff) * width) + x + (i % diff);
                    rightChunk[i] = chunk[index];
                }

                children[0].chunk = leftChunk;
                children[1].chunk = rightChunk;

                delete[] chunk;
                chunk = nullptr;

            }
            result = true;

        }

        return result;
    }

    bool split_horizontally(u32 y)
    {
        bool result = false;

        r32 width = get_width();
        r32 height = get_height();

        if (y < height && width > 0)
        {
            children = new TileBSPNode[2];

            Rectangle2 topBounds = bounds;
            Rectangle2 bottomBounds = bounds;

            topBounds.y2 -= (height - y);
            bottomBounds.y1 += y;

            children[0].bounds = topBounds;
            children[1].bounds = bottomBounds;

            children[0].parent = this;
            children[1].parent = this;

            children[0].level = level + 1;
            children[1].level = level + 1;

            if (chunk)
            {
                u32 *topChunk;
                u32 *bottomChunk;

                u32 topChunkSize = y*width;
                u32 bottomChunkSize = (height-y)*width;
                topChunk = new u32[topChunkSize];
                bottomChunk = new u32[bottomChunkSize];

                //copy tiles from original chunk to left chunk
                for (int i = 0; i < topChunkSize; i++)
                {
                    u32 index = i;
                    topChunk[i] = chunk[index];
                }
                //copy tiles from original chunk to right chunk
                for (int i = 0; i < bottomChunkSize; i++)
                {
                    u32 index = i + (y*width);
                    bottomChunk[i] = chunk[index];
                }

                children[0].chunk = topChunk;
                children[1].chunk = bottomChunk;

                delete[] chunk;
                chunk = nullptr;
            }
            result = true;

        }

        return result;
    }
};

#endif // BSP_H
