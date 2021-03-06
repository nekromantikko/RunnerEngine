#include <windows.h>
#include <Objbase.h>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <GL\GLU.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <fmod.hpp>
#include <fmod_errors.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include "platform.h"
#include <sstream>
#include "../debug.h"

#define MODELVIEW_LOCATION 0
#define PROJECTION_LOCATION 1

#define TEXTURE_LOCATION 2
#define LIGHTMAP_LOCATION 3
#define NORMALMAP_LOCATION 4

#define SPRITE_CLIPRECT_LOCATION 5
#define SPRITE_OFFSET_LOCATION 6
#define SPRITE_FLIP_LOCATION 7

#define SPRITE_COLOR_LOCATION 8
#define GLOW_LOCATION 9
#define AMBIENT_COLOR_LOCATION 10

#define TIME_ALPHA_LOCATION 11

#define CAMERA_POS_LOCATION 12
#define CAMERA_PREV_POS_LOCATION 13

#define TILES_POSITION_LOCATION 14
#define TILES_LAYOUT_LOCATION 15

#define LIGHT_AMOUNT_LOCATION 16
#define LIGHT_LOCATION 17

#define V_POS_LOCATION 0
#define V_TEXCOORD_LOCATION 1
#define V_NORMAL_LOCATION 2
#define V_TANGENT_LOCATION 3

#define PARTICLE_V_CURRENT_POS_LOCATION 0;
#define PARTICLE_V_PREVIOUS_POS_LOCATION 1;
#define PARTICLE_V_CURRENT_ROT_LOCATION 2;
#define PARTICLE_V_PREVIOUS_ROT_LOCATION 3;
#define PARTICLE_V_CURRENT_SCALE_LOCATION 4;
#define PARTICLE_V_PREVIOUS_SCALE_LOCATION 5;
#define PARTICLE_V_COLOR_LOCATION 6;
#define PARTICLE_V_STATE_LOCATION 7;

#define OGG_MUSIC
#define OGG_USE_TREMOR

//THIS IS THE PLATFORM CODE FOR WINDOWS!
//MAC + LINUX WOULD BE ALMOST THE SAME, EXCEPT WITH DIFFERENT VERSION OF FMOD

struct Texture
{
    GLuint id;
    u32 w, h;
};

SDL_GLContext runnerGlContext;
SDL_Window *runnerWindow;

//FMOD
FMOD_SYSTEM *soundSystem;

FMOD_CHANNELGROUP *masterChannelGroup;
FMOD_CHANNELGROUP *sfxChannelGroup;
FMOD_CHANNELGROUP *menuChannelGroup;
FMOD_CHANNELGROUP *worldChannelGroup;
FMOD_CHANNELGROUP *musicChannelGroup;
FMOD_CHANNEL *track1;
FMOD_CHANNEL *track2;

//RENDERING
GLuint screenVBO;
GLuint screenIBO;

GLuint reflectFBO;
GLuint reflectTexture;

//post processing
GLuint postProcessFBO;
GLuint postProcessTexture;

//blur (4 FBO:s per pass)
GLuint glowFBO, blurFBO[4];
GLuint glowTexture, blurTexture[4];
//screenshot buffer
GLuint screenshotFBO;
GLuint screenshotTexture;
Texture screenshot;

glm::mat4 orthoMatrix;

//background shader
struct BackgroundShader
{
    GLuint programID;
    GLuint colorsID;
};

BackgroundShader backgroundShader;

//t shader
GLuint texturedShader;
GLuint mirrorShader;
GLuint spriteShader;
GLuint particleShader;
GLuint modelShader;

GLuint tileShader;

//glow threashold shader
struct GlowThresholdShader
{
    GLuint programID;
    GLuint textureID;
};

GlowThresholdShader glowThresholdShader;

//gaussian blur shader
struct GaussianBlurShader
{
    GLuint programID;
    GLuint textureID;
    GLuint resolutionID;
    GLuint blurDirectionID;
};

GaussianBlurShader gaussianBlurShader;

struct CircleTransitionShader
{
    GLuint programID;
    GLuint positionID;
    GLuint radiusID;
};

CircleTransitionShader circleTransitionShader;

//post process framebuffer to screen shader (final image)
struct FinalImageShader
{
    GLuint programID;
    GLuint textureID;
};

FinalImageShader finalImageShader;

//blending 3d buffer to screen shader
GLuint blendShaderProgram;
GLuint blendVertexLocation;
GLuint blendTextureLocation;
GLuint blendColorUniformIndex;

void *platform_get_pp_texture()
{
    return (void*)&postProcessTexture;
}

void gl_load_shader_source(GLuint shader, std::string fname, std::string preprocessorStr = "#version 430 compatibility\n")
{
    //open file
    std::string shaderString;
    std::ifstream sourceFile(fname.c_str());

    //source file loaded
    if (sourceFile)
    {
        //get shader source
        shaderString.assign((std::istreambuf_iterator<char>(sourceFile)), std::istreambuf_iterator<char>());

        std::string finalShader = preprocessorStr;
        finalShader += shaderString;

        //set shader source
        const GLchar *shaderSource = finalShader.c_str();
        glShaderSource(shader, 1, (const GLchar**)&shaderSource, NULL);
    }
    else
    {
        std::cout << "Unable to open shader file " << fname.c_str() << std::endl;
    }
}

