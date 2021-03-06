/*
 * OpenGL query file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/OpenGL/spOpenGLQuery.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "RenderSystem/OpenGL/spOpenGLFunctionsARB.hpp"


namespace sp
{
namespace video
{


static const GLenum GLQueryTargets[] =
{
    GL_SAMPLES_PASSED, GL_ANY_SAMPLES_PASSED, GL_PRIMITIVES_GENERATED, GL_TIME_ELAPSED
};

OpenGLQuery::OpenGLQuery(const EQueryTypes Type) :
    Query       (Type   ),
    QueryObject_(0      ),
    Protocol_   (0      )
{
    glGenQueries(1, &QueryObject_);
}
OpenGLQuery::~OpenGLQuery()
{
    if (QueryObject_)
        glDeleteQueries(1, &QueryObject_);
}

void OpenGLQuery::begin()
{
    if (QueryObject_ && Protocol_ == 0)
    {
        glBeginQuery(GLQueryTargets[getType()], QueryObject_);
        Protocol_ = 1;
    }
}

void OpenGLQuery::end()
{
    if (QueryObject_ && Protocol_ == 1)
    {
        glEndQuery(GLQueryTargets[getType()]);
        Protocol_ = 2;
    }
}

u64 OpenGLQuery::result() const
{
    if (!QueryObject_ || Protocol_ != 2)
        return 0;
    
    /* Flush command buffer */
    glFlush();
    
    /* Wait until the result is available */
    GLint Available = 0;
    
    do
    {
        glGetQueryObjectiv(QueryObject_, GL_QUERY_RESULT_AVAILABLE, &Available);
    }
    while (!Available);
    
    /* Return result from the GL query object */
    GLuint Result = 0;
    glGetQueryObjectuiv(QueryObject_, GL_QUERY_RESULT, &Result);
    
    Protocol_ = 0;
    
    return Result;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
