/*
 * Deferred renderer shader classbacks header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DEFERRED_RENDERER_SHADER_CALLBACKS_H__
#define __SP_DEFERRED_RENDERER_SHADER_CALLBACKS_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/spShaderConfigTypes.hpp"

#include <vector>


namespace sp
{
namespace video
{


/*
 * All deferred renderer (DfRn) shader callbacks
 */

SHADER_OBJECT_CALLBACK(DfRnGBufferObjectShaderCallback      );
SHADER_OBJECT_CALLBACK(DfRnGBufferObjectShaderCallbackCB    );

SHADER_SURFACE_CALLBACK(DfRnGBufferSurfaceShaderCallback    );
SHADER_SURFACE_CALLBACK(DfRnGBufferSurfaceShaderCallbackCB  );

SHADER_OBJECT_CALLBACK(DfRnDeferredShaderCallback           );
SHADER_OBJECT_CALLBACK(DfRnDeferredShaderCallbackCB         );

SHADER_OBJECT_CALLBACK(DfRnShadowShaderCallback             );

SHADER_OBJECT_CALLBACK(DfRnDebugVPLShaderCallback           );


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================