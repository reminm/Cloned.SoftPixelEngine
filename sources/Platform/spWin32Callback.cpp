/*
 * Win32 main callback file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Platform/spSoftPixelDeviceWindows.hpp"

#if defined(SP_PLATFORM_WINDOWS)


#include "Base/spSharedObjects.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Base/spInputOutputControl.hpp"
#include "RenderSystem/spDesktopRenderContext.hpp"

// Windowsx.h is required for the "GET_X_LPARAM" and "GET_Y_LPARAM" macros.
#include <Windowsx.h>

#ifndef WM_XBUTTONDOWN
#   define WM_XBUTTONDOWN   0x020B
#endif
#ifndef WM_XBUTTONUP
#   define WM_XBUTTONUP     0x020C
#endif
#ifndef WM_XBUTTONDBLCLK
#   define WM_XBUTTONDBLCLK 0x020D
#endif


namespace sp
{


extern SoftPixelDevice* GlbEngineDev;
extern io::InputControl* GlbInputCtrl;

/*
 * Global functions
 */

static void RecordKeyEvent(u32 KeyCode, bool IsDown)
{
    if (IsDown)
        io::InputControl::keyEventDown(KeyCode);
    else
        io::InputControl::keyEventUp(KeyCode);
}

static void RecordKey(WPARAM wParam, LPARAM lParam, bool IsDown)
{
    /* Extract key code */
    const u32 KeyCode = static_cast<s32>(wParam);
    const u32 OEMCode = static_cast<s32>(lParam & (0xFF << 16)) >> 16;
    
    /* Check for extended keys */
    switch (KeyCode)
    {
        case io::KEY_SHIFT:
        {
            if (OEMCode == 0x36)
                RecordKeyEvent(io::KEY_RSHIFT, IsDown);
            else if (OEMCode == 0x2A)
                RecordKeyEvent(io::KEY_LSHIFT, IsDown);
        }
        break;
        
        case io::KEY_CONTROL:
        {
            if ( ( ( static_cast<u32>(lParam) >> 24 ) & 0x1 ) != 0 )
                RecordKeyEvent(io::KEY_RCONTROL, IsDown);
            else
                RecordKeyEvent(io::KEY_LCONTROL, IsDown);
        }
        break;
    }
    
    /* Record base key event */
    RecordKeyEvent(KeyCode, IsDown);
}

static s32 MouseCaptureCount = 0;

static void CaptureMouseButton(s32 MouseButton, HWND hWnd)
{
    if (!__isMouseKey[MouseButton])
        __hitMouseKey[MouseButton] = true;
    __isMouseKey[MouseButton] = true;
    
    if (++MouseCaptureCount == 1)
        SetCapture(hWnd);
}

static void ReleaseMouseButton(s32 MouseButton)
{
    __wasMouseKey[MouseButton] = true;
    __isMouseKey[MouseButton] = false;
    
    if (--MouseCaptureCount == 0)
        ReleaseCapture();
    
    if (MouseCaptureCount < 0)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ReleaseMouseButton", "\"MouseCaptureCount\" should never be less zero");
        #endif
        MouseCaptureCount = 0;
    }
}

