/*
 * OpenCL program header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENCL_PROGRAM_H__
#define __SP_OPENCL_PROGRAM_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENCL)


#include "Base/spInputOutputString.hpp"
#include "Framework/OpenCL/spOpenCLCoreHeader.hpp"
#include "Framework/OpenCL/spOpenCLBuffer.hpp"

#include <map>


namespace sp
{
namespace video
{


/**
\since Version 3.2
\ingroup group_gpgpu
*/
class SP_EXPORT OpenCLProgram
{
    
    public:
        
        OpenCLProgram(const io::stringc &SourceString, const io::stringc &CompilationOptions = "");
        ~OpenCLProgram();
        
        /* === Functions === */
        
        bool compile(const io::stringc &SourceString, const io::stringc &CompilationOptions = "");
        
        bool addKernel(const io::stringc &EntryPoint);
        
        /**
        Runs respectively executes the specified OpenCL kernel.
        \param EntryPoint: Specifies the kernel entry point, i.e. the main-function's name.
        \param Dimensions: Specifies the count of dimensions for the working groups. Must be 1, 2 or 3.
        \param GlobalWorkSizes: Specifies a pointer to an array of 'Dimensions' elements describing the global work sizes.
        \param LocalWorkSizes: Specifies a pointer to an array of 'Dimensions' elements describing the local work sizes.
        \return True on success otherwise false.
        */
        bool run(
            const io::stringc &EntryPoint, s32 Dimensions, const size_t* GlobalWorkSizes, const size_t* LocalWorkSizes
        );
        
        bool setParameter(const io::stringc &EntryPoint, u32 Index, const void* Buffer, u32 Size);
        bool setParameter(const io::stringc &EntryPoint, u32 Index, OpenCLBuffer* Buffer);
        
        /* === Inline functions === */
        
        inline bool run(
            const io::stringc &KernelEntryPoint, size_t GlobalWorkSize, size_t LocalWorkSize)
        {
            return run(KernelEntryPoint, 1, &GlobalWorkSize, &LocalWorkSize);
        }
        
        template <typename T> bool setParameter(const io::stringc &EntryPoint, u32 Index, const T &Value)
        {
            return setParameter(EntryPoint, Index, &Value, sizeof(T));
        }
        
        //! Returns true if this OpenCL program was build successfully.
        inline bool valid() const
        {
            return BuildSuccessful_;
        }
        
    private:
        
        /* === Functions === */
        
        void releaseProgram();
        
        bool buildProgram(const io::stringc &SourceString, const io::stringc &CompilationOptions);
        
        /* === Members === */
        
        cl_program clProgram_;
        std::map<std::string, cl_kernel> clKernelList_;
        
        bool BuildSuccessful_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
