/*
 * Axis-Aligned BoundingBox header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_AABB_H__
#define __SP_DIMENSION_AABB_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionLine3D.hpp"
#include "Base/spDimensionQuadrangle3D.hpp"
#include <algorithm>

namespace sp
{
namespace dim
{


template <typename T> class plane3d;


#define DefineConstOperator(OP)                                 \
    aabbox3d<T> operator OP (const aabbox3d<T> &other) const    \
    {                                                           \
        return aabbox3d<T>(Min OP other.Min, Max OP other.Max); \
    }

#define DefineOperator(OP)                              \
    aabbox3d<T>& operator OP (const aabbox3d<T> &other) \
    {                                                   \
        Min OP other.Min;                               \
        Max OP other.Max;                               \
        return *this;                                   \
    }


/**
Axis-aligned bounding-box (AABB) class (Min, Max).
\ingroup group_data_types
*/
template <typename T> class aabbox3d
{
    
    public:
        
        aabbox3d()
        {
        }
        aabbox3d(const vector3d<T> &MinEdge, const vector3d<T> &MaxEdge) :
            Min(MinEdge),
            Max(MaxEdge)
        {
        }
        aabbox3d(const line3d<T> &Line) :
            Min(Line.Start  ),
            Max(Line.End    )
        {
        }
        aabbox3d(const aabbox3d<T> &Other) :
            Min(Other.Min),
            Max(Other.Max)
        {
        }
        ~aabbox3d()
        {
        }
        
        /* === Operators === */
        
        DefineConstOperator(*)
        DefineConstOperator(/)
        DefineConstOperator(+)
        DefineConstOperator(-)
        
        DefineOperator(*=)
        DefineOperator(/=)
        DefineOperator(+=)
        DefineOperator(-=)
        
        /* === Extra functions === */
        
        //! Returns the center of the bounding box.
        inline vector3d<T> getCenter() const
        {
            return (Min + Max) / 2;
        }
        
        //! Returns the size of the bounding box.
        inline vector3d<T> getSize() const
        {
            return Max - Min;
        }
        
        //! Returns the volume of this bounding box.
        inline T getVolume() const
        {
            return getSize().getVolume();
        }
        
        //! Returns true if the two AABBs intersects.
        inline bool checkBoxBoxIntersection(const aabbox3d<T> &other) const
        {
            return Min <= other.Max && Max >= other.Min;
        }
        
        //! Returns true if the given AABB is completely inside this box.
        inline bool isBoxInside(const aabbox3d<T> &other) const
        {
            return Min >= other.Min && Max <= other.Max;
        }
        
        //! Returns true if the specified point is inside the box.
        inline bool isPointInside(const vector3d<T> &Point) const
        {
            return (
                Point.X >= Min.X && Point.Y >= Min.Y && Point.Z >= Min.Z &&
                Point.X <= Max.X && Point.Y <= Max.Y && Point.Z <= Max.Z
            );
        }
        
        //! Returns true if minimum and maximum are equal.
        inline bool empty(f32 Tolerance = math::ROUNDING_ERROR) const
        {
            return Min.equal(Max, Tolerance);
        }
        
        //! Returns true if this is a valid box.
        inline bool valid() const
        {
            return
                Max.X >= Min.X &&
                Max.Y >= Min.Y &&
                Max.Z >= Min.Z;
        }
        
        //! Repairs the bounding box if any component of "Min" is greater then the corresponding component of "Max".
        inline aabbox3d<T>& repair()
        {
            if (Min.X > Max.X) std::swap(Min.X, Max.X);
            if (Min.Y > Max.Y) std::swap(Min.Y, Max.Y);
            if (Min.Z > Max.Z) std::swap(Min.Z, Max.Z);
            return *this;
        }
        
        //! Inserts a point to the bounding box. This can result in that the box becomes larger.
        inline void insertPoint(const vector3d<T> &Point)
        {
            if (Point.X > Max.X) Max.X = Point.X;
            if (Point.Y > Max.Y) Max.Y = Point.Y;
            if (Point.Z > Max.Z) Max.Z = Point.Z;
            
            if (Point.X < Min.X) Min.X = Point.X;
            if (Point.Y < Min.Y) Min.Y = Point.Y;
            if (Point.Z < Min.Z) Min.Z = Point.Z;
        }
        
        //! Inserts the min- and max points of the given box to this bounding box. This can result in that the box becomes larger.
        inline void insertBox(const aabbox3d<T> &Other)
        {
            insertPoint(Other.Min);
            insertPoint(Other.Max);
        }
        
        //! Returns the bounding box's volume.
        inline T getBoxVolume() const
        {
            return
                (Max.X - Min.X).getAbs() *
                (Max.Y - Min.Y).getAbs() *
                (Max.Z - Min.Z).getAbs();
        }
        
        //! Returns a copy of this box multiplied by the specified size.
        inline aabbox3d<T> getScaled(const vector3d<T> &Size) const
        {
            return aabbox3d<T>(Min * Size, Max * Size);
        }
        
        /**
        Returns the maximal absolute vector of min and max components.
        \code
        aabbox3df(vector3df(-4, -2, -3), vector3df(1, 5, 6)).getMaxRadius() == vector3df(4, 5, 6);
        \endcode
        */
        inline vector3d<T> getMaxRadius() const
        {
            const vector3d<T> AbsMin(Min.getAbs());
            const vector3d<T> AbsMax(Max.getAbs());
            return vector3d<T>(
                math::Max(AbsMin.X, AbsMax.X),
                math::Max(AbsMin.Y, AbsMax.Y),
                math::Max(AbsMin.Z, AbsMax.Z)
            );
        }
        
        plane3d<T> getLeftPlane() const;
        plane3d<T> getRightPlane() const;
        plane3d<T> getTopPlane() const;
        plane3d<T> getBottomPlane() const;
        plane3d<T> getFrontPlane() const;
        plane3d<T> getBackPlane() const;
        
        plane3d<T> getPlane(u32 Index) const
        {
            switch (Index)
            {
                case 0: return getLeftPlane();
                case 1: return getRightPlane();
                case 2: return getTopPlane();
                case 3: return getBottomPlane();
                case 4: return getFrontPlane();
                case 5: return getBackPlane();
            }
            return plane3d<T>();
        }
        
        inline quadrangle3d<T> getLeftQuad() const
        {
            return quadrangle3d<T>(
                vector3d<T>(Min.X, Min.Y, Max.Z),
                vector3d<T>(Min.X, Max.Y, Max.Z),
                vector3d<T>(Min.X, Max.Y, Min.Z),
                vector3d<T>(Min.X, Min.Y, Min.Z)
            );
        }
        inline quadrangle3d<T> getRightQuad() const
        {
            return quadrangle3d<T>(
                vector3d<T>(Max.X, Min.Y, Min.Z),
                vector3d<T>(Max.X, Max.Y, Min.Z),
                vector3d<T>(Max.X, Max.Y, Max.Z),
                vector3d<T>(Max.X, Min.Y, Max.Z)
            );
        }
        inline quadrangle3d<T> getTopQuad() const
        {
            return quadrangle3d<T>(
                vector3d<T>(Min.X, Max.Y, Min.Z),
                vector3d<T>(Min.X, Max.Y, Max.Z),
                vector3d<T>(Max.X, Max.Y, Max.Z),
                vector3d<T>(Max.X, Max.Y, Min.Z)
            );
        }
        inline quadrangle3d<T> getBottomQuad() const
        {
            return quadrangle3d<T>(
                vector3d<T>(Min.X, Min.Y, Max.Z),
                vector3d<T>(Min.X, Min.Y, Min.Z),
                vector3d<T>(Max.X, Min.Y, Min.Z),
                vector3d<T>(Max.X, Min.Y, Max.Z)
            );
        }
        inline quadrangle3d<T> getFrontQuad() const
        {
            return quadrangle3d<T>(
                vector3d<T>(Min.X, Min.Y, Min.Z),
                vector3d<T>(Min.X, Max.Y, Min.Z),
                vector3d<T>(Max.X, Max.Y, Min.Z),
                vector3d<T>(Max.X, Min.Y, Min.Z)
            );
        }
        inline quadrangle3d<T> getBackQuad() const
        {
            return quadrangle3d<T>(
                vector3d<T>(Max.X, Min.Y, Max.Z),
                vector3d<T>(Max.X, Max.Y, Max.Z),
                vector3d<T>(Min.X, Max.Y, Max.Z),
                vector3d<T>(Min.X, Min.Y, Max.Z)
            );
        }
        
        /**
        Returns a corner of this box.
        \param Index: Specifies the corners index. This must be a value in the range [0 .. 7].
        \return 3D vector containing the corner coordinate.
        */
        vector3d<T> getCorner(u32 Index) const
        {
            switch (Index)
            {
                case 0: return vector3d<T>(Min.X, Min.Y, Min.Z);
                case 1: return vector3d<T>(Max.X, Min.Y, Min.Z);
                case 2: return vector3d<T>(Min.X, Max.Y, Min.Z);
                case 3: return vector3d<T>(Max.X, Max.Y, Min.Z);
                case 4: return vector3d<T>(Min.X, Min.Y, Max.Z);
                case 5: return vector3d<T>(Max.X, Min.Y, Max.Z);
                case 6: return vector3d<T>(Min.X, Max.Y, Max.Z);
                case 7: return vector3d<T>(Max.X, Max.Y, Max.Z);
            }
            return T(0);
        }
        
        line3d<T> getEdge(u32 Index) const
        {
            switch (Index)
            {
                case  0: return line3d<T>(vector3d<T>(Min.X, Min.Y, Min.Z), vector3d<T>(Max.X, Min.Y, Min.Z));
                case  1: return line3d<T>(vector3d<T>(Min.X, Max.Y, Min.Z), vector3d<T>(Max.X, Max.Y, Min.Z));
                case  2: return line3d<T>(vector3d<T>(Min.X, Max.Y, Max.Z), vector3d<T>(Max.X, Max.Y, Max.Z));
                case  3: return line3d<T>(vector3d<T>(Min.X, Min.Y, Max.Z), vector3d<T>(Max.X, Min.Y, Max.Z));
                
                case  4: return line3d<T>(vector3d<T>(Min.X, Min.Y, Min.Z), vector3d<T>(Min.X, Max.Y, Min.Z));
                case  5: return line3d<T>(vector3d<T>(Max.X, Min.Y, Min.Z), vector3d<T>(Max.X, Max.Y, Min.Z));
                case  6: return line3d<T>(vector3d<T>(Max.X, Min.Y, Max.Z), vector3d<T>(Max.X, Max.Y, Max.Z));
                case  7: return line3d<T>(vector3d<T>(Min.X, Min.Y, Max.Z), vector3d<T>(Min.X, Max.Y, Max.Z));
                
                case  8: return line3d<T>(vector3d<T>(Min.X, Min.Y, Min.Z), vector3d<T>(Min.X, Min.Y, Max.Z));
                case  9: return line3d<T>(vector3d<T>(Max.X, Min.Y, Min.Z), vector3d<T>(Max.X, Min.Y, Max.Z));
                case 10: return line3d<T>(vector3d<T>(Max.X, Max.Y, Min.Z), vector3d<T>(Max.X, Max.Y, Max.Z));
                case 11: return line3d<T>(vector3d<T>(Min.X, Max.Y, Min.Z), vector3d<T>(Min.X, Max.Y, Max.Z));
            }
            return line3d<T>();
        }
        
        //! Returns the closest point on the plane.
        vector3d<T> getClosestPoint(const plane3d<T> &Plane) const;
        
        /* === Members === */
        
        vector3d<T> Min; //!< Left-lower-front corner of the bounding box.
        vector3d<T> Max; //!< Right-upper-back corner of the bounding box.
        
        /* === Macros === */
        
        static const aabbox3d<T> OMEGA;     //!< Largest invalid bounding box (999999 to -999999).
        static const aabbox3d<T> IDENTITY;  //!< Identity bounding box (-1 to 1).
        static const aabbox3d<T> CUBE;      //!< Default cube with a size of 1 (-0.5 to 0.5).
        
};

template <typename T> const aabbox3d<T> aabbox3d<T>::OMEGA(math::OMEGA, -math::OMEGA);
template <typename T> const aabbox3d<T> aabbox3d<T>::IDENTITY(T(-1), T(1));
template <typename T> const aabbox3d<T> aabbox3d<T>::CUBE(T(-0.5), T(0.5));

typedef aabbox3d<s32> aabbox3di;
typedef aabbox3d<f32> aabbox3df;


#undef DefineConstOperator
#undef DefineOperator


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