//load, compile, attach and link background shader
void gl_load_background_shader()
{
    backgroundShader.programID = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    gl_load_shader_source(vertexShader, "res/shaders/background_vshader.glvs");
    glCompileShader(vertexShader);
    glAttachShader(backgroundShader.programID, vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_load_shader_source(fragmentShader, "res/shaders/background_fshader.glfs");
    glCompileShader(fragmentShader);
    glAttachShader(backgroundShader.programID, fragmentShader);

    glLinkProgram(backgroundShader.programID);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    /////////////////////////////////////////////////////

    backgroundShader.colorsID = glGetUniformLocation(backgroundShader.programID, "color");
}

void gl_load_circle_transition_shader()
{
    circleTransitionShader.programID = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    gl_load_shader_source(vertexShader, "res/shaders/point_vshader.txt");
    glCompileShader(vertexShader);
    glAttachShader(circleTransitionShader.programID, vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_load_shader_source(fragmentShader, "res/shaders/transition_fshader.txt");
    glCompileShader(fragmentShader);
    glAttachShader(circleTransitionShader.programID, fragmentShader);

    glLinkProgram(circleTransitionShader.programID);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    /////////////////////////////////////////////////////

    circleTransitionShader.positionID = glGetUniformLocation(circleTransitionShader.programID, "circlePos");
    circleTransitionShader.radiusID = glGetUniformLocation(circleTransitionShader.programID, "circleRadius");
}

void gl_load_textured_shader()
{
    texturedShader = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    gl_load_shader_source(vertexShader, "res/shaders/textured_vshader.glvs");
    glCompileShader(vertexShader);
    glAttachShader(texturedShader, vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_load_shader_source(fragmentShader, "res/shaders/textured_fshader.glfs");
    glCompileShader(fragmentShader);
    glAttachShader(texturedShader, fragmentShader);

    glLinkProgram(texturedShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ///////////////////////////////////////////////////

    glBindFragDataLocation(texturedShader, 0, "colorOutput");
}

void gl_load_mirror_shader()
{
    mirrorShader = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    gl_load_shader_source(vertexShader, "res/shaders/mirror_vshader.glvs");
    glCompileShader(vertexShader);
    glAttachShader(mirrorShader, vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_load_shader_source(fragmentShader, "res/shaders/mirror_fshader.glfs");
    glCompileShader(fragmentShader);
    glAttachShader(mirrorShader, fragmentShader);

    glLinkProgram(mirrorShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ///////////////////////////////////////////////////

    glBindFragDataLocation(mirrorShader, 0, "colorOutput");
}

void gl_load_sprite_shader()
{
    spriteShader = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    gl_load_shader_source(vertexShader, "res/shaders/sprite_vshader.txt");
    glCompileShader(vertexShader);
    glAttachShader(spriteShader, vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_load_shader_source(fragmentShader, "res/shaders/sprite_fshader.txt");
    glCompileShader(fragmentShader);
    glAttachShader(spriteShader, fragmentShader);

    glLinkProgram(spriteShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ///////////////////////////////////////////////////

    glBindFragDataLocation(spriteShader, 0, "colorOutput");
}

void gl_load_particle_shader()
{
    particleShader = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    gl_load_shader_source(vertexShader, "res/shaders/particle_vshader.txt");
    glCompileShader(vertexShader);
    glAttachShader(particleShader, vertexShader);

    GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    gl_load_shader_source(geometryShader, "res/shaders/particle_gshader.txt");
    glCompileShader(geometryShader);
    glAttachShader(particleShader, geometryShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_load_shader_source(fragmentShader, "res/shaders/particle_fshader.txt");
    glCompileShader(fragmentShader);
    glAttachShader(particleShader, fragmentShader);

    glLinkProgram(particleShader);
    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    ///////////////////////////////////////////////////

    glBindFragDataLocation(particleShader, 0, "colorOutput");
}

void gl_load_model_shader()
{
    modelShader = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    gl_load_shader_source(vertexShader, "res/shaders/model_vshader.txt");
    glCompileShader(vertexShader);
    glAttachShader(modelShader, vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_load_shader_source(fragmentShader, "res/shaders/model_fshader.txt");
    glCompileShader(fragmentShader);
    glAttachShader(modelShader, fragmentShader);

    glLinkProgram(modelShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ///////////////////////////////////////////////////

    glBindFragDataLocation(modelShader, 0, "colorOutput");
}

void gl_load_tile_shader()
{
    tileShader= glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    gl_load_shader_source(vertexShader, "res/shaders/tile_vshader.txt");
    glCompileShader(vertexShader);
    glAttachShader(tileShader, vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_load_shader_source(fragmentShader, "res/shaders/tile_fshader.txt");
    glCompileShader(fragmentShader);
    glAttachShader(tileShader, fragmentShader);

    glLinkProgram(tileShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ///////////////////////////////////////////////////

    glBindFragDataLocation(tileShader, 0, "colorOutput");
}

void gl_load_glow_threshold_shader()
{
    glowThresholdShader.programID = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    gl_load_shader_source(vertexShader, "res/shaders/deferred_shader.glvs");
    glCompileShader(vertexShader);
    glAttachShader(glowThresholdShader.programID, vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_load_shader_source(fragmentShader, "res/shaders/glowthreshold_shader.glfs");
    glCompileShader(fragmentShader);
    glAttachShader(glowThresholdShader.programID, fragmentShader);

    glLinkProgram(glowThresholdShader.programID);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ///////////////////////////////////////////////////

    //get variable locations
    glowThresholdShader.textureID = glGetUniformLocation(glowThresholdShader.programID, "tex");
}

void gl_load_gaussian_blur_shader()
{
    gaussianBlurShader.programID = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    gl_load_shader_source(vertexShader, "res/shaders/deferred_shader.glvs");
    glCompileShader(vertexShader);
    glAttachShader(gaussianBlurShader.programID, vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_load_shader_source(fragmentShader, "res/shaders/gaussian_blur_shader.glfs");
    glCompileShader(fragmentShader);
    glAttachShader(gaussianBlurShader.programID, fragmentShader);

    glLinkProgram(gaussianBlurShader.programID);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ///////////////////////////////////////////////////

    //get variable locations
    gaussianBlurShader.textureID = glGetUniformLocation(gaussianBlurShader.programID, "tex");
    gaussianBlurShader.resolutionID = glGetUniformLocation(gaussianBlurShader.programID, "resolution");
    gaussianBlurShader.blurDirectionID = glGetUniformLocation(gaussianBlurShader.programID, "dir");
}

void gl_load_final_image_shader()
{
    finalImageShader.programID = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    gl_load_shader_source(vertexShader, "res/shaders/deferred_shader.glvs");
    glCompileShader(vertexShader);
    glAttachShader(finalImageShader.programID, vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_load_shader_source(fragmentShader, "res/shaders/fbotoscreen_shader.glfs");
    glCompileShader(fragmentShader);
    glAttachShader(finalImageShader.programID, fragmentShader);

    glLinkProgram(finalImageShader.programID);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ///////////////////////////////////////////////////

    //get variable locations
    finalImageShader.textureID = glGetUniformLocation(finalImageShader.programID, "tex");
}

void gl_load_blend_shader()
{
    blendShaderProgram = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    gl_load_shader_source(vertexShader, "res/shaders/deferred_shader.glvs");
    glCompileShader(vertexShader);
    glAttachShader(blendShaderProgram, vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_load_shader_source(fragmentShader, "res/shaders/3dtoscreen_shader.txt");
    glCompileShader(fragmentShader);
    glAttachShader(blendShaderProgram, fragmentShader);

    glLinkProgram(blendShaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ///////////////////////////////////////////////////

    //get variable locations
    blendVertexLocation = glGetAttribLocation(blendShaderProgram, "vertexPos");
    blendTextureLocation = glGetUniformLocation(blendShaderProgram, "tex");

    blendColorUniformIndex = glGetUniformBlockIndex(blendShaderProgram, "GlobalColors");
    glUniformBlockBinding(blendShaderProgram, blendColorUniformIndex, 1);
}

void create_screen_vbo()
{
    //give the vertex / index data to the buffers
    v3 vertices[4];
    vertices[0] = {-1, 1, 0};
    vertices[1] = {-1, -1, 0};
    vertices[2] = {1, 1, 0};
    vertices[3] = {1, -1, 0};

    //indices
    u32 indices[6];
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 1;
    indices[4] = 3;
    indices[5] = 2;

    glGenBuffers(1, &screenVBO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(v3), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &screenIBO);
    glBindBuffer(GL_ARRAY_BUFFER, screenIBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(u32), indices, GL_STATIC_DRAW);
}

void delete_screen_vbo()
{
    glDeleteBuffers(1, &screenVBO);
    glDeleteBuffers(1, &screenIBO);
}

void init_renderer()
{
    create_screen_vbo();

    //post processing
    glGenFramebuffers(1, &postProcessFBO);

    glGenTextures(1, &postProcessTexture);
    glBindTexture(GL_TEXTURE_2D, postProcessTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, runnerScreenWidth, runnerScreenHeight + runnerScreenMarginal, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, runnerScreenWidth, runnerScreenHeight + runnerScreenMarginal, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, runnerScreenWidth, runnerScreenHeight  + runnerScreenMarginal, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, runnerScreenWidth / pow(2, i), (runnerScreenHeight + runnerScreenMarginal) / pow(2, i), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, runnerScreenWidth, runnerScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, screenshotFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, screenshotTexture, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    screenshot.id = screenshotTexture;
    screenshot.w = runnerScreenWidth;
    screenshot.h = runnerScreenHeight;

    //LOAD SHADERS
    gl_load_background_shader();
    gl_load_circle_transition_shader();
    gl_load_textured_shader();
    gl_load_mirror_shader();
    gl_load_sprite_shader();
    gl_load_particle_shader();
    gl_load_model_shader();
    gl_load_tile_shader();
    //gl_load_line_shader();

    //gl_load_world_shader();

    //for (int i = 0; i < MAX_LIGHTS; i++)
        //gl_load_world_light_shader(i);

    gl_load_glow_threshold_shader();
    gl_load_gaussian_blur_shader();
    gl_load_final_image_shader();
    //gl_load_blend_shader();

    /////////////////////////////////////////////////////

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
    orthoMatrix = glm::mat4(glm::ortho(0.f, (r32)runnerScreenWidth, (r32)runnerScreenHeight, 0.f, -16384.f, 16384.f));
}

void close_renderer()
{
    //delete background shader
    glDeleteProgram(backgroundShader.programID);
    glDeleteProgram(circleTransitionShader.programID);
    glDeleteProgram(texturedShader);
    glDeleteProgram(mirrorShader);
    glDeleteProgram(spriteShader);
    glDeleteProgram(modelShader);
    glDeleteProgram(tileShader);
    glDeleteProgram(particleShader);

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

    delete_screen_vbo();
}

void platform_clear_buffer()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void platform_swap_buffer()
{
    SDL_GL_SwapWindow(runnerWindow);
}

struct ParticleVertexArrayHandle
{
    GLuint vao;
    GLuint posBuffer;
    GLuint rotBuffer;
    GLuint scaleBuffer;
    GLuint colorBuffer;
    GLuint stateBuffer;

    ParticlePosition *position;
    ParticleRotation *rotation;
    ParticleScale *scale;
    v4 *color;
    float *status;

    u32 vertexCount;
};

struct VertexArrayHandle
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

VertexArrayHandle *platform_create_vertex_array(Mesh *mesh)
{
    VertexArrayHandle *handle = new VertexArrayHandle;

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

void platform_delete_vertex_array(VertexArrayHandle *buf)
{
    glDeleteVertexArrays(1, &buf->vao);
    glDeleteBuffers(1, &buf->posBuffer);
    glDeleteBuffers(1, &buf->texCoordBuffer);
    glDeleteBuffers(1, &buf->normalBuffer);
    glDeleteBuffers(1, &buf->tangentBuffer);
    glDeleteBuffers(1, &buf->indexBuffer);

    delete buf;
}

ParticleVertexArrayHandle *platform_create_particle_vertex_array(u32 pcount,
                                           ParticlePosition *pPosition,
                                           ParticleRotation *pRotation,
                                           ParticleScale *pScale,
                                           v4 *pColor,
                                           float *pStatus)
{
    ParticleVertexArrayHandle *handle = new ParticleVertexArrayHandle;
    handle->vertexCount = pcount;
    handle->position = pPosition;
    handle->rotation = pRotation;
    handle->scale = pScale;
    handle->color = pColor;
    handle->status = pStatus;

    glGenVertexArrays(1, &handle->vao);
    glBindVertexArray(handle->vao);

    glGenBuffers(1, &handle->posBuffer);
    glGenBuffers(1, &handle->rotBuffer);
    glGenBuffers(1, &handle->scaleBuffer);
    glGenBuffers(1, &handle->colorBuffer);
    glGenBuffers(1, &handle->stateBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, handle->posBuffer);
    glBufferData(GL_ARRAY_BUFFER, pcount * sizeof(ParticlePosition), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ParticlePosition), (GLvoid*)offsetof(ParticlePosition, current));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ParticlePosition), (GLvoid*)offsetof(ParticlePosition, previous));

    glBindBuffer(GL_ARRAY_BUFFER, handle->rotBuffer);
    glBufferData(GL_ARRAY_BUFFER, pcount * sizeof(ParticleRotation), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleRotation), (GLvoid*)offsetof(ParticleRotation, current));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleRotation), (GLvoid*)offsetof(ParticleRotation, previous));

    glBindBuffer(GL_ARRAY_BUFFER, handle->scaleBuffer);
    glBufferData(GL_ARRAY_BUFFER, pcount * sizeof(ParticleScale), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleScale), (GLvoid*)offsetof(ParticleScale, current));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleScale), (GLvoid*)offsetof(ParticleScale, previous));

    glBindBuffer(GL_ARRAY_BUFFER, handle->colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, pcount * sizeof(v4), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, handle->stateBuffer);
    glBufferData(GL_ARRAY_BUFFER, pcount * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);

    return handle;
}

void platform_update_particle_vertex_array(ParticleVertexArrayHandle *buf)
{
    glBindBuffer(GL_ARRAY_BUFFER, buf->posBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, buf->vertexCount * sizeof(ParticlePosition), (GLvoid*)buf->position);

    glBindBuffer(GL_ARRAY_BUFFER, buf->rotBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, buf->vertexCount * sizeof(ParticleRotation), (GLvoid*)buf->rotation);

    glBindBuffer(GL_ARRAY_BUFFER, buf->scaleBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, buf->vertexCount * sizeof(ParticleScale), (GLvoid*)buf->scale);

    glBindBuffer(GL_ARRAY_BUFFER, buf->colorBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, buf->vertexCount * sizeof(v4), (GLvoid*)buf->color);

    glBindBuffer(GL_ARRAY_BUFFER, buf->stateBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, buf->vertexCount * sizeof(float), (GLvoid*)buf->status);

    glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

void platform_delete_particle_vertex_array(ParticleVertexArrayHandle *buf)
{
    glDeleteVertexArrays(1, &buf->vao);
    glDeleteBuffers(1, &buf->posBuffer);
    glDeleteBuffers(1, &buf->rotBuffer);
    glDeleteBuffers(1, &buf->scaleBuffer);
    glDeleteBuffers(1, &buf->colorBuffer);
    glDeleteBuffers(1, &buf->stateBuffer);

    delete buf;
}

////////////////////////////
void gl_render_generic_framebuffer()
{
    //Set vertex pos data
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenIBO);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
}

void platform_render_background(v3 *colors)
{
    //glViewport(0.f, 0.f, runnerScreenWidth, runnerScreenHeight);
    glUseProgram(backgroundShader.programID);

    //Set color data
    glUniform3fv(backgroundShader.colorsID, 2, (GLfloat*)colors);

    gl_render_generic_framebuffer();
}

void platform_render_circle_transition(v2 pos, r32 radius)
{
    glUseProgram(circleTransitionShader.programID);

    glUniform2f(circleTransitionShader.positionID, pos.x, runnerScreenHeight - pos.y);
    glUniform1f(circleTransitionShader.radiusID, radius);

    gl_render_generic_framebuffer();
}

/*
void platform_render_world_element(InstanceDataProcessed *instanceData,
                                   v4 *color,
                                   u32 *frame,
                                   u32 instanceCount,
                            Texture *diffuse,
                            Texture *lightmap,
                            Texture *normal,
                            VertexArrayHandle *vbuffer,
                            ClipBufferHandle *clipBuffer,
                            r32 glow,
                            bool32 blend,
                            v4 ambient,
                            u32 lightAmount,
                            Light *lights)
{
    if (!blend)
        return;

    if (!blend)
    {
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthRange(1,0);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer3dFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if (!lightAmount)
        platform_render_world(instanceData, color, frame, instanceCount, diffuse, lightmap, vbuffer, clipBuffer, glow, ambient);
    else platform_render_world_light(instanceData, color, frame, instanceCount, diffuse, lightmap, normal, vbuffer, clipBuffer, glow, ambient, lightAmount, lights);

    if (!blend)
    {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, reflectFBO);

        glUseProgram(blendShaderProgram);
        glEnableVertexAttribArray(blendVertexLocation);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buffer3dTexture);

        //set color
        glBindBuffer(GL_UNIFORM_BUFFER, colorBuffer);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), &color);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        gl_render_generic_framebuffer(blendVertexLocation);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void platform_render_world(InstanceDataProcessed *instanceData,
                           v4 *color,
                            u32 *frame,
                           u32 instanceCount,
                           Texture *texture,
                            Texture *lightmap,
                            VertexArrayHandle *vbuffer,
                            ClipBufferHandle *clipBuffer,
                            r32 glow,
                            v4 ambient)
{
    //if (frame >= vbuffer->bufferCount)
        //throw std::runtime_error("render frame is larger than max!");
    glViewport(0.f, 0.f, runnerScreenWidth, runnerScreenHeight);
    glUseProgram(worldShaderProgram);
    glEnableVertexAttribArray(worldVertexLocation);
    glEnableVertexAttribArray(worldTexcoordLocation);

    glEnableVertexAttribArray(worldModelviewLocation);
    glEnableVertexAttribArray(worldModelviewLocation + 1);
    glEnableVertexAttribArray(worldModelviewLocation + 2);
    glEnableVertexAttribArray(worldModelviewLocation + 3);
    glEnableVertexAttribArray(worldFrameLocation);
    glEnableVertexAttribArray(worldColorLocation);

    //set matrices
    glm::mat4 orthoMatrix(glm::ortho(0.f, (r32)runnerScreenWidth, (r32)runnerScreenHeight, 0.f, -16384.f, 16384.f));

    //bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glUniform1i(worldTextureLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, lightmap->id);
    glUniform1i(worldLightmapLocation, 1);

    //glow
    glUniform1f(worldGlowLocation, glow);

    //ambient color
    glUniform3fv(worldAmbientColorLocation, 1, (GLfloat*)&ambient);

    //matrices
    glUniformMatrix4fv(worldProjectionLocation, 1, GL_FALSE, glm::value_ptr(orthoMatrix));

    //instance buffer stuffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceMatrixBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(InstanceDataProcessed) * instanceCount, instanceData);
    glVertexAttribPointer(worldModelviewLocation, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix));
    glVertexAttribPointer(worldModelviewLocation + 1, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix) + 16);
    glVertexAttribPointer(worldModelviewLocation + 2, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix) + 32);
    glVertexAttribPointer(worldModelviewLocation + 3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix) + 48);
    glVertexAttribDivisor(worldModelviewLocation, 1);
    glVertexAttribDivisor(worldModelviewLocation + 1, 1);
    glVertexAttribDivisor(worldModelviewLocation + 2, 1);
    glVertexAttribDivisor(worldModelviewLocation + 3, 1);
    glBindBuffer(GL_ARRAY_BUFFER, instanceFrameBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(u32) * instanceCount, frame);
    glVertexAttribIPointer(worldFrameLocation, 1, GL_UNSIGNED_INT, sizeof(u32), 0);
    glVertexAttribDivisor(worldFrameLocation, 1);
    glBindBuffer(GL_ARRAY_BUFFER, instanceColorBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v4) * instanceCount, color);
    glVertexAttribPointer(worldColorLocation, 4, GL_FLOAT, GL_FALSE, sizeof(v4), 0);
    glVertexAttribDivisor(worldColorLocation, 1);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, clipBuffer->id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, clipBuffer->id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //give the vertex / index data to the buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbuffer->vbo);
    glVertexAttribPointer(worldVertexLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, pos));
    glVertexAttribPointer(worldTexcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, uv));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbuffer->ibo);

    //draw vertex arrays
    glDrawElementsInstanced(GL_TRIANGLES, vbuffer->triangleCount * 3, GL_UNSIGNED_INT, NULL, instanceCount);
}

void platform_render_world_light(InstanceDataProcessed *instanceData,
                                 v4 *color,
                                   u32 *frame,
                                 u32 instanceCount,
                                 Texture *texture,
                                    Texture *lightmap,
                                    Texture *normal,
                                    VertexArrayHandle *vbuffer,
                                    ClipBufferHandle *clipBuffer,
                                    r32 glow,
                                    v4 ambient,
                                    u32 lightAmount,
                                    Light *lights)
{
    //if (frame >= vbuffer->bufferCount)
        //throw std::runtime_error("render frame is larger than max!");
    TIMED_BLOCK;

    u32 i = lightAmount - 1;

    glViewport(0.f, 0.f, runnerScreenWidth, runnerScreenHeight);
    glUseProgram(worldLightShaders[i].shaderProgram);
    glEnableVertexAttribArray(worldLightShaders[i].vertexLocation);
    glEnableVertexAttribArray(worldLightShaders[i].texcoordLocation);
    glEnableVertexAttribArray(worldLightShaders[i].vertNormLocation);
    glEnableVertexAttribArray(worldLightShaders[i].vertTanLocation);

    glEnableVertexAttribArray(worldLightShaders[i].modelviewLocation);
    glEnableVertexAttribArray(worldLightShaders[i].modelviewLocation + 1);
    glEnableVertexAttribArray(worldLightShaders[i].modelviewLocation + 2);
    glEnableVertexAttribArray(worldLightShaders[i].modelviewLocation + 3);
    glEnableVertexAttribArray(worldLightShaders[i].frameLocation);
    glEnableVertexAttribArray(worldLightShaders[i].colorLocation);

    //set matrices
    glm::mat4 orthoMatrix(glm::ortho(0.f, (r32)runnerScreenWidth, (r32)runnerScreenHeight, 0.f, -16384.f, 16384.f));


    //bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glUniform1i(worldLightShaders[i].textureLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, lightmap->id);
    glUniform1i(worldLightShaders[i].lightmapLocation, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normal->id);
    glUniform1i(worldLightShaders[i].normalLocation, 2);

    //glow
    glUniform1f(worldLightShaders[i].glowLocation, glow);

    //ambient color
    glUniform3fv(worldLightShaders[i].ambientColorLocation, 1, (GLfloat*)&ambient);

    //matrices
    glUniformMatrix4fv(worldLightShaders[i].projectionLocation, 1, GL_FALSE, glm::value_ptr(orthoMatrix));

    //set lights
    for (size_t j = 0; j < lightAmount; j++)
    {
        glUniform3fv(worldLightShaders[i].lightPosLocation + j*2, 1, (GLfloat*)&lights[j].position);
        glUniform4fv(worldLightShaders[i].lightColorLocation + j*2, 1, (GLfloat*)&lights[j].color);
    }

    //instance buffer stuffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceMatrixBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(InstanceDataProcessed) * instanceCount, instanceData);
    glVertexAttribPointer(worldLightShaders[i].modelviewLocation, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix));
    glVertexAttribPointer(worldLightShaders[i].modelviewLocation + 1, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix) + 16);
    glVertexAttribPointer(worldLightShaders[i].modelviewLocation + 2, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix) + 32);
    glVertexAttribPointer(worldLightShaders[i].modelviewLocation + 3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix) + 48);
    glVertexAttribDivisor(worldLightShaders[i].modelviewLocation, 1);
    glVertexAttribDivisor(worldLightShaders[i].modelviewLocation + 1, 1);
    glVertexAttribDivisor(worldLightShaders[i].modelviewLocation + 2, 1);
    glVertexAttribDivisor(worldLightShaders[i].modelviewLocation + 3, 1);
    glBindBuffer(GL_ARRAY_BUFFER, instanceFrameBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(u32) * instanceCount, frame);
    glVertexAttribIPointer(worldLightShaders[i].frameLocation, 1, GL_UNSIGNED_INT, sizeof(u32), 0);
    glVertexAttribDivisor(worldLightShaders[i].frameLocation, 1);
    glBindBuffer(GL_ARRAY_BUFFER, instanceColorBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v4) * instanceCount, color);
    glVertexAttribPointer(worldLightShaders[i].colorLocation, 4, GL_FLOAT, GL_FALSE, sizeof(v4), 0);
    glVertexAttribDivisor(worldLightShaders[i].colorLocation, 1);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, clipBuffer->id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, clipBuffer->id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //give the vertex / index data to the buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbuffer->vbo);
    glVertexAttribPointer(worldLightShaders[i].vertexLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, pos));
    glVertexAttribPointer(worldLightShaders[i].texcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, uv));
    glVertexAttribPointer(worldLightShaders[i].vertNormLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, normal));
    glVertexAttribPointer(worldLightShaders[i].vertTanLocation, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, tangent));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbuffer->ibo);

    //draw vertex arrays
    glDrawElementsInstanced(GL_TRIANGLES, vbuffer->triangleCount * 3, GL_UNSIGNED_INT, NULL, instanceCount);
}

void platform_render_textured(InstanceDataProcessed *instanceData,
                              v4 *color,
                                   u32 *frame,
                                u32 instanceCount,
                              Texture *texture,
                              VertexArrayHandle *vbuffer,
                              ClipBufferHandle *clipBuffer)
{
    //if (frame >= vbuffer->bufferCount)
        //throw std::runtime_error("render frame is larger than max!");

    glViewport(0.f, 0.f, runnerScreenWidth, runnerScreenHeight);
    glUseProgram(texturedShaderProgram);
    glEnableVertexAttribArray(texturedVertexLocation);
    glEnableVertexAttribArray(texturedTexcoordLocation);

    glEnableVertexAttribArray(texturedModelviewLocation);
    glEnableVertexAttribArray(texturedModelviewLocation + 1);
    glEnableVertexAttribArray(texturedModelviewLocation + 2);
    glEnableVertexAttribArray(texturedModelviewLocation + 3);
    glEnableVertexAttribArray(texturedFrameLocation);
    glEnableVertexAttribArray(texturedColorLocation);

    //set matrices
    glm::mat4 orthoMatrix(glm::ortho(0.f, (r32)runnerScreenWidth, (r32)runnerScreenHeight, 0.f, -16384.f, 16384.f));

    //bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glUniform1i(texturedTextureLocation, 0);

    //matrices
    glUniformMatrix4fv(texturedProjectionLocation, 1, GL_FALSE, glm::value_ptr(orthoMatrix));

    //instance buffer stuffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceMatrixBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(InstanceDataProcessed) * instanceCount, instanceData);
    glVertexAttribPointer(texturedModelviewLocation, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix));
    glVertexAttribPointer(texturedModelviewLocation + 1, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix) + 16);
    glVertexAttribPointer(texturedModelviewLocation + 2, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix) + 32);
    glVertexAttribPointer(texturedModelviewLocation + 3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix) + 48);
    glVertexAttribDivisor(texturedModelviewLocation, 1);
    glVertexAttribDivisor(texturedModelviewLocation + 1, 1);
    glVertexAttribDivisor(texturedModelviewLocation + 2, 1);
    glVertexAttribDivisor(texturedModelviewLocation + 3, 1);
    glBindBuffer(GL_ARRAY_BUFFER, instanceFrameBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(u32) * instanceCount, frame);
    glVertexAttribIPointer(texturedFrameLocation, 1, GL_UNSIGNED_INT, sizeof(u32), 0);
    glVertexAttribDivisor(texturedFrameLocation, 1);
    glBindBuffer(GL_ARRAY_BUFFER, instanceColorBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v4) * instanceCount, color);
    glVertexAttribPointer(texturedColorLocation, 4, GL_FLOAT, GL_FALSE, sizeof(v4), 0);
    glVertexAttribDivisor(texturedColorLocation, 1);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, clipBuffer->id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, clipBuffer->id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //give the vertex / index data to the buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbuffer->vbo);
    glVertexAttribPointer(texturedVertexLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, pos));
    glVertexAttribPointer(texturedTexcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, uv));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbuffer->ibo);

    //draw vertex arrays
    glDrawElementsInstanced(GL_TRIANGLES, vbuffer->triangleCount * 3, GL_UNSIGNED_INT, NULL, instanceCount);
}

void platform_render_shape(InstanceDataProcessed *instanceData,
                           v4 *color,
                           u32 instanceCount,
                           VertexArrayHandle *vbuffer,
                           bool32 fill)
{
    //if (frame >= vbuffer->bufferCount)
        //throw std::runtime_error("render frame is larger than max!");

    glViewport(0.f, 0.f, runnerScreenWidth, runnerScreenHeight);

    glUseProgram(lineShaderProgram);
    glEnableVertexAttribArray(lineVertexLocation);
    glEnableVertexAttribArray(lineModelviewLocation);
    glEnableVertexAttribArray(lineModelviewLocation + 1);
    glEnableVertexAttribArray(lineModelviewLocation + 2);
    glEnableVertexAttribArray(lineModelviewLocation + 3);
    glEnableVertexAttribArray(lineColorLocation);

    //set matrices
    glm::mat4 orthoMatrix(glm::ortho(0.f, (r32)runnerScreenWidth, (r32)runnerScreenHeight, 0.f, -16384.f, 16384.f));

    //matrices
    glUniformMatrix4fv(lineProjectionLocation, 1, GL_FALSE, glm::value_ptr(orthoMatrix));

    //instance buffer stuffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceMatrixBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(InstanceDataProcessed) * instanceCount, instanceData);
    glVertexAttribPointer(lineModelviewLocation, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix));
    glVertexAttribPointer(lineModelviewLocation + 1, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix) + 16);
    glVertexAttribPointer(lineModelviewLocation + 2, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix) + 32);
    glVertexAttribPointer(lineModelviewLocation + 3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataProcessed), (GLvoid*)offsetof(InstanceDataProcessed, modelviewMatrix) + 48);
    glVertexAttribDivisor(lineModelviewLocation, 1);
    glVertexAttribDivisor(lineModelviewLocation + 1, 1);
    glVertexAttribDivisor(lineModelviewLocation + 2, 1);
    glVertexAttribDivisor(lineModelviewLocation + 3, 1);
    glBindBuffer(GL_ARRAY_BUFFER, instanceColorBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v4) * instanceCount, color);
    glVertexAttribPointer(lineColorLocation, 4, GL_FLOAT, GL_FALSE, sizeof(v4), 0);
    glVertexAttribDivisor(lineColorLocation, 1);

    //give the vertex / index data to the buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbuffer->vbo);
    glVertexAttribPointer(lineVertexLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, pos));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbuffer->ibo);

    //draw vertex arrays
    GLenum mode = GL_LINE_LOOP;
    if (fill)
        mode = GL_TRIANGLE_FAN;

    glDrawElementsInstanced(mode, vbuffer->triangleCount * 3, GL_UNSIGNED_INT, NULL, instanceCount);
}*/

void platform_bind_vao(VertexArrayHandle *vao)
{
    if (vao)
        glBindVertexArray(vao->vao);
    else glBindVertexArray(0);
}

void platform_use_ui_shader()
{
    glUseProgram(texturedShader);
}

void platform_set_projection()
{
    glUniformMatrix4fv(PROJECTION_LOCATION, 1, GL_FALSE, glm::value_ptr(orthoMatrix));
}

void platform_render_hud_element(Transform xform, Texture *texture, v4 *clipRect, v2 *offset, v2 *flip, v4 *color)
{
    glm::mat4 modelview = glm::mat4(1.0f);
    modelview = glm::translate(modelview, glm::vec3(xform.position.x,xform.position.y,0));
    modelview = glm::rotate(modelview, (float)xform.rotation.x, glm::vec3(1, 0, 0));
    modelview= glm::rotate(modelview, (float)xform.rotation.y, glm::vec3(0, 1, 0));
    modelview = glm::rotate(modelview, (float)xform.rotation.z, glm::vec3(0, 0, 1));
    modelview = glm::scale(modelview, glm::vec3(xform.scale.x,xform.scale.y,xform.scale.z));
    glUniformMatrix4fv(MODELVIEW_LOCATION, 1, GL_FALSE, glm::value_ptr(modelview));

    glUniform4fv(SPRITE_CLIPRECT_LOCATION, 1, (GLfloat*)clipRect);
    glUniform2fv(SPRITE_OFFSET_LOCATION, 1, (GLfloat*)offset);
    glUniform2fv(SPRITE_FLIP_LOCATION, 1, (GLfloat*)flip);
    glUniform4fv(SPRITE_COLOR_LOCATION, 1, (GLfloat*)color);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glUniform1i(TEXTURE_LOCATION, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void platform_use_mirror_shader()
{
    glUseProgram(mirrorShader);
}

void platform_render_mirror(Transform xform, v4 *clipRect)
{
    glm::mat4 modelview = glm::mat4(1.0f);
    modelview = glm::translate(modelview, glm::vec3(xform.position.x,xform.position.y,0));
    modelview = glm::rotate(modelview, (float)xform.rotation.x, glm::vec3(1, 0, 0));
    modelview= glm::rotate(modelview, (float)xform.rotation.y, glm::vec3(0, 1, 0));
    modelview = glm::rotate(modelview, (float)xform.rotation.z, glm::vec3(0, 0, 1));
    modelview = glm::scale(modelview, glm::vec3(xform.scale.x,xform.scale.y,xform.scale.z));
    glUniformMatrix4fv(MODELVIEW_LOCATION, 1, GL_FALSE, glm::value_ptr(modelview));

    glUniform4fv(SPRITE_CLIPRECT_LOCATION, 1, (GLfloat*)clipRect);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postProcessTexture);
    glUniform1i(TEXTURE_LOCATION, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void platform_render_reflections()
{
    glUseProgram(finalImageShader.programID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, reflectTexture);

    glViewport(0.f, 0.f, runnerScreenWidth, runnerScreenHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, postProcessFBO);
    //glClear(GL_COLOR_BUFFER_BIT);

    gl_render_generic_framebuffer();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void platform_use_sprite_shader()
{
    glUseProgram(spriteShader);
}

void platform_set_lights(Light *lights, u32 lightCount)
{
    glUniform1ui(LIGHT_AMOUNT_LOCATION, lightCount);
    for (size_t j = 0; j < lightCount; j++)
    {
        glUniform3fv(LIGHT_LOCATION + j*2, 1, (GLfloat*)&lights[j].position);
        glUniform4fv(LIGHT_LOCATION + 1 + j*2, 1, (GLfloat*)&lights[j].color);
    }
}

void platform_set_ambient_color(v3 *color)
{
    glUniform3fv(AMBIENT_COLOR_LOCATION, 1, (GLfloat*)color);
}

void platform_render_sprite(Transform xform, Texture *texture, Texture *lightmap, Texture *normal, v4 *clipRect, v2 *offset, v2 *flip, v4 *color, r32 glow)
{
    glm::mat4 modelview = glm::mat4(1.0f);
    modelview = glm::translate(modelview, glm::vec3(xform.position.x,xform.position.y,0));
    modelview = glm::rotate(modelview, (float)xform.rotation.x, glm::vec3(1, 0, 0));
    modelview= glm::rotate(modelview, (float)xform.rotation.y, glm::vec3(0, 1, 0));
    modelview = glm::rotate(modelview, (float)xform.rotation.z, glm::vec3(0, 0, 1));
    modelview = glm::scale(modelview, glm::vec3(xform.scale.x,xform.scale.y,xform.scale.z));
    glUniformMatrix4fv(MODELVIEW_LOCATION, 1, GL_FALSE, glm::value_ptr(modelview));

    glUniform4fv(SPRITE_CLIPRECT_LOCATION, 1, (GLfloat*)clipRect);
    glUniform2fv(SPRITE_OFFSET_LOCATION, 1, (GLfloat*)offset);
    glUniform2fv(SPRITE_FLIP_LOCATION, 1, (GLfloat*)flip);
    glUniform4fv(SPRITE_COLOR_LOCATION, 1, (GLfloat*)color);
    glUniform1f(GLOW_LOCATION, glow);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glUniform1i(TEXTURE_LOCATION, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, lightmap->id);
    glUniform1i(LIGHTMAP_LOCATION, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normal->id);
    glUniform1i(NORMALMAP_LOCATION, 2);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

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

void platform_use_model_shader()
{
    glUseProgram(modelShader);
}

void platform_render_model(Transform xform, VertexArrayHandle *vao, Texture *texture, Texture *lightmap, Texture *normal, r32 glow)
{
    glBindVertexArray(vao->vao);

    glm::mat4 modelview = glm::mat4(1.0f);
    modelview = glm::translate(modelview, glm::vec3(xform.position.x,xform.position.y,0));
    modelview = glm::rotate(modelview, (float)xform.rotation.x, glm::vec3(1, 0, 0));
    modelview = glm::rotate(modelview, (float)xform.rotation.y, glm::vec3(0, 1, 0));
    modelview = glm::rotate(modelview, (float)xform.rotation.z, glm::vec3(0, 0, 1));
    modelview = glm::scale(modelview, glm::vec3(xform.scale.x,xform.scale.y,xform.scale.z));
    glUniformMatrix4fv(MODELVIEW_LOCATION, 1, GL_FALSE, glm::value_ptr(modelview));

    glUniform1f(GLOW_LOCATION, glow);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glUniform1i(TEXTURE_LOCATION, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, lightmap->id);
    glUniform1i(LIGHTMAP_LOCATION, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normal->id);
    glUniform1i(NORMALMAP_LOCATION, 2);

    glDrawElements(GL_TRIANGLES, vao->triangleCount * 3, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}

void platform_use_tile_shader()
{
    glUseProgram(tileShader);
}

void platform_render_tiles(v3 *position, Texture *layout, Texture *texture, Texture *lightmap, Texture *normal)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glUniform1i(TEXTURE_LOCATION, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, lightmap->id);
    glUniform1i(LIGHTMAP_LOCATION, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normal->id);
    glUniform1i(NORMALMAP_LOCATION, 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, layout->id);
    glUniform1i(TILES_LAYOUT_LOCATION, 3);

    glUniform3fv(TILES_POSITION_LOCATION, 1, (GLfloat*)position);

    gl_render_generic_framebuffer();
}

void platform_set_camera_pos(v2 current, v2 previous)
{
    glUniform2fv(CAMERA_POS_LOCATION, 1, (GLfloat*)&current);
    glUniform2fv(CAMERA_PREV_POS_LOCATION, 1, (GLfloat*)&previous);
}
void platform_use_particle_shader(u32 lightCount)
{
    glUseProgram(particleShader);
}
void platform_render_particles(ParticleVertexArrayHandle *vao, float a, Texture *texture, Texture *lightmap, Texture *normal, v4 *clipRect, v2 *offset, v2 *flip, r32 glow)
{
    glBindVertexArray(vao->vao);

    glUniform1f(GLOW_LOCATION, glow);
    glUniform1f(TIME_ALPHA_LOCATION, a);

    glUniform4fv(SPRITE_CLIPRECT_LOCATION, 1, (GLfloat*)clipRect);
    glUniform2fv(SPRITE_OFFSET_LOCATION, 1, (GLfloat*)offset);
    glUniform2fv(SPRITE_FLIP_LOCATION, 1, (GLfloat*)flip);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glUniform1i(TEXTURE_LOCATION, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, lightmap->id);
    glUniform1i(LIGHTMAP_LOCATION, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normal->id);
    glUniform1i(NORMALMAP_LOCATION, 2);

    glDrawArrays(GL_POINTS, 0, vao->vertexCount);
    glBindVertexArray(0);
}

//////////////////////////////

void platform_bind_framebuffer_reflect()
{
    glBindFramebuffer(GL_FRAMEBUFFER, reflectFBO);
    glClear(GL_COLOR_BUFFER_BIT);
}

void platform_bind_framebuffer_post_process()
{
    glBindFramebuffer(GL_FRAMEBUFFER, postProcessFBO);
}

void platform_unbind_framebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void gl_render_glow_threshold()
{
    glUseProgram(glowThresholdShader.programID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postProcessTexture);

    glViewport(0.f, 0.f, runnerScreenWidth, runnerScreenHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, glowFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    gl_render_generic_framebuffer();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void gl_render_gaussian_blur()
{
    glUseProgram(gaussianBlurShader.programID);

    //pass one (horizontal)
    glUniform2f(gaussianBlurShader.blurDirectionID, 1.0, 0.0);

    glActiveTexture(GL_TEXTURE0);
    for (s32 i = 0; i <= 3; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[i]);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, glowTexture);

        r32 texWidth = runnerScreenWidth / pow(2, i);
        r32 texHeight = runnerScreenHeight /  pow(2, i);
        glUniform2f(gaussianBlurShader.resolutionID, texWidth, texHeight);

        glViewport(0.f, 0.f, texWidth, texHeight);

        gl_render_generic_framebuffer();
    }

    //pass two (vertical)
    glUniform2f(gaussianBlurShader.blurDirectionID, 0.0, 1.0);

    glBindFramebuffer(GL_FRAMEBUFFER, postProcessFBO);
    glViewport(0.f, 0.f, runnerScreenWidth, runnerScreenHeight);
    //set blending to additive
    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

    for (s32 i = 0; i <= 3; i++)
    {
        glBindTexture(GL_TEXTURE_2D, blurTexture[i]);

        r32 texWidth = runnerScreenWidth / pow(2, i);
        r32 texHeight = runnerScreenHeight /  pow(2, i);
        glUniform2f(gaussianBlurShader.resolutionID, texWidth, texHeight);

        gl_render_generic_framebuffer();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, postProcessTexture);

    glBindTexture(GL_TEXTURE_2D, NULL);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
}

void platform_render_bloom()
{
    gl_render_glow_threshold();
    gl_render_gaussian_blur();
}

void platform_take_screenshot()
{
    glBindFramebuffer(GL_FRAMEBUFFER, screenshotFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    //glViewport(0.f, 0.f, runnerScreenWidth, runnerScreenHeight);

    glUseProgram(finalImageShader.programID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postProcessTexture);

    gl_render_generic_framebuffer();

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void platform_render_final_image()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glUseProgram(finalImageShader.programID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postProcessTexture);

    gl_render_generic_framebuffer();

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture *platform_get_screenshot()
{
    return &screenshot;
}

void platform_set_fullscreen(bool s)
{
    if (s)
    {
        SDL_ShowCursor(false);
        SDL_SetWindowFullscreen(runnerWindow, SDL_WINDOW_FULLSCREEN);
    }
    else
    {
        SDL_ShowCursor(true);
        SDL_SetWindowFullscreen(runnerWindow, 0);
    }
}

//AUDIO

struct Sound
{
    //Mix_Chunk *chunk;
    FMOD_SOUND *sound;
};

struct Music
{
    FMOD_SOUND *sound;
};

u32 platform_play_world_sound(Sound *sound, s32 loops, fvec2 *pos, fvec2 *vel)
{
    if (!sound)
        return NULL;
    /*Mix_VolumeChunk(sound->chunk, MIX_MAX_VOLUME * volume);
    Mix_PlayChannel(channel, sound->chunk, loops);
    Mix_ChannelFinished(channel_finished);*/
    FMOD_CHANNEL *chan;
    FMOD_System_PlaySound(soundSystem, sound->sound, worldChannelGroup, 1, &chan);

    FMOD_Channel_Set3DLevel(chan, 1.0f);

    FMOD_VECTOR position = {0, 0, 0};
    FMOD_VECTOR velocity = {0, 0, 0};

    if (pos)
    {
        position.x = pixels_to_meters(pos->x);
        position.y = pixels_to_meters(pos->y);
    }
    if (vel)
    {
        velocity.x = vel->x;
        velocity.y = vel->y;
    }

    FMOD_Channel_Set3DAttributes(chan, &position, &velocity, NULL);
    FMOD_Channel_SetLoopCount(chan, loops);
    FMOD_Channel_Set3DMinMaxDistance(chan, 1.0, 32.0);

    FMOD_Channel_SetPaused(chan, false);

    return (intptr_t)chan;

}
u32 platform_play_menu_sound(Sound *sound, s32 loops)
{
    if (!sound)
        return NULL;

    FMOD_CHANNEL *chan;
    FMOD_System_PlaySound(soundSystem, sound->sound, menuChannelGroup, 1, &chan);

    FMOD_Channel_Set3DLevel(chan, 0.0f);
    FMOD_Channel_SetLoopCount(chan, loops);
    FMOD_Channel_SetPaused(chan, false);

    return (intptr_t)chan;
}
void platform_stop_channel(u32 channel)
{
    //Mix_HaltChannel(channel);
    if (channel)
    {
        FMOD_Channel_Stop((FMOD_CHANNEL*)channel);
    }
}
void platform_pause_channel(u32 channel)
{
    //Mix_Pause(channel);
    if (channel)
    {
        FMOD_Channel_SetPaused((FMOD_CHANNEL*)channel, true);
    }
}
void platform_resume_channel(u32 channel)
{
    //Mix_Resume(channel);
    if (channel)
    {
        FMOD_Channel_SetPaused((FMOD_CHANNEL*)channel, false);
    }
}
u32 platform_play_music(Music *music, bool32 mute, s32 fade, r64 pos)
{
    if (!music)
        return NULL;

    FMOD_CHANNEL *chan;
    FMOD_System_PlaySound(soundSystem, music->sound, musicChannelGroup, 1, &chan);

    //fade in
    /*if (fade)
    {
        unsigned long long dspclock;
        int rate;

        FMOD_System_GetSoftwareFormat(soundSystem, &rate, NULL, NULL);

        FMOD_ChannelGroup_GetDSPClock(musicChannelGroup, &dspclock, NULL);
        FMOD_Channel_AddFadePoint(track1, dspclock, 0.0f);
        FMOD_Channel_AddFadePoint(track1, dspclock + (fade * rate), 1.0f);
    }*/

    FMOD_Channel_SetMute(chan, mute);
    FMOD_Channel_SetLoopCount(chan, -1);
    FMOD_Channel_SetPaused(chan, false);

    return (intptr_t)chan;
}
void platform_stop_music(s32 fade)
{
    /*if (fade)
    {
        unsigned long long dspclock;
        int rate;

        FMOD_System_GetSoftwareFormat(soundSystem, &rate, NULL, NULL);

        FMOD_ChannelGroup_GetDSPClock(musicChannelGroup, &dspclock, NULL);
        FMOD_Channel_AddFadePoint(track1, dspclock, 1.0f);
        FMOD_Channel_AddFadePoint(track1, dspclock + (fade * rate), 0.0f);
        FMOD_Channel_SetDelay(track1, 0, dspclock + (fade * rate), true);
    }*/
    FMOD_ChannelGroup_Stop(musicChannelGroup);
}
void platform_set_sfx_volume(r32 volume)
{
    //Mix_Volume(-1, MIX_MAX_VOLUME * volume);
    FMOD_ChannelGroup_SetVolume(sfxChannelGroup, volume);
}
void platform_set_music_volume(r32 volume)
{
    //Mix_VolumeMusic(MIX_MAX_VOLUME * volume);
    FMOD_ChannelGroup_SetVolume(musicChannelGroup, volume);
}
bool platform_music_is_playing()
{
    //return Mix_PlayingMusic();
    bool32 result;
    FMOD_ChannelGroup_IsPlaying(musicChannelGroup, &result);
    return result;
}

//FMOD
void platform_update_audio()
{
    FMOD_System_Update(soundSystem);
}
void platform_set_listener_attributes(fvec2 *pos, fvec2 *vel)
{
    FMOD_VECTOR position = {0, 0, 5};
    FMOD_VECTOR velocity = {0, 0, 0};

    if (pos)
    {
        position.x = pixels_to_meters(pos->x);
        position.y = pixels_to_meters(pos->y);
    }
    if (vel)
    {
        velocity.x = vel->x;
        velocity.y = vel->y;
    }

    FMOD_System_Set3DListenerAttributes(soundSystem, 0, &position, &velocity, NULL, NULL);
}
void platform_pause_world_sounds()
{
    FMOD_ChannelGroup_SetPaused(worldChannelGroup, true);
}
void platform_resume_world_sounds()
{
    FMOD_ChannelGroup_SetPaused(worldChannelGroup, false);
}
void platform_stop_world_sounds()
{
    FMOD_ChannelGroup_Stop(worldChannelGroup);
}
void platform_stop_menu_sounds()
{
    FMOD_ChannelGroup_Stop(menuChannelGroup);
}

void platform_mute_channel(s32 channel, bool32 mute)
{
    FMOD_CHANNEL *chan = (FMOD_CHANNEL*)channel;
    FMOD_Channel_SetMute(chan, mute);
}
void platform_fade_channel(s32 channel, r32 startVol, r32 endVol, r32 length)
{
    FMOD_CHANNEL *chan = (FMOD_CHANNEL*)channel;
    unsigned long long dspclock;
    int rate;

    FMOD_System_GetSoftwareFormat(soundSystem, &rate, NULL, NULL);

    FMOD_ChannelGroup_GetDSPClock(musicChannelGroup, &dspclock, NULL);
    FMOD_Channel_AddFadePoint(chan, dspclock, startVol);
    FMOD_Channel_AddFadePoint(chan, dspclock + (length * rate), endVol);
}

//INPUT
SDL_GameController *gameController = NULL;
SDL_Haptic *haptic = NULL;

void platform_init_controller()
{
    if (SDL_NumJoysticks())
    {
        gameController = SDL_GameControllerOpen(0);
        haptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(gameController));
        SDL_HapticRumbleInit(haptic);
    }
    else gameController = NULL;
}

void platform_deinit_controller()
{
    if (gameController)
        SDL_GameControllerClose(gameController);
    if (haptic)
    {
        SDL_HapticRumbleStop(haptic);
        SDL_HapticClose(haptic);
    }
}

bool platform_controller_exists()
{
    return (gameController != 0);
}

void handle_keyboard_event(SDL_KeyboardEvent &event, Input *input)
{
    switch(event.keysym.scancode)
    {
    case SDL_SCANCODE_LEFT:
        input->keyboard.keyLeft = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_RIGHT:
        input->keyboard.keyRight = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_UP:
        input->keyboard.keyUp = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_DOWN:
        input->keyboard.keyDown = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_SPACE:
        input->keyboard.keySpace = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_LSHIFT:
        input->keyboard.keyLeftShift = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_ESCAPE:
        input->keyboard.keyEscape = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_F1:
        input->keyboard.keyF1 = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_R:
        input->keyboard.keyR = (event.state == SDL_PRESSED);
        break;
    case SDL_SCANCODE_D:
        input->keyboard.keyD = (event.state == SDL_PRESSED);
        break;
    default:
        break;
    }
}

void handle_controller_button_event(SDL_ControllerButtonEvent &event, Input *input)
{
    switch(event.button)
    {
    case SDL_CONTROLLER_BUTTON_A:
        input->controller.aButton = (event.state == SDL_PRESSED);
        break;
    case SDL_CONTROLLER_BUTTON_B:
        input->controller.bButton = (event.state == SDL_PRESSED);
        break;
    case SDL_CONTROLLER_BUTTON_X:
        input->controller.xButton = (event.state == SDL_PRESSED);
        break;
    case SDL_CONTROLLER_BUTTON_Y:
        input->controller.yButton = (event.state == SDL_PRESSED);
        break;
    case SDL_CONTROLLER_BUTTON_START:
        input->controller.startButton = (event.state == SDL_PRESSED);
        break;
    default:
        break;
    }
}

void handle_controller_axis_event(SDL_ControllerAxisEvent &event, Input *input)
{
    switch(event.axis)
    {
    case SDL_CONTROLLER_AXIS_LEFTX:
        input->controller.xAxisLeft = event.value / 32767.0;
        break;
    case SDL_CONTROLLER_AXIS_LEFTY:
        input->controller.yAxisLeft = event.value / 32767.0;
        break;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        input->controller.triggerAxisRight = event.value / 32767.0;
        break;
    default:
        break;
    }
}

void handle_controller_device_event(SDL_ControllerDeviceEvent &event, Input *input)
{
    switch(event.type)
    {
    case SDL_CONTROLLERDEVICEADDED:
        if (event.which == 0)
        {
            if (!gameController)
                platform_init_controller();
        }
        break;
    case SDL_CONTROLLERDEVICEREMOVED:
        if (event.which == 0)
        {
            if (gameController)
                platform_deinit_controller();
        }

        break;
    default:
        break;
    }
}


void platform_poll_input(Input *input)
{
    bool controllerExists = platform_controller_exists();
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_QUIT:
            input->exit = true;
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            handle_keyboard_event(event.key, input);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            if (controllerExists)
                handle_controller_button_event(event.cbutton, input);
            break;
        case SDL_CONTROLLERAXISMOTION:
            if (controllerExists)
                handle_controller_axis_event(event.caxis, input);
            break;
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
        case SDL_CONTROLLERDEVICEREMAPPED:
            handle_controller_device_event(event.cdevice, input);
        default:
            break;
        }
    }
}

//haptic
void platform_controller_rumble(r32 strength, u32 lengthInMs)
{
    if (haptic)
    {
        SDL_HapticRumblePlay(haptic, strength, lengthInMs);
    }
}

//RESOURCES

Texture *platform_load_texture(const char* fname, bool srgb)
{
    //generate and set current image ID
    ILuint imgID;
    ilGenImages(1, &imgID);
    ilBindImage(imgID);

    //load
    ilLoadImage(fname);
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

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

    glTexImage2D(GL_TEXTURE_2D, 0, format, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    //delete file from memory
    ilDeleteImages(1, &imgID);

    return texture;
}
void platform_delete_texture(Texture *texture)
{
    glDeleteTextures(1, (GLuint*)texture);
    delete texture;
}
void platform_get_texture_width(Texture *texture, u32 *w)
{
    *w = texture->w;
    /*glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, (GLint*)w);
    glBindTexture(GL_TEXTURE_2D, 0);*/
}
void platform_get_texture_height(Texture *texture, u32 *h)
{
    *h = texture->h;
    /*glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint*)h);
    glBindTexture(GL_TEXTURE_2D, 0);*/
}

Texture *platform_create_empty_texture(s32 w, s32 h)
{
    Texture *texture = new Texture;
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

Texture *platform_create_tile_layer_texture(v3 *data, u32 w, u32 h)
{
    Texture *texture = new Texture;
    glGenTextures(1, (GLuint*)texture);
    texture->w = w;
    texture->h = h;

    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

void platform_update_tile_layer_texture(Texture *texture, v3 *data, u32 w, u32 h)
{
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Mesh platform_load_mesh(const char* fname)
{
    Mesh mesh;

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
                u32 nextIndex = mesh.positions.size();
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
                    for (u32 i = 0; i < mesh.positions.size(); i++)
                    {
                        if (mesh.positions.at(i) == tempPos && mesh.texCoords.at(i) == tempCoord && mesh.normals.at(i) == tempNormal)
                        {
                            exists = true;
                            tempIndices.push_back(i);
                        }
                    }

                    if (!exists)
                    {
                        mesh.positions.push_back(tempPos);
                        mesh.texCoords.push_back(tempCoord);
                        mesh.normals.push_back(tempNormal);

                        tempIndices.push_back(nextIndex++);
                    }
                    ++vertAmount;
                }
                //make triangle fan from polygon
                for (u32 i = 0; i < vertAmount - 2; i++)
                    mesh.tris.emplace_back(tempIndices.at(0), tempIndices.at(i + 1), tempIndices.at(i + 2));
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

    platform_calculate_tangent(&mesh);

    return mesh;
}

void platform_calculate_tangent(Mesh *mesh)
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

Sound *platform_load_sound(const char* fname)
{
    /*Mix_Chunk *chunk = Mix_LoadWAV(fname);
    if (chunk)
    {
        Sound *sound = new Sound;
        sound->chunk = chunk;
        return sound;
    }

    return NULL;*/
    FMOD_RESULT result;
    Sound *sound = new Sound;
    result = FMOD_System_CreateSound(soundSystem, fname, FMOD_CREATESAMPLE | FMOD_3D | FMOD_LOOP_NORMAL | FMOD_3D_LINEARSQUAREROLLOFF, NULL, &sound->sound);

    if (result == FMOD_OK)
        return sound;

    delete sound;
    return NULL;
}
void platform_delete_sound(Sound *sound)
{
    //Mix_FreeChunk(sound->chunk);
    //delete sound;
    if (sound)
    {
        FMOD_Sound_Release(sound->sound);
        delete sound;
    }
}

Music *platform_load_music(const char* fname)
{
    /*Music *music = new Music;
    music->music = Mix_LoadMUS(fname);
    if (!music->music)
        std::cout << "problem loading " << fname << ": " << Mix_GetError() << std::endl;
    return music;*/
    FMOD_RESULT result;
    Music *music = new Music;
    result = FMOD_System_CreateSound(soundSystem, fname, FMOD_CREATESTREAM | FMOD_2D | FMOD_LOOP_NORMAL, NULL, &music->sound);

    if (result == FMOD_OK)
        return music;

    delete music;
    return NULL;
}
void platform_delete_music(Music *music)
{
    /*Mix_FreeMusic(music->music);
    delete music;*/
    if (music)
    {
        FMOD_Sound_Release(music->sound);
        delete music;
    }
}

//INIT / QUIT

void init_SDL()
{
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_HAPTIC);

    //Use openGL 4.3 (for now)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
}

void init_FMOD()
{
    //init sound
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    FMOD_System_Create(&soundSystem);
    FMOD_System_Init(soundSystem, 256, FMOD_INIT_NORMAL, 0);

    //create channel groups
    FMOD_System_CreateChannelGroup(soundSystem, "SFXGroup", &sfxChannelGroup);
    FMOD_System_CreateChannelGroup(soundSystem, "WorldGroup", &worldChannelGroup);
    FMOD_System_CreateChannelGroup(soundSystem, "MenuGroup", &menuChannelGroup);
    FMOD_System_CreateChannelGroup(soundSystem, "MusicGroup", &musicChannelGroup);

    //set groups as children of master group
    FMOD_System_GetMasterChannelGroup(soundSystem, &masterChannelGroup);
    FMOD_ChannelGroup_AddGroup(sfxChannelGroup, worldChannelGroup, 1, 0);
    FMOD_ChannelGroup_AddGroup(sfxChannelGroup, menuChannelGroup, 1, 0);
    FMOD_ChannelGroup_AddGroup(masterChannelGroup, sfxChannelGroup, 1, 0);
    FMOD_ChannelGroup_AddGroup(masterChannelGroup, musicChannelGroup, 1, 0);

    //set listener attributes
    FMOD_VECTOR vector_forward = {0, 0, -1};
    FMOD_VECTOR vector_up = {0, -1, 0};
    FMOD_System_Set3DListenerAttributes(soundSystem, 0, NULL, NULL, &vector_forward, &vector_up);
}

void close_FMOD()
{
    //FMOD quit
    FMOD_System_Release(soundSystem);
    CoUninitialize();
}

void platform_init()
{
    //initialize sdl
    init_SDL();

    //create window and opengl context
    runnerWindow = SDL_CreateWindow("Parcool", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, runnerScreenWidth, runnerScreenHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    runnerGlContext = SDL_GL_CreateContext(runnerWindow);

    //initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();
    //set clear color to black
    glClearColor(0.f, 0.f, 0.f, 0.f);

    //initialize DevIL
    ilInit();

    init_renderer();

    init_FMOD();
}

void platform_quit()
{
    close_renderer();

    ilShutDown();

    //destroy opengl context and window
    SDL_GL_DeleteContext(runnerGlContext);
    SDL_DestroyWindow(runnerWindow);

    //quit sdl
    SDL_Quit();

    close_FMOD();
}

void platform_show_error(const char* error)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime error!", error, NULL);
}

r64 platform_time_in_ms()
{
    return SDL_GetPerformanceCounter() / (SDL_GetPerformanceFrequency() / 1000.0);
}
u64 platform_get_ticks()
{
    return SDL_GetPerformanceCounter();
}
void platform_disable_vsync()
{
    SDL_GL_SetSwapInterval(0);
}
void platform_enable_vsync()
{
    SDL_GL_SetSwapInterval(1);
}
u64 platform_ms_to_ticks(r64 t)
{
    r64 timeInSeconds = t / 1000.f;
    return std::round(timeInSeconds * SDL_GetPerformanceFrequency());
}
r64 platform_ticks_to_ms(u64 t)
{
    r64 ticksPerSecond = t * 1000;
    return ticksPerSecond / SDL_GetPerformanceFrequency();
}
/*
s32 main(s32 argc, char **argv)
{
    platform_init();

    std::cout << "Initialization complete!\n";

    platform_clear_buffer();
    platform_render_background(v4(2,0,0,1), v4(0,0,2,1));
    platform_swap_buffer();

    SDL_Delay(4000);
    platform_quit();

    std::cout << "Deinitialization complete!\n";

    return 0;
}*/
