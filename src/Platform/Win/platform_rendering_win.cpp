#include "../platform_rendering.h"

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
void init_renderer()
{
    create_screen_vbo();

    //post processing
    glGenFramebuffers(1, &postProcessFBO);

    glGenTextures(1, &postProcessTexture);
    glBindTexture(GL_TEXTURE_2D, postProcessTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT + SCREEN_MARGINAL, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, postProcessFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, postProcessTexture, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    //reflect fbo
    glGenFramebuffers(1, &reflectFBO);

    glGenTextures(1, &reflectTexture);
    glBindTexture(GL_TEXTURE_2D, reflectTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT + SCREEN_MARGINAL, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, reflectFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, reflectTexture, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    //blur stuff
    glGenFramebuffers(1, &glowFBO);
    glGenFramebuffers(4, blurFBO);

    glGenTextures(1, &glowTexture);
    glBindTexture(GL_TEXTURE_2D, glowTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT  + SCREEN_MARGINAL, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, glowFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, glowTexture, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    for (s32 i = 0; i <= 3; i++)
    {
        glGenTextures(1, blurTexture + i);
        glBindTexture(GL_TEXTURE_2D, blurTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH / pow(2, i), (SCREEN_HEIGHT + SCREEN_MARGINAL) / pow(2, i), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[i]);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, blurTexture[i], 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
    }

    //screenshot buffer and texture
    glGenFramebuffers(1, &screenshotFBO);

    glGenTextures(1, &screenshotTexture);
    glBindTexture(GL_TEXTURE_2D, screenshotTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, screenshotFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, screenshotTexture, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    screenshot.id = screenshotTexture;
    //screenshot.w = SCREEN_WIDTH;
    //screenshot.h = SCREEN_HEIGHT;

    //////////////////////////////////////////////////////////////////////////

    //Uniform buffers
    glGenBuffers(1, &lightDataUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, lightDataUBO);
    glBufferData(GL_UNIFORM_BUFFER, 528, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, lightDataBindingIndex, lightDataUBO);

    ///////////////////////////////////////////////////////////////////////////

    //LOAD SHADERS
    gl_load_background_shader();
    gl_load_circle_transition_shader();
    //gl_load_line_shader();

    //gl_load_world_shader();

    //for (int i = 0; i < MAX_LIGHTS; i++)
        //gl_load_world_light_shader(i);

    gl_load_glow_threshold_shader();
    gl_load_gaussian_blur_shader();
    gl_load_final_image_shader();
    //gl_load_blend_shader();

    //////////////////////////////////////////////////////

    //Set blending
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    glEnable(GL_FRAMEBUFFER_SRGB);

    //enable wireframe
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    //back face culling
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    //projectionmatrix
    orthoMatrix = glm::mat4(glm::ortho(0.f, (r32)SCREEN_WIDTH, (r32)SCREEN_HEIGHT, 0.f, -16384.f, 16384.f));
}

void close_renderer()
{
    //delete background shader
    glDeleteProgram(backgroundShader.programID);
    glDeleteProgram(circleTransitionShader.programID);

    //glDeleteProgram(lineShaderProgram);

    //glDeleteProgram(worldShaderProgram);

    glDeleteProgram(glowThresholdShader.programID);
    glDeleteProgram(gaussianBlurShader.programID);
    glDeleteProgram(finalImageShader.programID);
    //glDeleteProgram(blendShaderProgram);


    //free textures
    //glDeleteTextures(1, &deferredDiffuseTexture);
    //glDeleteTextures(1, &deferredLightmapTexture);
    //glDeleteTextures(1, &deferredPositionTexture);
    //glDeleteTextures(1, &deferredNormalTexture);
    glDeleteTextures(1, &reflectTexture);
    glDeleteTextures(1, &postProcessTexture);
    glDeleteTextures(1, &glowTexture);
    glDeleteTextures(4, blurTexture);
    glDeleteTextures(1, &screenshotTexture);

    glDeleteBuffers(1, &lightDataUBO);

    delete_screen_vbo();
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

InternalShader *platform_load_shader(const char* vert, const char* frag)
{
    InternalShader *result = new InternalShader;

    GLuint shaderID = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const GLchar**)vert, NULL);
    glCompileShader(vertexShader);
    glAttachShader(shaderID, vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const GLchar**)frag, NULL);
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

InternalTexture *platform_load_texture(rImage *image, bool srgb)
{
    glBindTexture(GL_TEXTURE_2D, NULL);

    Texture *texture = new Texture;
    //create gl texture
    glGenTextures(1, (GLuint*)texture);
    texture->w = ilGetInteger(IL_IMAGE_WIDTH);
    texture->h = ilGetInteger(IL_IMAGE_HEIGHT);

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

    return texture;
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

InternalTexture *platform_create_empty_texture(s32 w, s32 h)
{
    glBindTexture(GL_TEXTURE_2D, NULL);

    InternalTexture *texture = new InternalTexture;
    //create gl texture
    glGenTextures(1, (GLuint*)texture);
    texture->w = w;
    texture->h = h;

    //bind texture ID
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

////////////////////////////////////////////////////////////////////////////////////////

InternalTexture *platform_create_tile_index_map(u32 w, u32 h)
{
    glBindTexture(GL_TEXTURE_2D, NULL);

    InternalTexture *texture = new InternalTexture;
    glGenTextures(1, (GLuint*)texture);
    texture->w = w;
    texture->h = h;

    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

void platform_populate_tile_index_map(InternalTexture *texture, u32 w, u32 h, u8 *pixels)
{
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

////////////////////////////////////////////////////////////////////////////////////////

InternalTexture *platform_create_palette_texture(Palette *palette)
{
    InternalTexture *texture = new InternalTexture;
    glGenTextures(1, (GLuint*)texture);

    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB565, 256, 1, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, palette);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

void platform_update_palette_texture(InternalTexture *texture, Palette *palette)
{
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, palette);
    glBindTexture(GL_TEXTURE_2D, 0);
}

////////////////////////////////////////////////////////////////////////////////////////

void platform_shader_set_texture(const char* propertyName, InternalTexture *tex, TextureType type = TEXTURE_BASE_COLOR)
{
    GLint textureLocation = glGetUniformLocation(currentShaderProgram, propertyName);

    glActiveTexture(GL_TEXTURE0 + type);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glUniform1i(textureLocation, type);
}
void platform_shader_set_float(const char* propertyName, float f)
{
    GLint floatLocation = glGetUniformLocation(currentShaderProgram, propertyName);

    glUniform1f(floatLocation, (GLfloat)f);
}
void platform_shader_set_vector(const char* propertyName, v2 vec)
{
    GLint vecLocation = glGetUniformLocation(currentShaderProgram, propertyName);

    glUniform2f(vecLocation, (GLfloat)vec.x, (GLfloat)vec.y);
}
void platform_shader_set_vector(const char* propertyName, v3 vec)
{
    GLint vecLocation = glGetUniformLocation(currentShaderProgram, propertyName);

    glUniform3f(vecLocation, (GLfloat)vec.x, (GLfloat)vec.y, (GLfloat)vec.z);
}
void platform_shader_set_vector(const char* propertyName, v4 vec)
{
    GLint vecLocation = glGetUniformLocation(currentShaderProgram, propertyName);

    glUniform4f(vecLocation, (GLfloat)vec.x, (GLfloat)vec.y, (GLfloat)vec.z, (GLfloat)vec.w);
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

void platform_blit()
{
    gl_render_generic_framebuffer();
}

////////////////////////////////////////////////////////////////////////////////////////

void platform_clear_buffer()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void platform_swap_buffer()
{
    SDL_GL_SwapWindow(runnerWindow);
}

////////////////////////////////////////////////////////////////////////////////////////

InternalMesh *platform_create_mesh(MeshData *mesh)
{
    InternalMesh *handle = new InternalMesh;

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

    return handle;
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
