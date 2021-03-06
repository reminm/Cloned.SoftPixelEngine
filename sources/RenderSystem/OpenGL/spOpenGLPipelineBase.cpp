/*
 * OpenGL base renderer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLPipelineBase.hpp"

#if defined(SP_COMPILE_WITH_OPENGL) || defined(SP_COMPILE_WITH_OPENGLES1) || defined(SP_COMPILE_WITH_OPENGLES2)

#include "SceneGraph/spSceneNode.hpp"
#include "Base/spSharedObjects.hpp"
#include "RenderSystem/spRenderContext.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)
#   include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"
#   include "RenderSystem/OpenGL/spOpenGLTexture.hpp"
#elif defined(SP_COMPILE_WITH_OPENGLES1) || defined(SP_COMPILE_WITH_OPENGLES2)
#   if defined(SP_COMPILE_WITH_OPENGLES1)
#       include "RenderSystem/OpenGLES/spOpenGLES1Texture.hpp"
#   endif
#   if defined(SP_COMPILE_WITH_OPENGLES2)
#       include "RenderSystem/OpenGLES/spOpenGLES2Texture.hpp"
#   endif
#   include "RenderSystem/OpenGLES/spOpenGLESFunctionsARB.hpp"
#endif

#include <boost/foreach.hpp>


namespace sp
{
namespace video
{


/*
 * ======= Internal members =======
 */

GLenum GLMeshBufferUsage[] =
{
    GL_STATIC_DRAW_ARB, GL_DYNAMIC_DRAW_ARB
};

s32 GLBlendingList[] =
{
    GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
};

s32 GLCompareList[] =
{
    GL_NEVER, GL_EQUAL, GL_NOTEQUAL, GL_LESS, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_ALWAYS,
};

#if defined(SP_COMPILE_WITH_OPENGL)

GLenum GLTexDimensions[] =
{
    GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP,
    GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_CUBE_MAP_ARRAY,
    GL_TEXTURE_RECTANGLE, GL_TEXTURE_BUFFER,
    GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D_ARRAY
};

GLenum GLPrimitiveModes[] =
{
    GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP, GL_TRIANGLES,
    GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS, GL_QUAD_STRIP, GL_POLYGON,
    GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP_ADJACENCY, GL_TRIANGLES_ADJACENCY, GL_PATCHES,
};

GLenum GLStencilOperationList[] =
{
    GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_INCR_WRAP, GL_DECR, GL_DECR_WRAP, GL_INVERT
};

#elif defined(SP_COMPILE_WITH_OPENGLES1) || defined(SP_COMPILE_WITH_OPENGLES2)

GLenum GLBasicDataTypes[] =
{
    GL_FLOAT, ~0, GL_BYTE, GL_SHORT, GL_FIXED, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, ~0
};

GLenum GLPrimitiveModes[] =
{
    GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP, GL_TRIANGLES,
    GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, ~0, ~0, ~0,
    ~0, ~0, ~0, ~0, ~0
};

#endif


/*
 * ======= GLBasePipeline class =======
 */

GLBasePipeline::GLBasePipeline() :
    RenderSystem        (RENDERER_DUMMY ),
    GLMajorVersion_     (1              ),
    GLMinorVersion_     (1              ),
    MultiTextureCount_  (0              ),
    isInvertScreen_     (false          )
{
}
GLBasePipeline::~GLBasePipeline()
{
}


/*
 * ======= Render system information =======
 */

io::stringc GLBasePipeline::getRenderer() const
{
    return io::stringc(glGetString(GL_RENDERER));
}
io::stringc GLBasePipeline::getVersion() const
{
    return io::stringc("OpenGL ") + glGetString(GL_VERSION);
}
io::stringc GLBasePipeline::getVendor() const
{
    return io::stringc(glGetString(GL_VENDOR));
}
io::stringc GLBasePipeline::getShaderVersion() const
{
    return "";
}

s32 GLBasePipeline::getMaxAnisotropicFilter() const
{
    GLfloat MaxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MaxAnisotropy);
    return static_cast<s32>(MaxAnisotropy);
}

dim::EMatrixCoordinateSystmes GLBasePipeline::getProjectionMatrixType() const
{
    return dim::MATRIX_RIGHTHANDED;
}


/*
 * ======= Video buffer control functions =======
 */

void GLBasePipeline::clearBuffers(const s32 ClearFlags)
{
    GLbitfield Mask = 0;
    
    if (ClearFlags & BUFFER_COLOR)
        Mask |= GL_COLOR_BUFFER_BIT;
    if (ClearFlags & BUFFER_DEPTH)
        Mask |= GL_DEPTH_BUFFER_BIT;
    if (ClearFlags & BUFFER_STENCIL)
        Mask |= GL_STENCIL_BUFFER_BIT;
    
    glClear(Mask);
}


/*
 * ======= Configuration functions =======
 */

