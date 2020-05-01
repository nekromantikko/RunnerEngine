#include "../platform_rendering.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GL\GLU.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "../platform_main.h"

#define MODELVIEW_LOCATION 0
#define PROJECTION_LOCATION 1

#define V_POS_LOCATION 0
#define V_TEXCOORD_LOCATION 1
#define V_NORMAL_LOCATION 2
#define V_TANGENT_LOCATION 3

struct InternalTexture
{
    u32 id;
};

struct InternalShader
{
    u32 id; //program id

    u32 propertyCount;
    s32 *propertyLocation;
};

struct InternalMesh
{
    GLuint vao;
    GLuint posBuffer;
    GLuint texCoordBuffer;
    GLuint normalBuffer;
    GLuint tangentBuffer;
    GLuint indexBuffer;

    u32 vertexCount;
    u32 triangleCount;
};

GLuint lightDataUBO;
GLuint lightDataBindingIndex = 0;
GLint lightDataSize = 0;

GLuint currentShaderProgram;

//orthogonal projection matrix
glm::mat4 orthoMatrix;

////////////////////////////////////////////////////////////////////////////////////////

//INIT
void platform_init_rendering()
{
    //initialize DevIL
    ilInit();

    //////////////////////////////////////////////////////////////////////////

    //Uniform buffers
    glGenBuffers(1, &lightDataUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, lightDataUBO);
    glBufferData(GL_UNIFORM_BUFFER, 528, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, lightDataBindingIndex, lightDataUBO);

    ///////////////////////////////////////////////////////////////////////////

    //Set blending
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    glEnable(GL_FRAMEBUFFER_SRGB);

    //back face culling
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    //projectionmatrix
    orthoMatrix = glm::mat4(glm::ortho(0.f, (r32)SCREEN_WIDTH, (r32)SCREEN_HEIGHT, 0.f, -16384.f, 16384.f));
}

void platform_deinit_rendering()
{
    glDeleteBuffers(1, &lightDataUBO);

    ilShutDown();
}

////////////////////////////////////////////////////////////////////////////

void platform_load_image(rImage *image, const char* fname)
{
    //generate and set current image ID
    ILuint imgID;
    ilGenImages(1, &imgID);
    ilBindImage(imgID);

    //load
    ilLoadImage(fname);
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    image->width = ilGetInteger(IL_IMAGE_WIDTH);
    image->height = ilGetInteger(IL_IMAGE_HEIGHT);

    s32 imageSize = image->width * image->height * 4;
    image->pixels = new u8[imageSize];

    memcpy(image->pixels, ilGetData(), imageSize);

    //delete file from memory
    ilDeleteImages(1, &imgID);
}
void platform_delete_image(rImage image)
{
    delete[] image.pixels;
}

void platform_load_palette(const char *fname, void *output, int s = 16)
{
    FILE *file = fopen(fname, "r");
    if (file != NULL)
    {
        char id1[9];
        char id2[5];
        char colorCountStr[8];

        fscanf(file, "%s9", id1);
        fscanf(file, "%s5", id2);
        fscanf(file, "%s8", colorCountStr);

        int colorCount = atoi(colorCountStr);
        colorCount = std::min(s, colorCount);

        char r[4];
        char g[4];
        char b[4];

        for (int i = 0; i < colorCount; i++)
        {
            fscanf(file, "%s4", r);
            fscanf(file, "%s4", g);
            fscanf(file, "%s4", b);

            //gamma correct
            u8 r_gamma, g_gamma, b_gamma;
            r_gamma = std::pow((atoi(r) / 255.f), 2.2) * 255.f;
            g_gamma = std::pow((atoi(g) / 255.f), 2.2) * 255.f;
            b_gamma = std::pow((atoi(b) / 255.f), 2.2) * 255.f;

            u8 r5, g6, b5;
            r5 = (r_gamma >> 3);
            g6 = (g_gamma >> 2);
            b5 = (b_gamma >> 3);

            u8 *data = (u8*)output + i*2;

            u8 hi, lo;
            hi = (g6 >> 3) + (r5 << 3);
            lo = (g6 << 5) + b5;

            data[0] = lo;
            data[1] = hi;


        }

    }
}

