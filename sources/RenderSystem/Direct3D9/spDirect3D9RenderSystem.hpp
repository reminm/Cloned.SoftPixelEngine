/*
 * Direct3D9 render system header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERSYSTEM_DIRECT3D9_H__
#define __SP_RENDERSYSTEM_DIRECT3D9_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/Direct3D9/spDirect3D9Texture.hpp"
#include "RenderSystem/Direct3D9/spDirect3D9Shader.hpp"

#include <d3d9.h>
#include <d3dx9.h>
#if defined(SP_COMPILER_VC)
#   include <dxerr.h>
#else
#   include <dxerr9.h>
#endif


namespace sp
{
namespace video
{


extern const io::stringc d3dDllFileName;

#define D3D9_RENDER_SYS (static_cast<Direct3D9RenderSystem*>(GlbRenderSys))
#define D3D9_DEVICE     (D3D9_RENDER_SYS->getD3DDevice())


//! Direct3D9 render system. This renderer supports Direct3D 9.0c.
class SP_EXPORT Direct3D9RenderSystem : public RenderSystem
{
    
    public:
        
        Direct3D9RenderSystem();
        ~Direct3D9RenderSystem();
        
        /* === Initialization functions === */
        
        void setupConfiguration();
        
        /* === Renderer information === */
        
        io::stringc getRenderer()           const;
        io::stringc getVersion()            const;
        io::stringc getVendor()             const;
        io::stringc getShaderVersion()      const;
        
        s32 getMultitexCount()              const;
        s32 getMaxAnisotropicFilter()       const;
        s32 getMaxLightCount()              const;
        
        bool queryVideoSupport(const EVideoFeatureSupport Query) const;
        
        /* === Video buffer control functions === */
        
        void clearBuffers(const s32 ClearFlags = BUFFER_COLOR | BUFFER_DEPTH);
        
        /* === Configuration functions === */
        
        void setShadeMode(const EShadeModeTypes ShadeMode);
        
        void setClearColor(const color &Color);
        void setColorMask(bool isRed, bool isGreen, bool isBlue, bool isAlpha = true);
        void setDepthMask(bool isDepth);
        void setAntiAlias(bool isAntiAlias);
        
        void setDepthRange(f32 Near, f32 Far);
        void setDepthClip(bool Enable);
        
        /* === Stencil buffer === */
        
        void setStencilMask(u32 BitMask);
        void setStencilMethod(const ESizeComparisionTypes Method, s32 Reference = 0, u32 BitMask = ~0);
        void setStencilOperation(const EStencilOperations FailOp, const EStencilOperations ZFailOp, const EStencilOperations ZPassOp);
        void setClearStencil(s32 Stencil);
        
        /* === Rendering functions === */
        
        bool setupMaterialStates(const MaterialStates* Material, bool Forced = false);
        
        void setupTextureLayer(
            u8 LayerIndex, const dim::matrix4f &TexMatrix, const ETextureEnvTypes EnvType,
            const EMappingGenTypes GenType, s32 MappingCoordsFlags
        );
        
        void updateLight(
            u32 LightID, const scene::ELightModels LightType, bool IsVolumetric,
            const dim::vector3df &Direction, const scene::SLightCone &SpotCone, const scene::SLightAttenuation &Attn
        );
        
        /* === Hardware mesh buffers === */
        
        void createVertexBuffer(void* &BufferID);
        void createIndexBuffer(void* &BufferID);
        
        void deleteVertexBuffer(void* &BufferID);
        void deleteIndexBuffer(void* &BufferID);
        
        void updateVertexBuffer(
            void* BufferID, const dim::UniversalBuffer &BufferData, const VertexFormat* Format, const EHWBufferUsage Usage
        );
        void updateIndexBuffer(
            void* BufferID, const dim::UniversalBuffer &BufferData, const IndexFormat* Format, const EHWBufferUsage Usage
        );
        
        void updateVertexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index);
        void updateIndexBufferElement(void* BufferID, const dim::UniversalBuffer &BufferData, u32 Index);
        
        bool bindMeshBuffer(const MeshBuffer* Buffer);
        void unbindMeshBuffer();
        void drawMeshBufferPart(const MeshBuffer* Buffer, u32 StartOffset, u32 NumVertices);
        void drawMeshBuffer(const MeshBuffer* MeshBuffer);
        
        /* === Queries === */
        
        Query* createQuery(const EQueryTypes Type);
        void deleteQuery(Query* &QueryObj);

        /* === Render states === */
        
        void setRenderState(const video::ERenderStates Type, s32 State);
        s32 getRenderState(const video::ERenderStates Type) const;
        
        /* === Lighting === */
        
        void addDynamicLightSource(
            u32 LightID, scene::ELightModels Type,
            video::color &Diffuse, video::color &Ambient, video::color &Specular,
            f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic
        );
        
        void setLightStatus(u32 LightID, bool Enable, bool UseAllRCs = false);
        
        void setLightColor(
            u32 LightID,
            const video::color &Diffuse, const video::color &Ambient, const video::color &Specular,
            bool UseAllRCs = false
        );
        
        /* === Fog effect === */
        
        void setFog(const EFogTypes Type);
        void setFogColor(const video::color &Color);
        void setFogRange(f32 Range, f32 NearPlane = 1.0f, f32 FarPlane = 1000.0f, const EFogModes Mode = FOG_PALE);
        
        /* === Clipping planes === */
        
        void setClipPlane(u32 Index, const dim::plane3df &Plane, bool Enable);
        
        /* === Shader programs === */
        
        ShaderClass* createShaderClass(const VertexFormat* VertexInputLayout = 0);
        
        Shader* createShader(
            ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
            const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = "", u32 Flags = 0
        );
        
        Shader* createCgShader(
            ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
            const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = "",
            const c8** CompilerOptions = 0
        );
        
        void unbindShaders();
        
        /* === Simple drawing functions === */
        
        void endSceneRendering();
        
        void beginDrawing2D();
        
        void setBlending(const EBlendingTypes SourceBlend, const EBlendingTypes DestBlend);
        void setClipping(bool Enable, const dim::point2di &Position, const dim::size2di &Dimension);
        void setViewport(const dim::point2di &Position, const dim::size2di &Dimension);
        
        bool setRenderTarget(Texture* Target);
        
        void setPointSize(s32 Size = 1);
        
        /* === Image drawing === */
        
        void draw2DImage(
            const Texture* Tex, const dim::point2di &Position, const color &Color = color(255)
        );
        
        void draw2DImage(
            const Texture* Tex, const dim::rect2di &Position,
            const dim::rect2df &Clipping = dim::rect2df(0.0f, 0.0f, 1.0f, 1.0f),
            const color &Color = color(255)
        );
        
        void draw2DImage(
            const Texture* Tex, const dim::point2di &Position, f32 Rotation, f32 Radius, const color &Color = color(255)
        );
        
        void draw2DImage(
            const Texture* Tex,
            const dim::point2di &lefttopPosition,
            const dim::point2di &righttopPosition,
            const dim::point2di &rightbottomPosition,
            const dim::point2di &leftbottomPosition,
            const dim::point2df &lefttopClipping = dim::point2df(0.0f, 0.0f),
            const dim::point2df &righttopClipping = dim::point2df(1.0f, 0.0f),
            const dim::point2df &rightbottomClipping = dim::point2df(1.0f, 1.0f),
            const dim::point2df &leftbottomClipping = dim::point2df(0.0f, 1.0f),
            const color &lefttopColor = color(255),
            const color &righttopColor = color(255),
            const color &rightbottomColor = color(255),
            const color &leftbottomColor = color(255)
        );
        
        /* === Primitive drawing === */
        
        color getPixelColor(const dim::point2di &Position) const;
        f32 getPixelDepth(const dim::point2di &Position) const;
        
        void draw2DPoint(const dim::point2di &Position, const color &Color = 255);
        
        void draw2DLine(
            const dim::point2di &PositionA, const dim::point2di &PositionB, const color &Color = 255
        );
        void draw2DLine(
            const dim::point2di &PositionA, const dim::point2di &PositionB, const color &ColorA, const color &ColorB
        );
        void draw2DLine(
            const dim::point2di &PositionA, const dim::point2di &PositionB, const color &Color, s32 DotLength
        );
        
        void draw2DRectangle(
            const dim::rect2di &Rect, const color &Color = 255, bool isSolid = true
        );
        void draw2DRectangle(
            const dim::rect2di &Rect, const color &lefttopColor, const color &righttopColor,
            const color &rightbottomColor, const color &leftbottomColor, bool isSolid = true
        );
        
        /* === Extra drawing functions === */
        
        void draw2DPolygon(
            const ERenderPrimitives Type, const scene::SPrimitiveVertex2D* VerticesList, u32 Count
        );
        void draw2DPolygonImage(
            const ERenderPrimitives Type, Texture* Tex, const scene::SPrimitiveVertex2D* VerticesList, u32 Count
        );
        
        /* === 3D drawing functions === */
        
        void draw3DPoint(
            const dim::vector3df &Position, const color &Color = 255
        );
        void draw3DLine(
            const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &Color = 255
        );
        void draw3DLine(
            const dim::vector3df &PositionA, const dim::vector3df &PositionB, const color &ColorA, const color &ColorB
        );
        void draw3DEllipse(
            const dim::vector3df &Position, const dim::vector3df &Rotation = 0.0f,
            const dim::size2df &Radius = 0.5f, const color &Color = 255
        );
        void draw3DTriangle(
            Texture* hTexture, const dim::triangle3df &Triangle, const color &Color = 255
        );
        
        /* === Texture loading and creating === */
        
        Texture* createTexture(const STextureCreationFlags &CreationFlags);
        
        Texture* createScreenShot(const dim::point2di &Position = 0, dim::size2di Size = 0);
        void createScreenShot(Texture* Tex, const dim::point2di &Position = 0);
        
        void deleteTexture(Texture* &Tex);

        /* === Font loading and text drawing === */
        
        Font* createBitmapFont(const io::stringc &FontName = "", s32 FontSize = 0, s32 Flags = 0);
        
        /* === Matrix controll === */
        
        void updateModelviewMatrix();
        
        void setProjectionMatrix(const dim::matrix4f &Matrix);
        void setViewMatrix      (const dim::matrix4f &Matrix);
        void setWorldMatrix     (const dim::matrix4f &Matrix);
        void setTextureMatrix   (const dim::matrix4f &Matrix, u8 TextureLayer = 0);
        
        /* === Special renderer functions === */
        
        void releaseAllResources();
        void recreateAllResources();

        inline IDirect3DDevice9* getD3DDevice() const
        {
            return D3DDevice_;
        }
        
        template <class T> static inline void releaseObject(T* &Object)
        {
            if (Object)
            {
                Object->Release();
                Object = 0;
            }
        }
        
    private:
        
        friend class Direct3D9Texture;
        friend class Direct3D9RenderContext;

        /* === Structures === */

        struct SResourceManagement
        {
            SResourceManagement();
            ~SResourceManagement();

            /* Functions */
            void releaseAll();

            /* Templates */
            template <typename K, typename V> inline void add(std::map<K, V> &List, const K &Key, const V &Value)
            {
                List[Key] = Value;
            }
            template <typename K, typename V> inline void remove(std::map<K, V> &List, const K &Key)
            {
                typename std::map<K, V>::iterator it = List.find(Key);

                if (it != List.end())
                    List.erase(it);
            }
            template <typename K, typename V> inline void release(std::map<K, V> &List)
            {
                for (typename std::map<K, V>::iterator it = List.begin(); it != List.end(); ++it)
                    Direct3D9RenderSystem::releaseObject(it->second);
            }
            template <typename K, typename V> inline bool contains(std::map<K, V> &List, const K &Key)
            {
                return List.find(Key) != List.end();
            }

            /* Members */
            std::map<void*, IDirect3DVertexBuffer9*> VertexBuffers;
            std::map<void*, IDirect3DIndexBuffer9*> IndexBuffers;
            std::map<Texture*, IDirect3DBaseTexture9*> TextureResources;
            std::map<Query*, IDirect3DQuery9*> Queries;
        };
        
        /* === Functions === */
        
        void updatePrimitiveList(const SPrimitiveVertex* VertexList, u32 Size);
        void updatePrimitiveListFlexible(const SPrimitiveVertex* VertexList, u32 Count);
        
        bool setRenderTargetSurface(const s32 Index, Texture* Target);
        
        void releaseFontObject(Font* FontObj);
        
        void drawTexturedFont(const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color);
        void drawBitmapFont(const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color);
        
        void createTexturedFontVertexBuffer(dim::UniversalBuffer &VertexBuffer, VertexFormatUniversal &VertFormat);
        void setupTexturedFontGlyph(void* &RawVertexData, const SFontGlyph &Glyph, const dim::rect2df &Mapping);
        
        void bindDrawingColor(const video::color &Color);
        void unbindDrawingColor();
        
        /* === Inline functions === */
        
        inline D3DCOLORVALUE getD3DColor(const video::color &Color)
        {
            D3DCOLORVALUE Result;
            Color.getFloatArray(&Result.r);
            return Result;
        }
        
        /* === Members === */
        
        IDirect3D9* D3DInstance_;
        IDirect3DDevice9* D3DDevice_;
        
        IDirect3DVertexBuffer9* D3DDefVertexBuffer_;
        IDirect3DVertexBuffer9* D3DDefFlexibleVertexBuffer_;
        
        D3DCAPS9 DevCaps_;
        D3DLIGHT9 D3DActiveLight_;
        
        IDirect3DSurface9* PrevRenderTargetSurface_;
        
        IDirect3DTexture9* CurD3DTexture_;
        IDirect3DCubeTexture9* CurD3DCubeTexture_;
        IDirect3DVolumeTexture9* CurD3DVolumeTexture_;
        
        video::color ClearColor_, ClearColorMask_;
        DWORD ClearStencil_;

        SResourceManagement ResMngr_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
