/*
 * Mesh modifier header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_MESH_MODIFIER_H__
#define __SP_SCENE_MESH_MODIFIER_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionMatrix4.hpp"


namespace sp
{
namespace video
{
    class MeshBuffer;
}
namespace scene
{


class Mesh;


//! Namespace for mesh buffer modification. This is only to modify vertex coordinates and delta connections.
namespace MeshModifier
{

//! Crop vertex interface class. This can be used as interface for the "cropTriangle" template.
class CropVertex
{
    
    public:
        
        CropVertex()
        {
        }
        virtual ~CropVertex()
        {
        }
        
        /* Operators */
        
        virtual CropVertex& operator = (const CropVertex &Other) = 0;
        virtual CropVertex& operator += (const CropVertex &Other) = 0;
        virtual CropVertex& operator -= (const CropVertex &Other) = 0;
        virtual CropVertex& operator *= (f32 Factor) = 0;
        virtual CropVertex& operator /= (f32 Factor) = 0;
        
        /* Functions */
        
        virtual dim::vector3df getCoord() const = 0;
        
};

//! Translates each vertex coordinate in the specified direction.
SP_EXPORT void meshTranslate(video::MeshBuffer &Surface, const dim::vector3df &Direction);
//! Transforms each vertex coordinate by multiplying it with the specified size.
SP_EXPORT void meshTransform(video::MeshBuffer &Surface, const dim::vector3df &Size);
//! Transforms each vertex coordinate by multiplying it with the specified transformation matrix.
SP_EXPORT void meshTransform(video::MeshBuffer &Surface, const dim::matrix4f &Matrix);
//! Turns each vertex coordinate by rotating them with the specified rotation vector. This function performs a YXZ matrix rotation.
SP_EXPORT void meshTurn(video::MeshBuffer &Surface, const dim::vector3df &Rotation);

//! Flips the mesh. i.e. each vertex coordinate will be inverted.
SP_EXPORT void meshFlip(video::MeshBuffer &Surface);
//! Flips each vertex coordiante only for the specified axles.
SP_EXPORT void meshFlip(video::MeshBuffer &Surface, bool isXAxis, bool isYAxis, bool isZAxis);

/**
Fits the mesh. i.e. each vertex's coordinate will be transformed to the specified box.
\param[in,out] Obj Specifies the Mesh object.
\param[in] Position Specifies the left-bottom-front position of the transformation box.
\param[in] Size Specifies the size of the transformation box.
*/
SP_EXPORT void meshFit(Mesh &Obj, const dim::vector3df &Position, const dim::vector3df &Size);

/**
Spherifies the whole mesh. Best primitive to get a nice sphere is the cube (but use more than 1 segment, e.g. 10 or more).
\param[in,out] Obj Specifies the Mesh object.
\param[in] Factor Transformation factor in the range from [-1.0 .. 1.0].
*/
SP_EXPORT void meshSpherify(Mesh &Obj, f32 Factor);

/**
Twists the whole mesh. Can be used to create an other kind of a spiral when using a cube
(with more than 1 segment, e.g. 10 or more).
\param[in,out] Obj Specifies the Mesh object.
\param[in] Rotation Rotation degree for the twist transformation.
*/
SP_EXPORT void meshTwist(Mesh &Obj, f32 Rotation);

/**
Crops a triangle and generates new triangles.
\paramt VtxT 
\since Version 3.2
\todo Unfinished
*/
template <class VtxT> bool cropTriangle(
    const VtxT &VertexA, const VtxT VertexB, const VtxT &VertexC, VtxT (&OutputVertices)[4],
    u32 &OutputCount, const dim::plane3df &Plane)
{
    return false; //!!!
}

} // /namespace MeshModifier


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