void platform_load_mesh(MeshData *mesh, const char* fname)
{
    std::ifstream file(fname);
    if (file.is_open())
    {
        std::vector<v3> vertPosList;
        std::vector<v2> texCoordList;
        std::vector<v3> normalList;

        //read file to the end
        while (!file.eof())
        {
            std::string line;
            std::getline(file, line);
            std::string firstWord;
            std::stringstream sstream(line);
            sstream >> firstWord;

            //load vertex coordinate
            if (firstWord.compare("v") == 0)
            {
                r32 x, y, z;
                sstream >> x >> y >> z;
                vertPosList.emplace_back(std::round(x), std::round(-y), std::round(z));
            }
            //load texture coordinate
            else if (firstWord.compare("vt") == 0)
            {
                r32 u, v;
                sstream >> u >> v;
                texCoordList.emplace_back(u, 1 - v);
            }
            //load normals
            else if (firstWord.compare("vn") == 0)
            {
                r32 nx, ny, nz;
                sstream >> nx >> ny >> nz;
                normalList.emplace_back(nx,ny,nz);
            }
            //load face
            else if (firstWord.compare("f") == 0)
            {
                std::string vert;
                u32 vertAmount = 0;
                u32 nextIndex = mesh->positions.size();
                std::vector<u32> tempIndices;
                while (sstream >> vert)
                {
                    std::stringstream sstream2(vert);
                    u32 posIndex, texIndex, normIndex;
                    sstream2 >> posIndex;
                    sstream2.ignore(1);
                    sstream2 >> texIndex;
                    sstream2.ignore(1);
                    sstream2 >> normIndex;
                    posIndex--;
                    texIndex--;
                    normIndex--;

                    v3 tempPos = {vertPosList.at(posIndex).x, vertPosList.at(posIndex).y, vertPosList.at(posIndex).z};
                    v2 tempCoord = {texCoordList.at(texIndex).x, texCoordList.at(texIndex).y};
                    v3 tempNormal = {normalList.at(normIndex).x, normalList.at(normIndex).y, normalList.at(normIndex).z};

                    bool exists = false;
                    for (u32 i = 0; i < mesh->positions.size(); i++)
                    {
                        if (mesh->positions.at(i) == tempPos && mesh->texCoords.at(i) == tempCoord && mesh->normals.at(i) == tempNormal)
                        {
                            exists = true;
                            tempIndices.push_back(i);
                        }
                    }

                    if (!exists)
                    {
                        mesh->positions.push_back(tempPos);
                        mesh->texCoords.push_back(tempCoord);
                        mesh->normals.push_back(tempNormal);

                        tempIndices.push_back(nextIndex++);
                    }
                    ++vertAmount;
                }
                //make triangle fan from polygon
                for (u32 i = 0; i < vertAmount - 2; i++)
                    mesh->tris.emplace_back(tempIndices.at(0), tempIndices.at(i + 1), tempIndices.at(i + 2));
            }
        }
        file.close();
    }
    else
    {
        std::stringstream errormsg;
        errormsg << "Cannot load obj-file " << fname;
        platform_show_error(errormsg.str().c_str());
    }

    platform_calculate_tangent(mesh);
}