void GLBasePipeline::setFrontFace(bool isFrontFace)
{
    isFrontFace_ = isFrontFace;
    
    if (!isInvertScreen_)
        glFrontFace(isFrontFace ? GL_CW : GL_CCW);
    else
        glFrontFace(isFrontFace ? GL_CCW : GL_CW);
}

void GLBasePipeline::setClearColor(const color &Color)
{
    /* Used for following macro */
    const bool UseAllRCs = true;
    
    /* Get color values as floats */
    f32 FltColor[4];
    Color.getFloatArray(FltColor);
    
    /* Set clear color for each render context */
    ChangeRenderStateForEachContext(
        glClearColor(FltColor[0], FltColor[1], FltColor[2], FltColor[3]);
    )
}
void GLBasePipeline::setColorMask(bool isRed, bool isGreen, bool isBlue, bool isAlpha)
{
    glColorMask(isRed, isGreen, isBlue, isAlpha);
}
void GLBasePipeline::setDepthMask(bool isDepth)
{
    glDepthMask(isDepth);
}

void GLBasePipeline::setDepthRange(f32 Near, f32 Far)
{
    RenderSystem::setDepthRange(Near, Far);
    glDepthRange(Near, Far);
}


/*
 * ======= Hardware mesh buffers =======
 */

void GLBasePipeline::createVertexBuffer(void* &BufferID)
{
    if (glGenBuffersARB)
    {
        BufferID = new u32;
        glGenBuffersARB(1, (u32*)BufferID);
    }
}
void GLBasePipeline::createIndexBuffer(void* &BufferID)
{
    createVertexBuffer(BufferID);
}

void GLBasePipeline::deleteVertexBuffer(void* &BufferID)
{
    if (glDeleteBuffersARB && BufferID)
    {
        glDeleteBuffersARB(1, (u32*)BufferID);
        delete static_cast<u32*>(BufferID);
        BufferID = 0;
    }
}
void GLBasePipeline::deleteIndexBuffer(void* &BufferID)
{
    deleteVertexBuffer(BufferID);
}

void GLBasePipeline::updateVertexBuffer(
    void* BufferID, const dim::UniversalBuffer &BufferData, const VertexFormat* Format, const EHWBufferUsage Usage)
{
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] && BufferID && BufferData.getCount())
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, *(u32*)BufferID);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, BufferData.getSize(), BufferData.getArray(), GLMeshBufferUsage[Usage]);
    }
}
void GLBasePipeline::updateIndexBuffer(
    void* BufferID, const dim::UniversalBuffer &BufferData, const IndexFormat* Format, const EHWBufferUsage Usage)
{
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] && BufferID && BufferData.getCount())
    {
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, *(u32*)BufferID);
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, BufferData.getSize(), BufferData.getArray(), GLMeshBufferUsage[Usage]);
    }
}

void GLBasePipeline::updateVertexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index)
{
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] && BufferID && BufferData.getCount())
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, *(u32*)BufferID);
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, BufferData.getStride() * Index, BufferData.getStride(), BufferData.getArray(Index, 0));
    }
}
void GLBasePipeline::updateIndexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index)
{
    if (RenderQuery_[RENDERQUERY_HARDWARE_MESHBUFFER] && BufferID && BufferData.getCount())
    {
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, *(u32*)BufferID);
        glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, BufferData.getStride() * Index, BufferData.getStride(), BufferData.getArray(Index, 0));
    }
}


/*
 * ======= Simple drawing functions =======
 */

void GLBasePipeline::setBlending(const EBlendingTypes SourceBlend, const EBlendingTypes DestBlend)
{
    glBlendFunc(GLBlendingList[SourceBlend], GLBlendingList[DestBlend]);
}

void GLBasePipeline::setClipping(bool Enable, const dim::point2di &Position, const dim::size2di &Size)
{
    if (Enable)
    {
        glEnable(GL_SCISSOR_TEST);
        
        if (isInvertScreen_)
        {
            glScissor(
                Position.X, Position.Y,
                Size.Width, Size.Height
            );
        }
        else
        {
            glScissor(
                Position.X, gSharedObjects.ScreenHeight - Position.Y - Size.Height,
                Size.Width, Size.Height
            );
        }
    }
    else
        glDisable(GL_SCISSOR_TEST);
}

void GLBasePipeline::setViewport(const dim::point2di &Position, const dim::size2di &Size)
{
    if (isInvertScreen_)
    {
        glViewport(
            Position.X, Position.Y,
            Size.Width, Size.Height
        );
    }
    else
    {
        glViewport(
            Position.X, gSharedObjects.ScreenHeight - Size.Height - Position.Y,
            Size.Width, Size.Height
        );
    }
}

void GLBasePipeline::setLineSize(s32 Size)
{
    glLineWidth(static_cast<GLfloat>(Size));
}


/*
 * ======= Texture loading and creation =======
 */