SP_EXPORT LRESULT CALLBACK SpWin32Callback(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    static s32 LastCursorX, LastCursorY;
    
    switch (Message)
    {
        /* === Keyboard events === */
        
        case WM_KEYDOWN:
        {
            RecordKey(wParam, lParam, true);
        }
        return 0;
        
        case WM_KEYUP:
        {
            RecordKey(wParam, lParam, false);
        }
        return 0;
        
        case WM_CHAR:
        {
            if (GlbInputCtrl->getWordInput())
            {
                switch (wParam)
                {
                    case 0x08: // Backspace
                        if (GlbInputCtrl->getEnteredWord().size() > 0)
                            GlbInputCtrl->getEnteredWord().resize(GlbInputCtrl->getEnteredWord().size() - 1);
                        break;
                        
                    /*case 0x09: // Tabulator
                        GlbInputCtrl->getEnteredWord() += io::stringc('\t');
                        break;*/
                        
                    default:
                        if (wParam >= 32 && wParam < 256)
                            GlbInputCtrl->getEnteredWord() += io::stringc((TCHAR)wParam);
                        break;
                }
            }
        }
        return 0;
        
        /* === Left mouse button events === */
        
        case WM_LBUTTONDOWN:
        {
            CaptureMouseButton(io::MOUSE_LEFT, hWnd);
        }
        return 0;
        
        case WM_LBUTTONUP:
        {
            ReleaseMouseButton(io::MOUSE_LEFT);
        }
        return 0;
        
        case WM_LBUTTONDBLCLK:
        {
            CaptureMouseButton(io::MOUSE_LEFT, hWnd);
            __dbclkMouseKey[io::MOUSE_LEFT] = true;
        }
        return 0;
        
        /* === Right mouse button events === */
        
        case WM_RBUTTONDOWN:
        {
            CaptureMouseButton(io::MOUSE_RIGHT, hWnd);
        }
        return 0;
        
        case WM_RBUTTONUP:
        {
            ReleaseMouseButton(io::MOUSE_RIGHT);
        }
        return 0;
        
        case WM_RBUTTONDBLCLK:
        {
            CaptureMouseButton(io::MOUSE_RIGHT, hWnd);
            __dbclkMouseKey[io::MOUSE_RIGHT] = true;
        }
        return 0;
        
        /* === Middle mouse button events === */
        
        case WM_MBUTTONDOWN:
        {
            CaptureMouseButton(io::MOUSE_MIDDLE, hWnd);
        }
        return 0;
        
        case WM_MBUTTONUP:
        {
            ReleaseMouseButton(io::MOUSE_MIDDLE);
        }
        return 0;
        
        case WM_MBUTTONDBLCLK:
        {
            CaptureMouseButton(io::MOUSE_MIDDLE, hWnd);
            __dbclkMouseKey[io::MOUSE_MIDDLE] = true;
        }
        return 0;
        
        /* === X mouse button events === */
        
        case WM_XBUTTONDOWN:
        {
            const s16 XButton = HIWORD(wParam);
            
            if (XButton == XBUTTON1)
                CaptureMouseButton(io::MOUSE_XBUTTON1, hWnd);
            else if (XButton == XBUTTON2)
                CaptureMouseButton(io::MOUSE_XBUTTON2, hWnd);
        }
        break;
        
        case WM_XBUTTONUP:
        {
            const s16 XButton = HIWORD(wParam);
            
            if (XButton == XBUTTON1)
                ReleaseMouseButton(io::MOUSE_XBUTTON1);
            else if (XButton == XBUTTON2)
                ReleaseMouseButton(io::MOUSE_XBUTTON2);
        }
        break;
        
        case WM_XBUTTONDBLCLK:
        {
            const s16 XButton = HIWORD(wParam);
            
            if (XButton == 0x0001)
            {
                CaptureMouseButton(io::MOUSE_XBUTTON1, hWnd);
                __dbclkMouseKey[io::MOUSE_XBUTTON1] = true;
            }
            else if (XButton == 0x0002)
            {
                CaptureMouseButton(io::MOUSE_XBUTTON2, hWnd);
                __dbclkMouseKey[io::MOUSE_XBUTTON2] = true;
            }
        }
        break;
        
        /* === Mouse motion events === */
        
        // THIS IS UNUSED
        /*case WM_MOUSEMOVE:
        {
            gSharedObjects.CursorSpeedX = GET_X_LPARAM(lParam) - LastCursorX;
            gSharedObjects.CursorSpeedY = GET_Y_LPARAM(lParam) - LastCursorY;
            LastCursorX = GET_X_LPARAM(lParam);
            LastCursorY = GET_Y_LPARAM(lParam);
        }
        return 0;*/
        
        case WM_MOUSEWHEEL:
        {
            gSharedObjects.MouseWheel = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        }
        return 0;
        
        /* === General window events === */
        
        case WM_CLOSE:
        {
            static_cast<SoftPixelDeviceWin32*>(GlbEngineDev)->isWindowOpened_ = false;
            PostQuitMessage(0);
        }
        return 0;
        
        case WM_KILLFOCUS:
        {
            io::InputControl::clearInput();
            ReleaseCapture();
        }
        return 0;
        
        /*case WM_ACTIVATEAPP:
        {
            if ((BOOL)lParam != FALSE)
            {
                if (GlbRenderCtx->getFullscreen())
                    GlbRenderCtx->setFullscreen(false);
            }
        }
        return 0;*/
        
        case WM_DROPFILES:
        {
            video::RenderContext* Context = (video::RenderContext*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

            if (Context)
            {
                TCHAR Filename[MAX_PATH] = { 0 };
                HDROP hDrop = (HDROP)wParam;

                /* Query number of droped files */
                u32 NumFiles = DragQueryFile(hDrop, 0xFFFFFFFF, 0, 0);

                /* Register filename from each droped file */
                for (u32 i = 0; i < NumFiles; ++i)
                {
                    if (DragQueryFile(hDrop, i, Filename, MAX_PATH))
                        Context->registerDropedFile(Filename, i, NumFiles);
                }

                DragFinish(hDrop);
            } 
        }
        return 0;
        
        case WM_SIZE:
        {
            video::RenderContext* Context = (video::RenderContext*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            
            if (Context)
            {
                /* Resize context with the new window client area size */
                WORD Width  = LOWORD(lParam);
                WORD Height = HIWORD(lParam);
                
                video::DesktopRenderContext::EnableWindowResize_ = false;
                {
                    Context->setResolution(dim::size2di(Width, Height));
                }
                video::DesktopRenderContext::EnableWindowResize_ = true;
            }
        }
        return 0;
        
        case WM_SYSCOMMAND:
        {
            // Prevent for a powersave mode of monitor or the screensaver
            if ((wParam & 0xFFF0) == SC_SCREENSAVE || (wParam & 0xFFF0) == SC_MONITORPOWER)
                return 0;
        }
        break;
    }
    
    return DefWindowProc(hWnd, Message, wParam, lParam);
}


} // /namespace sp


#endif



// ================================================================================
