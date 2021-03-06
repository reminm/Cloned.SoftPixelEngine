/*
 * Direct3D11 render system header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERSYSTEM_DIRECT3D11_H__
#define __SP_RENDERSYSTEM_DIRECT3D11_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D11)


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11Texture.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11Shader.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11DefaultShader.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11HardwareBuffer.hpp"
#include "Platform/spSoftPixelDeviceFlags.hpp"

#include <D3D11.h>
#include <D3DX11.h>


namespace sp
{
namespace video
{


#define D3D11_RENDER_SYS        (static_cast<Direct3D11RenderSystem*>(GlbRenderSys))
#define D3D11_DEVICE            (D3D11_RENDER_SYS->getD3DDevice())
#define D3D11_DEVICE_CONTEXT    (D3D11_RENDER_SYS->getD3DDeviceContext())


//! Direct3D11 render system. This renderer supports Direct3D 11.0.
class SP_EXPORT Direct3D11RenderSystem : public RenderSystem
{
    
    public:
        
        Direct3D11RenderSystem(const SRendererProfileFlags &ProfileFlags);
        ~Direct3D11RenderSystem();
        
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
        
        void printWarning();
        
        /* === Video buffer control functions === */
        
        void clearBuffers(const s32 ClearFlags = BUFFER_COLOR | BUFFER_DEPTH);
        
        /* === Configuration functions === */
        
        void setShadeMode(const EShadeModeTypes ShadeMode);
        
        void setClearColor(const color &Color);
        void setColorMask(bool isRed, bool isGreen, bool isBlue, bool isAlpha = true);
        
        void setDepthMask(bool isDepth);
        void setDepthRange(f32 Near, f32 Far);
        
        void setAntiAlias(bool isAntiAlias);
        
        /* === Rendering functions === */
        
        bool setupMaterialStates(const MaterialStates* Material, bool Forced);
        
        void bindTextureLayers(const TextureLayerListType &TexLayers);
        void unbindTextureLayers(const TextureLayerListType &TexLayers);
        
        void setupShaderClass(const scene::MaterialNode* Object, ShaderClass* ShaderObject);
        
        void updateMaterialStates(MaterialStates* Material, bool isClear = false);
        
        void updateLight(
            u32 LightID, const scene::ELightModels LightType, bool isVolumetric,
            const dim::vector3df &Direction, f32 SpotInnerConeAngle, f32 SpotOuterConeAngle,
            f32 AttenuationConstant, f32 AttenuationLinear, f32 AttenuationQuadratic
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
            bool UseAllRCs
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
        
        ShaderResource* createShaderResource();

        bool dispatch(ShaderClass* ShdClass, const dim::vector3d<u32> &GroupSize);
        
        /* === Simple drawing functions === */
        
        void beginDrawing2D();
        
        void setBlending(const EBlendingTypes SourceBlend, const EBlendingTypes DestBlend);
        void setClipping(bool Enable, const dim::point2di &Position, const dim::size2di &Dimension);
        void setViewport(const dim::point2di &Position, const dim::size2di &Dimension);
        
        bool setRenderTarget(Texture* Target);
        bool setRenderTarget(Texture* Target, ShaderClass* ShdClass);
        
        /* === Image drawing === */
        
        void draw2DImage(
            const Texture* Tex, const dim::point2di &Position, const color &Color = color(255)
        );
        
        void draw2DImage(
            const Texture* Tex, const dim::rect2di &Position,
            const dim::rect2df &Clipping = dim::rect2df(0.0f, 0.0f, 1.0f, 1.0f),
            const color &Color = color(255)
        );

        void drawFullscreenQuad();
        
        /* === Primitive drawing === */
        
        void draw2DPoint(const dim::point2di &Position, const color &Color = 255);
        
        void draw2DLine(
            const dim::point2di &PositionA, const dim::point2di &PositionB, const color &Color = 255
        );
        void draw2DLine(
            const dim::point2di &PositionA, const dim::point2di &PositionB, const color &ColorA, const color &ColorB
        );
        
        void draw2DRectangle(
            const dim::rect2di &Rect, const color &Color = 255, bool isSolid = true
        );
        void draw2DRectangle(
            const dim::rect2di &Rect, const color &lefttopColor, const color &righttopColor,
            const color &rightbottomColor, const color &leftbottomColor, bool isSolid = true
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
        void draw3DTriangle(
            Texture* Tex, const dim::triangle3df &Triangle, const color &Color = 255
        );
        
        /* === Texture loading and creating === */
        
        Texture* createTexture(const STextureCreationFlags &CreationFlags);
        
        /* === Matrix controll === */
        
        void updateModelviewMatrix();
        
        /* === Special renderer functions === */

        //! This is an internal D3D11 utility function and creates a shader resource view (SRV) for the specified hardware buffer.
        ID3D11ShaderResourceView* createShaderResourceView(
            ID3D11Buffer* HWBuffer, u32 ElementCount, const ERendererDataTypes DataType, u32 DataSize
        );
        //! This is an internal D3D11 utility function and creates an unordered access view (UAV) for the specified hardware buffer.
        ID3D11UnorderedAccessView* createUnorderedAccessView(
            ID3D11Buffer* HWBuffer, u32 ElementCount, const ERendererDataTypes DataType, u32 DataSize, s32 Flags
        );
        //! This is an internal D3D11 utility function and creates a CPU access buffer for the specified hardware buffer.
        ID3D11Buffer* createCPUAccessBuffer(ID3D11Buffer* GPUOutputBuffer);

        /* === Inline functions === */
        
        inline ID3D11Device* getD3DDevice() const
        {
            return D3DDevice_;
        }
        inline ID3D11DeviceContext* getD3DDeviceContext() const
        {
            return D3DDeviceContext_;
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
        
        friend class Direct3D11ShaderClass;
        friend class Direct3D11Shader;
        friend class Direct3D11Texture;
        friend class Direct3D11RenderContext;
        friend class Direct3D11ConstantBuffer;
        friend class Direct3D11ShaderResource;
        friend class D3D11HardwareBuffer;
        friend class VertexFormat;
        
        /* === Macros === */
        
        static const u32 MAX_SHADER_RESOURCES   = 64;
        static const u32 MAX_SAMPLER_STATES     = 16;
        static const u32 MAX_NUM_PRIM_VERTICES  = 8;
        
        /* === Structures === */
        
        struct SConstBuffer2DVS
        {
            dim::float4x4 ProjectionMatrix;
            dim::float4x4 WorldMatrix;
            dim::float4 TextureTransform;
            dim::float4 Position;
        };
        
        struct SConstBuffer2DPS
        {
            dim::float4 Color;
            s32 UseTexture;
            s32 Pad0[3];
        };
        
        struct SConstBufferPrimVS
        {
            struct SVertex
            {
                dim::float4 Position;
                dim::float4 Color;
            };
            
            dim::float4x4 WVPMatrix;
            SVertex Vertices[MAX_NUM_PRIM_VERTICES];
        };
        
        struct SQuad2DVertex
        {
            dim::point2df Position;
            dim::point2df TexCoord;
        };
        
        struct SConstBufferFont
        {
            dim::float4x4 WVPMatrix;
            dim::float4 Color;
            dim::float2 Position;
        };
        
        struct SDefaultFontShader
        {
            SDefaultFontShader();
            ~SDefaultFontShader();
            
            /* Functions */
            void setupText(const io::stringc &Text);
            
            /* Members */
            ShaderClass* ShdClass;
            ShaderResource* ResGlyphs;
            ShaderResource* ResText;
            ShaderResource* ResCharOffset;
        };
        
        struct SDxAdapter
        {
            io::stringw Description;
            u32 VendorId;
        };
        
        /* === Functions === */
        
        bool createDxDevice(const ED3DFeatureLevels SetupFeatureLevel);
        bool queryDxFactory();
        
        const D3D_FEATURE_LEVEL* getDxFeatureLevel(const ED3DFeatureLevels FeatureLevel) const;
        
        size_t enumAdapters();
        
        void createDefaultResources();
        void createRendererStates();
        void createStandardVertexBuffers();
        
        void updateShaderResources();
        
        void updateVertexInputLayout(VertexFormat* Format, bool isCreate);
        void addVertexInputLayoutAttribute(std::vector<D3D11_INPUT_ELEMENT_DESC>* InputDesc, const SVertexAttribute &Attrib);
        
        void drawTexturedFont(const Font* FontObj, const dim::point2di &Position, const io::stringc &Text, const color &Color);
        
        void createTexturedFontVertexBuffer(dim::UniversalBuffer &VertexBuffer, VertexFormatUniversal &VertFormat);
        void setupTexturedFontGlyph(void* &RawVertexData, const SFontGlyph &Glyph, const dim::rect2df &Mapping);
        
        void setupShaderResourceView(u32 Index, ID3D11ShaderResourceView* ResourceView);
        void setupSamplerState(u32 Index, ID3D11SamplerState* SamplerState);
        
        void generateMIPsForPrevRT(Texture* NewTarget);
        
        void drawPrimVertices(
            const D3D11_PRIMITIVE_TOPOLOGY Topology, u32 NumVertices,
            const dim::vector4df* Coords, const dim::vector4df* Colors
        );
        
        static DXGI_FORMAT getDxFormat(const ERendererDataTypes DataType, s32 Size, bool IsNormalize = false);

        /* === Private members === */
        
        /* Direct3D members */
        
        IDXGIFactory* Factory_;
        
        ID3D11Device* D3DDevice_;
        ID3D11DeviceContext* D3DDeviceContext_;
        
        ID3D11RenderTargetView* RenderTargetView_;
        ID3D11RenderTargetView* OrigRenderTargetView_;
        
        ID3D11DepthStencilView* DepthStencilView_;
        ID3D11DepthStencilView* OrigDepthStencilView_;
        
        ID3D11RasterizerState* RasterizerState_;
        ID3D11DepthStencilState* DepthStencilState_;
        ID3D11BlendState* BlendState_;
        
        /* Descriptions */
        
        //D3D11_INPUT_ELEMENT_DESC* VertexLayout2D_;
        
        D3D11_RASTERIZER_DESC RasterizerDesc_;
        D3D11_DEPTH_STENCIL_DESC DepthStencilDesc_;
        D3D11_BLEND_DESC BlendDesc_;
        
        /* Containers */
        
        u32 NumBoundedResources_;
        ID3D11ShaderResourceView* ShaderResourceViewList_[MAX_SHADER_RESOURCES];
        
        u32 NumBoundedSamplers_;
        ID3D11SamplerState* SamplerStateList_[MAX_SAMPLER_STATES];
        
        D3D11VertexBuffer* Quad2DVertexBuffer_; //!< Quad 2D vertex buffer (for 2D image and rectangle drawing).
        
        /* Other members */
        
        bool isMultiSampling_;
        f32 FinalClearColor_[4];
        video::color ClearColor_;
        
        D3D_FEATURE_LEVEL FeatureLevel_;
        
        std::vector<SDxAdapter> Adapters_;
        SDxAdapter* ActiveAdapter_;
        
        /* Default basic shader objects */
        
        D3D11DefaultShader DefaultShader_;
        bool UseDefaultBasicShader_;
        
        ShaderClass* DefaultBasicShader2D_;     //!< Default drawing 2D shader (for 2D images and rectangles).
        ShaderClass* DefaultPrimShader_;        //!< Default primitive drawing shader (for lines and points).
        
        VertexFormatUniversal* Draw2DVertFmt_;
        
        SConstBuffer2DVS ConstBuffer2DVS_;
        SConstBuffer2DPS ConstBuffer2DPS_;
        SConstBufferPrimVS ConstBufferPrimVS_;

        //SDefaultFontShader DefaultFontShader_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
