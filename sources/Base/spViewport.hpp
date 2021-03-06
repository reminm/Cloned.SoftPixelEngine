/*
 * Viewport header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_VIEWPORT_H__
#define __SP_VIEWPORT_H__


#include "Base/spDimensionVector2D.hpp"
#include "Base/spDimensionSize2D.hpp"
#include "Base/spDimensionRect2D.hpp"


namespace sp
{
namespace video
{


/**
Viewport class which provides a 2D position and a 2D size.
\see dim::point2d
\see dim::size2d
\since Version 3.3
*/
class Viewport
{
    
    public:
        
        Viewport()
        {
        }
        Viewport(const dim::point2di &Position, const dim::size2di &Size) :
            Position_   (Position   ),
            Size_       (Size       )
        {
        }
        Viewport(const dim::rect2di &Viewport) :
            Position_   (Viewport.getLTPoint()  ),
            Size_       (Viewport.getSize()     )
        {
        }
        Viewport(const Viewport &Other) :
            Position_   (Other.Position_),
            Size_       (Other.Size_    )
        {
        }
        ~Viewport()
        {
        }
        
        /* === Inline functions === */
        
        inline void setPosition(const dim::point2di &Position)
        {
            Position_ = Position;
        }
        inline const dim::point2di& getPosition() const
        {
            return Position_;
        }
        
        inline void setSize(const dim::size2di &Size)
        {
            Size_ = Size;
        }
        inline const dim::size2di& getSize() const
        {
            return Size_;
        }
        
        //! Sets the viewport via the specified rectangle. Position is (Left, Top) and size is (Right - Left, Bottom - Top).
        inline void setRect(const dim::rect2di &Rect)
        {
            Position_.X     = Rect.Left;
            Position_.Y     = Rect.Top;
            Size_.Width     = Rect.getWidth();
            Size_.Height    = Rect.getHeight();
        }
        //! Returns the viewport as rectangle (X, Y, X + Width, Y + Height).
        inline dim::rect2di getRect() const
        {
            return dim::rect2di(
                Position_.X,
                Position_.Y,
                Position_.X + Size_.Width,
                Position_.Y + Size_.Height
            );
        }
        
        //! Returns the ratio between width and height (or rather 'width : height') or 0.0 if with and height is zero.
        inline f32 getRatioWH() const
        {
            return (Size_.Width | Size_.Height) != 0 ?
                static_cast<f32>(Size_.Width) / static_cast<f32>(Size_.Height) :
                0.0f;
        }
        
        //! Returns the ratio between width and height (or rather 'height : width') or 0.0 if with and height is zero.
        inline f32 getRatioHW() const
        {
            return (Size_.Width | Size_.Height) != 0 ?
                static_cast<f32>(Size_.Height) / static_cast<f32>(Size_.Width) :
                0.0f;
        }
        
    private:
        
        /* === Members === */
        
        dim::point2di Position_;
        dim::size2di Size_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