void platform_calculate_tangent(MeshData *mesh)
{
    u32 vertexCount = mesh->positions.size();

    v3 *tan1 = new v3[vertexCount * 2];
    v3 *tan2 = tan1 + vertexCount;
    memset(tan1, 0, vertexCount * 2 * sizeof(v3));

    //for each triangle in mesh
    for (u32 i = 0; i < mesh->tris.size(); i++)
    {
        Triangle *triangle = &mesh->tris.at(i);
        u32 i1 = triangle->index[0];
        u32 i2 = triangle->index[1];
        u32 i3 = triangle->index[2];

        const v3 vert1 = mesh->positions.at(i1);
        const v3 vert2 = mesh->positions.at(i2);
        const v3 vert3 = mesh->positions.at(i3);

        const v2 uv1 = mesh->texCoords.at(i1);
        const v2 uv2 = mesh->texCoords.at(i2);
        const v2 uv3 = mesh->texCoords.at(i3);

        r32 x1 = vert2.x - vert1.x;
        r32 x2 = vert3.x - vert1.x;
        r32 y1 = vert2.y - vert1.y;
        r32 y2 = vert3.y - vert1.y;
        r32 z1 = vert2.z - vert1.z;
        r32 z2 = vert3.z - vert1.z;

        r32 s1 = uv2.x - uv1.x;
        r32 s2 = uv3.x - uv1.x;
        r32 t1 = uv2.y - uv1.y;
        r32 t2 = uv3.y - uv1.y;

        r32 r = 1.f / (s1 * t2 - s2 * t1);
        v3 sdir({(t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r});
        v3 tdir({(s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r});

        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;

        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;
    }

    for (u32 i = 0; i < vertexCount; i++)
    {
        const v3 &n = mesh->normals.at(i);
        const v3 &t = tan1[i];

        v4 tangent;
        //Gram-Schmidt orthogonalize
        tangent.xyz() = Normalize(t - n * Inner(n, t));

        //Calculate handedness
        tangent.w = (Inner(Cross(n, t), tan2[i]) < 0.0F) ? -1.0F : 1.0F;

        mesh->tangents.push_back(tangent);
    }

    delete[] tan1;
}

////////////////////////////////////////////////////////////////////////////////////////

ShaderPropertyType gl_convert_shader_property_type(GLenum propertyType)
{
    ShaderPropertyType result;

    switch(propertyType)
    {
    case GL_FLOAT:
        result = SHADER_PROPERTY_FLOAT;
        break;
    case GL_FLOAT_VEC2:
        result = SHADER_PROPERTY_VEC2;
        break;
    case GL_FLOAT_VEC3:
        result = SHADER_PROPERTY_VEC3;
        break;
    case GL_FLOAT_VEC4:
        result = SHADER_PROPERTY_VEC4;
        break;
    case GL_INT:
        result = SHADER_PROPERTY_INT;
        break;
    case GL_UNSIGNED_INT:
        result = SHADER_PROPERTY_UINT;
        break;
    case GL_SAMPLER_2D:
        result = SHADER_PROPERTY_TEXTURE2D;
        break;
    }

    return result;
}

void platform_load_string_from_file(const char *fname, char *contents)
{
    std::string temp;
    std::ifstream sourceFile(fname);

    temp.assign((std::istreambuf_iterator<char>(sourceFile)), std::istreambuf_iterator<char>());
    memcpy(contents, temp.c_str(), temp.length());
    contents[temp.length()] = NULL;
}

InternalShader *platform_load_shader(const char* vert, const char* frag)
{
    InternalShader *result = new InternalShader;

    GLuint shaderID = glCreateProgram();

    char vertexSource[4096];
    char fragmentSource[4096];
    platform_load_string_from_file(vert, vertexSource);
    platform_load_string_from_file(frag, fragmentSource);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, NULL);
    glCompileShader(vertexShader);
    glAttachShader(shaderID, vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, NULL);
    glCompileShader(fragmentShader);
    glAttachShader(shaderID, fragmentShader);

    glLinkProgram(shaderID);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    result->id = shaderID;

    return result;

    /*if (shader == SHADER_SPRITE_LIT ||
        shader == SHADER_SPRITE_LIT_NORMAL ||
        shader == SHADER_TILES_LIT ||
        shader == SHADER_TILES_LIT_NORMAL)
    {
        GLuint lightDataIndex = glGetUniformBlockIndex(shaderID, "LightData");
        glUniformBlockBinding(shaderID, lightDataIndex, lightDataBindingIndex);
    }*/
}

void platform_get_shader_property_count(InternalShader *shader, u32 *count)
{
    glGetProgramiv(shader->id, GL_ACTIVE_UNIFORMS, (GLint*)count);
}

void platform_get_shader_properties(InternalShader *shader, u32 count, char **names, ShaderPropertyType *types)
{
    for (int i = 0; i < count; i++)
    {
        GLenum type;
        glGetActiveUniform(shader->id, (GLuint)i, SHADER_PROPERTY_MAX_NAME_LENGTH, NULL, NULL, &type, names[i]);
        types[i] = gl_convert_shader_property_type(type);
    }
}

void platform_delete_shader(InternalShader *shader)
{
    glDeleteProgram(shader->id);
}

////////////////////////////////////////////////////////////////////////////////////////

void platform_use_shader(InternalShader *shader)
{
    glUseProgram(shader->id);
}

////////////////////////////////////////////////////////////////////////////////////////

InternalTexture *platform_create_indexed_sprite_sheet(rImage *image)
{
    glBindTexture(GL_TEXTURE_2D, NULL);

    InternalTexture *texture = new InternalTexture;
    //create gl texture
    glGenTextures(1, (GLuint*)texture);
    //texture->w = ilGetInteger(IL_IMAGE_WIDTH);
    //texture->h = ilGetInteger(IL_IMAGE_HEIGHT);

    //bind texture ID
    glBindTexture(GL_TEXTURE_2D, texture->id);

    GLint format = GL_R8;

    glTexImage2D(GL_TEXTURE_2D, 0, format, image->width, image->height, 0, GL_RED, GL_UNSIGNED_BYTE, image->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

void platform_load_texture(InternalTexture *texture, rImage *image, bool srgb)
{
    glBindTexture(GL_TEXTURE_2D, NULL);

    texture = new InternalTexture;
    //create gl texture
    glGenTextures(1, (GLuint*)texture);

    //bind texture ID
    glBindTexture(GL_TEXTURE_2D, texture->id);

    GLint format;
    if (srgb)
        format = GL_SRGB_ALPHA;
    else format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}
void platform_delete_texture(InternalTexture *texture)
{
    glDeleteTextures(1, (GLuint*)texture);
    delete texture;
}
void platform_get_texture_width(InternalTexture*texture, u32 *w)
{
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, (GLint*)w);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void platform_get_texture_height(InternalTexture *texture, u32 *h)
{

    glBindTexture(GL_TEXTURE_2D, texture->id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint*)h);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void platform_create_empty_texture(InternalTexture *texture, s32 w, s32 h)
{
    glBindTexture(GL_TEXTURE_2D, NULL);

    texture = new InternalTexture;
    //create gl texture
    glGenTextures(1, (GLuint*)texture);

    //bind texture ID
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

////////////////////////////////////////////////////////////////////////////////////////

void platform_create_tile_index_map(InternalTexture *texture, u32 w, u32 h)
{
    glBindTexture(GL_TEXTURE_2D, NULL);

    texture = new InternalTexture;
    glGenTextures(1, (GLuint*)texture);

    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void platform_populate_tile_index_map(InternalTexture *texture, u32 w, u32 h, u8 *pixels)
{
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

////////////////////////////////////////////////////////////////////////////////////////

void platform_create_palette_texture(InternalTexture *texture, Palette *palette)
{
    texture = new InternalTexture;
    glGenTextures(1, (GLuint*)texture);

    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB565, 256, 1, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, palette);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void platform_update_palette_texture(InternalTexture *texture, Palette *palette)
{
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, palette);
    glBindTexture(GL_TEXTURE_2D, 0);
}

////////////////////////////////////////////////////////////////////////////////////////

void platform_shader_set_texture(u32 index, InternalTexture *tex)
{

}
void platform_shader_set_float(u32 index,  u32 count, float *f)
{

}
void platform_shader_set_vector(u32 index, u32 count, v2 *vec)
{

}
void platform_shader_set_vector(u32 index, u32 count, v3 *vec)
{

}
void platform_shader_set_vector(u32 index, u32 count, v4 *vec)
{

}
void platform_shader_set_int(u32 index,  u32 count, s32 *i)
{

}
void platform_shader_set_uint(u32 index,  u32 count, u32 *u)
{

}

////////////////////////////////////////////////////////////////////////////////////////

void platform_set_lights(Light *lights, u32 lightCount)
{
    glBindBuffer(GL_UNIFORM_BUFFER, lightDataUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 12, sizeof(glm::uint), &lightCount);
    glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(glm::vec4) * lightCount * 2, (GLfloat*)lights);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void platform_set_ambient_color(v3 *color)
{
    glBindBuffer(GL_UNIFORM_BUFFER, lightDataUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec3), (GLfloat*)color);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

////////////////////////////////////////////////////////////////////////////////////////

void platform_render(Transform xform)
{
    glm::mat4 modelview = glm::mat4(1.0f);
    modelview = glm::translate(modelview, glm::vec3(xform.position.x,xform.position.y,0));
    modelview = glm::rotate(modelview, (float)xform.rotation.x, glm::vec3(1, 0, 0));
    modelview= glm::rotate(modelview, (float)xform.rotation.y, glm::vec3(0, 1, 0));
    modelview = glm::rotate(modelview, (float)xform.rotation.z, glm::vec3(0, 0, 1));
    modelview = glm::scale(modelview, glm::vec3(xform.scale.x,xform.scale.y,xform.scale.z));
    glUniformMatrix4fv(MODELVIEW_LOCATION, 1, GL_FALSE, glm::value_ptr(modelview));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////

void platform_clear_buffer()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

////////////////////////////////////////////////////////////////////////////////////////

void platform_create_mesh(InternalMesh *handle, MeshData *mesh)
{
    handle = new InternalMesh;

    glGenVertexArrays(1, &handle->vao);
    glBindVertexArray(handle->vao);

    glGenBuffers(1, &handle->posBuffer);
    glGenBuffers(1, &handle->texCoordBuffer);
    glGenBuffers(1, &handle->normalBuffer);
    glGenBuffers(1, &handle->tangentBuffer);
    glGenBuffers(1, &handle->indexBuffer);

    u32 vertexCount = mesh->positions.size();
    handle->vertexCount = vertexCount;
    handle->triangleCount = mesh->tris.size();

    glBindBuffer(GL_ARRAY_BUFFER, handle->posBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(v3), mesh->positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, handle->texCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(v2), mesh->texCoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, handle->normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(v3), mesh->normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, handle->tangentBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(v4), mesh->tangents.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle->indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->tris.size() * sizeof(Triangle), mesh->tris.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void platform_delete_mesh(InternalMesh *buf)
{
    glDeleteVertexArrays(1, &buf->vao);
    glDeleteBuffers(1, &buf->posBuffer);
    glDeleteBuffers(1, &buf->texCoordBuffer);
    glDeleteBuffers(1, &buf->normalBuffer);
    glDeleteBuffers(1, &buf->tangentBuffer);
    glDeleteBuffers(1, &buf->indexBuffer);

    delete buf;
}

////////////////////////////////////////////////////////////////////////////////////////

void platform_use_mesh(InternalMesh *mesh)
{
    if (mesh)
        glBindVertexArray(mesh->vao);
    else glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////////////////////////

void platform_set_projection()
{
    glUniformMatrix4fv(PROJECTION_LOCATION, 1, GL_FALSE, glm::value_ptr(orthoMatrix));
}

////////////////////////////////////////////////////////////////////////////////////////

void platform_enable_depth_test()
{
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthRange(1,0);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void platform_disable_depth_test()
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
}
