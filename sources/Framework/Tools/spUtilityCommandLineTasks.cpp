/*
 * Command line tasks file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spUtilityCommandLineTasks.hpp"

#ifdef SP_COMPILE_WITH_COMMANDLINE


#include "Base/spSharedObjects.hpp"
#include "Base/spInputOutputControl.hpp"
#include "Base/spInputOutputOSInformator.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spRenderContext.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Platform/spSoftPixelDevice.hpp"
#include "Framework/Network/spNetworkSystem.hpp"
#include "Framework/Tools/spUtilityCommandLine.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern video::RenderSystem* __spVideoDriver;
extern video::RenderContext* __spRenderContext;
extern scene::SceneGraph* __spSceneManager;
extern io::InputControl* __spInputControl;
extern io::OSInformator* __spOSInformator;

namespace tool
{


namespace CommandLineTasks
{

static const c8* ERR_NO_SCENE_GRAPH = "No active scene graph";
static const c8* ERR_NO_CAMERA      = "No active camera";
static const c8* ERR_NO_CONTEXT     = "No active render context";

SP_EXPORT void cmdWireframe(CommandLineUI &Cmd, scene::SceneGraph* Graph, const video::EWireframeTypes Type)
{
    if (Graph)
    {
        Graph->setWireframe(Type);
        Cmd.confirm("Switched Wireframe Mode");
    }
    else
        Cmd.error(ERR_NO_SCENE_GRAPH);
}

SP_EXPORT void cmdFullscreen(CommandLineUI &Cmd)
{
    video::RenderContext* ActiveContext = video::RenderContext::getActiveRenderContext();
    
    if (ActiveContext)
    {
        ActiveContext->setFullscreen(!ActiveContext->getFullscreen());
        Cmd.confirm("Switched Fullscreen Mode");
    }
    else
        Cmd.error(ERR_NO_CONTEXT);
}

SP_EXPORT void cmdView(CommandLineUI &Cmd, scene::Camera* Cam)
{
    if (Cam)
    {
        const dim::vector3df Pos(Cam->getPosition(true));
        const dim::vector3df Rot(Cam->getRotation(true));
        
        Cmd.confirm(
            "Camera Position = ( " +
            io::stringc::numberFloat(Pos.X, 1, true) + " , " +
            io::stringc::numberFloat(Pos.Y, 1, true) + " , " +
            io::stringc::numberFloat(Pos.Z, 1, true) + " )"
        );
        Cmd.confirm(
            "Camera Rotation = ( " +
            io::stringc::numberFloat(Rot.X, 1, true) + " , " +
            io::stringc::numberFloat(Rot.Y, 1, true) + " , " +
            io::stringc::numberFloat(Rot.Z, 1, true) + " )"
        );
    }
    else
        Cmd.error(ERR_NO_CAMERA);
}

SP_EXPORT void cmdVsync(CommandLineUI &Cmd)
{
    video::RenderContext* ActiveContext = video::RenderContext::getActiveRenderContext();
    
    if (ActiveContext)
    {
        ActiveContext->setVsync(!ActiveContext->getVsync());
        Cmd.confirm(
            io::stringc("Vertical Synchronisation ") + (ActiveContext->getVsync() ? "Enabled" : "Disabled")
        );
    }
    else
        Cmd.error(ERR_NO_CONTEXT);
}

SP_EXPORT void cmdScene(CommandLineUI &Cmd, scene::SceneGraph* Graph)
{
    if (Graph)
    {
        Cmd.confirm("Objects:      " + io::stringc(Graph->getSceneObjectsCount())    );
        Cmd.confirm("Scene Nodes:  " + io::stringc(Graph->getNodeList().size())      );
        Cmd.confirm("Cameras:      " + io::stringc(Graph->getCameraList().size())    );
        Cmd.confirm("Lights:       " + io::stringc(Graph->getLightList().size())     );
        Cmd.confirm("Billboards:   " + io::stringc(Graph->getBillboardList().size()) );
        Cmd.confirm("Terrains:     " + io::stringc(Graph->getTerrainList().size())   );
        Cmd.confirm("Meshes:       " + io::stringc(Graph->getMeshList().size())      );
        Cmd.confirm("Mesh Buffers: " + io::stringc(Graph->getSceneMeshBufferCount()) );
        Cmd.confirm("Vertices:     " + io::stringc(Graph->getSceneVertexCount())     );
        Cmd.confirm("Triangles:    " + io::stringc(Graph->getSceneTriangleCount())   );
    }
    else
        Cmd.error(ERR_NO_SCENE_GRAPH);
}

SP_EXPORT void cmdHardware(CommandLineUI &Cmd)
{
    Cmd.confirm(__spOSInformator->getOSVersion());
    
    const u32 CPUCores = __spOSInformator->getProcessorCount();
    const u32 CPUSpeed = __spOSInformator->getProcessorSpeed();
    
    if (CPUCores > 1)
        Cmd.confirm("CPU @ " + io::stringc(CPUCores) + " x " + io::stringc(CPUSpeed) + " MHz");
    else
        Cmd.confirm("CPU @ " + io::stringc(CPUSpeed) + " MHz");
    
    Cmd.confirm(__spVideoDriver->getRenderer() + ": " + __spVideoDriver->getVendor());
}

SP_EXPORT void cmdNetwork(CommandLineUI &Cmd)
{
    #ifdef SP_COMPILE_WITH_NETWORKSYSTEM
    
    Cmd.confirm("Network Adapters:");
    
    std::list<network::SNetworkAdapter> Adapters = network::NetworkSystem::getNetworkAdapters();
    
    if (!Adapters.empty())
    {
        foreach (const network::SNetworkAdapter &Adapter, Adapters)
        {
            Cmd.confirm(
                "  " + (Adapter.Description.size() ? Adapter.Description : io::stringc("Unnamed")) +
                " [ IP = " + Adapter.IPAddress + ", Mask = " + Adapter.IPMask + (Adapter.Enabled ? ", Enabled ]" : ", Disabled ]")
            );
        }
    }
    else
        Cmd.confirm("No network adapters available");
    
    if (!__spDevice->getNetworkSystemList().empty())
    {
        foreach (network::NetworkSystem* NetSys, __spDevice->getNetworkSystemList())
        {
            Cmd.confirm(NetSys->getDescription() + ":");
            Cmd.confirm("  Server IP: " + (NetSys->isServer() ? "127.0.0.1" :  NetSys->getServer()->getAddress().getIPAddressName()));
            Cmd.confirm("  Clients: " + io::stringc(NetSys->getClientList().size()));
            Cmd.confirm("  Connected: " + io::stringc(NetSys->isConnected() ? "Yes" : "No"));
            Cmd.confirm("  Running Session: " + io::stringc(NetSys->isSessionRunning() ? "Yes" : "No"));
        }
    }
    else
        Cmd.confirm("No network system has been created");
    
    #else
    
    Cmd.error("Engine was not compiled with network system");
    
    #endif
}

SP_EXPORT void cmdResolution(CommandLineUI &Cmd, const io::stringc &Command)
{
    /* Get active render context */
    video::RenderContext* ActiveContext = video::RenderContext::getActiveRenderContext();
    
    if (!ActiveContext)
    {
        Cmd.error(ERR_NO_CONTEXT);
        return;
    }
    
    /* Get resolution string */
    io::stringc ResParam;
    if (!Cmd.getCmdParam(Command, ResParam))
        return;
    
    /* Find separator character 'x' */
    const s32 SeparatorPos = ResParam.find("x");
    
    if (SeparatorPos == -1)
    {
        Cmd.error("Missing 'x' separator character in resolution parameter (e.g. \"800x600\")");
        return;
    }
    
    /* Extract resolution values */
    const dim::size2di Resolution(
        ResParam.left(SeparatorPos).val<s32>(),
        ResParam.right(ResParam.size() - SeparatorPos - 1).val<s32>()
    );
    
    /* Change new resolution */
    ActiveContext->setResolution(Resolution);
    
    /* Update scene resolution */
    Cmd.setRect(dim::rect2di(0, 0, Resolution.Width, Resolution.Height/2));
    
    if (__spSceneManager && __spSceneManager->getActiveCamera())
        __spSceneManager->getActiveCamera()->setViewport(dim::rect2di(0, 0, Resolution.Width, Resolution.Height));
    
    /* Print confirmation message */
    Cmd.confirm("Changed Resolution: ( " + io::stringc(Resolution.Width) + " x " + io::stringc(Resolution.Height) + " )");
}

} // /namespace CommandLineTasks


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