Texture* GLBasePipeline::createTexture(const STextureCreationFlags &CreationFlags)
{
    /* Create new OpenGL texture */
    Texture* NewTexture = 0;
    
    #if defined(SP_COMPILE_WITH_OPENGL)
    NewTexture = new OpenGLTexture(CreationFlags);
    #elif defined(SP_COMPILE_WITH_OPENGLES1) || defined(SP_COMPILE_WITH_OPENGLES2)
    #   if defined(SP_COMPILE_WITH_OPENGLES1)
    if (RendererType_ == RENDERER_OPENGLES1)
        NewTexture = new OpenGLES1Texture(CreationFlags);
    else
    #   endif
    #   if defined(SP_COMPILE_WITH_OPENGLES2)
        NewTexture = new OpenGLES2Texture(CreationFlags);
    #   endif
    #endif
    
    /* Check for POT texture size problem in OpenGL */
    ImageBuffer* ImgBuffer = NewTexture->getImageBuffer();
    
    if (!ImgBuffer->isSizePOT())
    {
        switch (ImgBuffer->getFormat())
        {
            case PIXELFORMAT_RGB:
                NewTexture->setFormat(PIXELFORMAT_RGBA);
                break;
            case PIXELFORMAT_BGR:
                NewTexture->setFormat(PIXELFORMAT_BGRA);
                break;
            default:
                break;
        }
    }
    
    #if defined(SP_COMPILE_WITH_OPENGLES1) || defined(SP_COMPILE_WITH_OPENGLES2)
    /* Validate texture size for OpenGL|ES */
    NewTexture->ensurePOT();
    #endif
    
    /* Add the texture to the texture list */
    TextureListSemaphore_.lock();
    TextureList_.push_back(NewTexture);
    TextureListSemaphore_.unlock();
    
    return NewTexture;
}


/*
 * ======= Matrix controll =======
 */

void GLBasePipeline::setWorldMatrix(const dim::matrix4f &Matrix)
{
    scene::spWorldMatrix = Matrix;
}

dim::matrix4f GLBasePipeline::getViewMatrix() const
{
    /* Invert view matrix to convert from left-handed to right-handed coordinate system */
    return dim::matrix4f(
        1,  0,  0,  0,
        0,  1,  0,  0,
        0,  0, -1,  0,
        0,  0,  0,  1
    ) * scene::spViewMatrix;
}


/*
 * ======= Static functions =======
 */

bool GLBasePipeline::printGLError(const io::stringc &Desc, bool PrintSuccess)
{
    const GLenum Err = glGetError();
    
    const io::stringc Msg = ">> (DEBUG) GL Command \"" + Desc + "\" ";
    
    switch (Err)
    {
        case GL_NO_ERROR:
            if (PrintSuccess)
                io::Log::message(Msg + "succeeded");
            return false;
            
        case GL_INVALID_ENUM:
            io::Log::error(Msg + "failed (GL_INVALID_ENUM)");
            break;
        case GL_INVALID_VALUE:
            io::Log::error(Msg + "failed (GL_INVALID_VALUE)");
            break;
        case GL_INVALID_OPERATION:
            io::Log::error(Msg + "failed (GL_INVALID_OPERATION)");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            io::Log::error(Msg + "failed (GL_INVALID_FRAMEBUFFER_OPERATION)");
            break;
        case GL_OUT_OF_MEMORY:
            io::Log::error(Msg + "failed (GL_OUT_OF_MEMORY)");
            break;
        case GL_STACK_UNDERFLOW:
            io::Log::error(Msg + "failed (GL_STACK_UNDERFLOW)");
            break;
        case GL_STACK_OVERFLOW:
            io::Log::error(Msg + "failed (GL_STACK_OVERFLOW)");
            break;
        default:
            io::Log::error(Msg + "failed (Unknown Error Code: " + io::stringc(Err) + ")");
            break;
    }
    
    return true;
}


/*
 * ======= Private: =======
 */

void GLBasePipeline::initExtensionInfo()
{
    if (ExtensionSupportMap_.empty())
        GLExtensionLoader::filterExtensionStrings(ExtensionSupportMap_);
}

bool GLBasePipeline::queryExtensionSupport(const io::stringc &TargetExtension) const
{
    return ExtensionSupportMap_.find(TargetExtension.str()) != ExtensionSupportMap_.end();
}

void GLBasePipeline::setInvertScreen(bool Enable)
{
    /* Invert screen mode */
    isInvertScreen_ = Enable;
    setFrontFace(isFrontFace_);
    
    /* Force the system to update render mode next time */
    setRenderMode(RENDERMODE_NONE);
}

GLenum GLBasePipeline::getGlTexDimension(const ETextureTypes Type)
{
    #if defined(SP_COMPILE_WITH_OPENGL)
    return GLTexDimensions[Type - TEXTURE_1D];
    #else
    //if (RendererType_ == RENDERER_OPENGLES1)
        return GL_TEXTURE_2D;
    //return GLTexDimensions[Type - TEXTURE_1D];
    #endif
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
