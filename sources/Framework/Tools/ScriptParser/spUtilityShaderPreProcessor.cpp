/*
 * Shader pre-processor file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/ScriptParser/spUtilityShaderPreProcessor.hpp"

#ifdef SP_COMPILE_WITH_SHADER_PREPROCESSOR


#include "Base/spInputOutputFileSystem.hpp"
#include "Base/spBaseExceptions.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace tool
{


ShaderPreProcessor::ShaderPreProcessor() :
    Tkn_        (0                      ),
    Options_    (0                      ),
    OutString_  (0                      ),
    IndentMask_ (io::stringc::space(4)  )
{
}
ShaderPreProcessor::~ShaderPreProcessor()
{
}

bool ShaderPreProcessor::preProcessShader(
    const io::stringc &InputShaderCode, io::stringc &OutputShaderCode,
    const video::EShaderTypes ShaderType, const video::EShaderVersions ShaderVersion,
    const io::stringc &EntryPoint, u32 Options)
{
    /* Get options for easy access */
    const bool IsGLSL = (ShaderVersion >= video::GLSL_VERSION_1_20 && ShaderVersion <= video::GLSL_VERSION_4_30);
    const bool IsHLSL = (ShaderVersion >= video::HLSL_VERTEX_1_0 && ShaderVersion <= video::HLSL_DOMAIN_5_0);
    
    bool PrevTokenNL = false;
    Options_ = Options;
    
    /* Validate parameters */
    io::Log::message("Preprocess shader code [" + io::stringc(IsGLSL ? "GLSL" : "HLSL") + "]");
    io::Log::ScopedTab Unused;
    
    if (EntryPoint.empty())
        return exitWithError("Invalid entry point");
    
    /* Parse tokens from input shader code */
    TokenIt_ = Parser_.parseTokens(InputShaderCode.c_str(), COMMENTSTYLE_ANSI_C);
    
    if (!TokenIt_)
        return exitWithError("Invalid token iterator");
    
    /* Validate brackets */
    if (!validateBrackets())
        return false;
    
    OutString_ = &OutputShaderCode;
    
    /* Reconstruct shader code for output */
    while (nextToken(false))
    {
        /* Apply processing options */
        if ((Options_ & SHADER_PREPROCESS_SKIP_BLANKS) != 0)
        {
            if (type() == TOKEN_NEWLINE)
            {
                if (PrevTokenNL)
                    continue;
                PrevTokenNL = true;
            }
            else if (PrevTokenNL && Tkn_->isWhiteSpace())
                continue;
            else
                PrevTokenNL = false;
        }
        
        if ((Options_ & SHADER_PREPROCESS_NO_TABS) != 0 && type() == TOKEN_TAB)
        {
            append("    ");
            continue;
        }
        
        if (IsGLSL)
        {
            /* Solve macros directly */
            if (type() == TOKEN_NAME)
            {
                if ((Options_ & SHADER_PREPROCESS_SOLVE_MACROS) != 0)
                    solveMacrosGLSL();
                if (Tkn_->Str == EntryPoint)
                {
                    if (processEntryPointGLSL())
                        continue;
                }
            }
            
            /* Solve HLSL attributes (e.g. [loop], [unroll] etc.) */
            if (type() == TOKEN_SQUARED_BRACKET_LEFT)
            {
                TokenIt_->prev();
                const bool IsPrevName = (TokenIt_->getPrevToken(true, true).Type == TOKEN_NAME);
                TokenIt_->next();
                
                if (!IsPrevName && solveAttributesGLSL())
                    continue;
            }
        }
        
        /* Append current token string */
        append();
    }
    
    return true;
}


/*
 * ======= Private: ========
 */

bool ShaderPreProcessor::exitWithError(const io::stringc &Message, const SToken* InvalidToken)
{
    io::Log::error(Message + " at " + InvalidToken->getRowColumnString());
    return false;
}

bool ShaderPreProcessor::exitWithError(const io::stringc &Message, bool AppendTokenPos)
{
    if (AppendTokenPos)
        return exitWithError(Message, Tkn_);
    io::Log::error(Message);
    return false;
}

bool ShaderPreProcessor::validateBrackets()
{
    const SToken* InvalidToken = 0;
    
    switch (TokenIt_->validateBrackets(InvalidToken))
    {
        case VALIDATION_ERROR_UNEXPECTED:
            return exitWithError("Unexpected bracket token", InvalidToken);
        case VALIDATION_ERROR_UNCLOSED:
            return exitWithError("Unclosed brackets");
        default:
            break;
    }
    
    return true;
}

bool ShaderPreProcessor::nextToken(bool IgnoreWhiteSpaces)
{
    /* Get next token */
    Tkn_ = &TokenIt_->getNextToken(IgnoreWhiteSpaces);
    return !Tkn_->eof() && Tkn_->valid();
}

bool ShaderPreProcessor::nextToken(const ETokenTypes NextTokenType)
{
    /* Find next token with specified type */
    Tkn_ = &TokenIt_->getNextToken(NextTokenType);
    return !Tkn_->eof() && Tkn_->valid();
}

bool ShaderPreProcessor::nextTokenCheck(const ETokenTypes CheckTokenType)
{
    /* Get next token and check if the type matches the specified type */
    if (!nextToken())
        return false;
    return type() == CheckTokenType;
}

void ShaderPreProcessor::append()
{
    Tkn_->appendString(*OutString_);
}

void ShaderPreProcessor::append(const io::stringc &Str)
{
    *OutString_ += (Indent_ + Str);
}

void ShaderPreProcessor::pushIndent()
{
    Indent_ += IndentMask_;
}

void ShaderPreProcessor::popIndent()
{
    if (Indent_.size() > IndentMask_.size())
        Indent_.resize(Indent_.size() - IndentMask_.size());
    else
        Indent_.clear();
}

void ShaderPreProcessor::solveMacrosGLSL()
{
    io::stringc& Name = Tkn_->Str;
    
    /* Check for vector and matrix macros */
    static const SDataTypeConversion ConversionTypes[] =
    {
        { "float", 0, "vec", "mat" },
        { "double", 0, "dvec", "dmat" },
        { "half", "float", "vec", "mat" },
        { "int", 0, "ivec", "imat" },
        { "uint", 0, "uvec", "umat" },
        { "bool", 0, "bvec", "bmat" },
        { 0, 0, "", "" }
    };
    
    const SDataTypeConversion* Entry = ConversionTypes;
    
    while (Entry->HLSLDataType != 0)
    {
        if (Name.leftEqual(Entry->HLSLDataType) && solveMacroVectorGLSL(Name, *Entry))
            return;
        ++Entry;
    }
    
    /* Check for other macros */
    struct SDefMacro
    {
        const c8* From;
        const c8* To;
    };
    
    static const SDefMacro DefMacros[] =
    {
        { "groupshared", "shared" },
        { "GroupMemoryBarrier", "groupMemoryBarrier" },
        { "ddx", "dFdx" },
        { "ddy", "dFdy" },
        { "frac", "fract" },
        { "lerp", "mix" },
        { 0, 0 }
    };
    
    const SDefMacro* Macro = DefMacros;
    
    while (Macro->From != 0)
    {
        if (Name == Macro->From)
        {
            Name = Macro->To;
            return;
        }
        ++Macro;
    }
}

bool ShaderPreProcessor::solveMacroVectorGLSL(io::stringc &Name, const SDataTypeConversion &Type)
{
    const u32 TypeLen = strlen(Type.HLSLDataType);
    
    if (Name.size() <= TypeLen)
    {
        if (Type.GLSLDataType != 0)
            Name = Type.GLSLDataType;
        return true;
    }
    
    /* Setup vector type */
    const c8 Num1 = Name[TypeLen];
    
    if (Num1 < '1' || Num1 > '4')
        return false;
    
    if (Name.size() == TypeLen + 1)
    {
        if (Num1 == '1')
        {
            /* Check for special case "half" -> "float" */
            if (Type.GLSLDataType != 0)
                Name = Type.GLSLDataType;
        }
        else
        {
            /* Solve vector macros: "vec2/3/4" etc. */
            Name = Type.GLSLVecType + Num1;
        }
        return true;
    }
    
    /* Check if name is an (n)x(m)-matrix */
    if (Name.size() < TypeLen + 3)
        return false;
    
    /* Setup matrix type */
    const c8 Num2 = Name[TypeLen + 2];
    
    if (Num2 < '1' || Num2 > '4')
        return false;
    
    if (Num1 == '1' && Num2 == '1')
    {
        /* Check for special case "half1x1" -> "float" */
        if (Type.GLSLDataType != 0)
            Name = Type.GLSLDataType;
    }
    else if (Num1 == '1')
        Name = Type.GLSLVecType + Num2;
    else if (Num2 == '1')
        Name = Type.GLSLVecType + Num1;
    else if (Num1 == Num2)
        Name = Type.GLSLMatType + Num1;
    else
        Name = Type.GLSLMatType + Num1 + io::stringc("x") + Num2;
    
    return true;
}

bool ShaderPreProcessor::solveAttributesGLSL()
{
    /* Get attribute name */
    nextToken();
    
    if (type() != TOKEN_NAME)
    {
        io::Log::error("Unexpected token while processing HLSL attribute at " + Tkn_->getRowColumnString());
        return false;
    }
    
    /* Process attribute */
    const io::stringc& Name = Tkn_->Str;
    
    if (Name == "numthreads")
        return solveAttributeNumThreadsGLSL();
    if (Name == "maxvertexcount")
        return solveAttributeMaxVertexCountGLSL();
    
    //to be continued ...
    
    /* Read until the end of the attribute */
    return nextToken(TOKEN_SQUARED_BRACKET_RIGHT);
}

bool ShaderPreProcessor::solveAttributeNumThreadsGLSL()
{
    append("layout");
    
    c8 LocalSizeCoord = 'x';
    
    while (nextToken())
    {
        if (type() == TOKEN_NUMBER_INT)
        {
            if (LocalSizeCoord <= 'z')
            {
                io::stringc LocalSizeStr = "local_size_x = ";
                LocalSizeStr[11] = LocalSizeCoord++;
                append(LocalSizeStr);
            }
            else
                return exitWithError("Too many arguments for \"numthreads\" attribute");
        }
        else if (type() == TOKEN_SQUARED_BRACKET_RIGHT)
        {
            append(" in;");
            break;
        }
        
        append();
    }
    
    return true;
}

bool ShaderPreProcessor::solveAttributeMaxVertexCountGLSL()
{
    while (nextToken())
    {
        if (type() == TOKEN_NUMBER_INT)
            State_.MaxVertexCount = Tkn_->Str.val<u32>();
        else if (type() == TOKEN_SQUARED_BRACKET_RIGHT)
            break;
    }
    return true;
}

bool ShaderPreProcessor::processEntryPointGLSL()
{
    std::vector<SInputArgument> InputArgs;
    SInputArgument CurArg;
    
    while (nextToken())
    {
        if (type() == TOKEN_COMMA || type() == TOKEN_BRACKET_LEFT)
        {
            if (processInputArgGLSL(CurArg))
                InputArgs.push_back(CurArg);
            else
                return false;
        }
        else if (type() == TOKEN_BRACKET_RIGHT)
            break;
        else
            return exitWithError("Unexpected token in entry-point argument-list");
    }
    
    /* Append shader main function */
    append("void main()\n{\n");
    
    /* Append input arguments */
    pushIndent();
    
    foreach (const SInputArgument &Arg, InputArgs)
    {
        if (Arg.Semantic == "SV_GroupID")
            append(Arg.getHeader() + " = gl_WorkGroupID;\n");
        else if (Arg.Semantic == "SV_GroupThreadID")
            append(Arg.getHeader() + " = gl_LocalInvocationID;\n");
        else if (Arg.Semantic == "SV_DispatchThreadID")
            append(Arg.getHeader() + " = gl_GlobalInvocationID;\n");
        else if (Arg.Semantic == "SV_GroupIndex")
            append(Arg.getHeader() + " = gl_LocalInvocationIndex;\n");
    }
    
    popIndent();
    
    /* Find function block beginning */
    return nextToken(TOKEN_BRACE_LEFT);
}

bool ShaderPreProcessor::processInputArgGLSL(SInputArgument &Arg)
{
    try
    {
        /* Get argument data type */
        if (!nextTokenCheck(TOKEN_NAME))
            throw io::DefaultException("data-type");
        
        if ((Options_ & SHADER_PREPROCESS_SOLVE_MACROS) != 0)
            solveMacrosGLSL();
        
        Arg.DataType = Tkn_->Str;
        
        /* Get argument identifier */
        if (!nextTokenCheck(TOKEN_NAME))
            throw io::DefaultException("identifier");
        
        Arg.Identifier = Tkn_->Str;
        
        /* Get argument semantic */
        if (!nextTokenCheck(TOKEN_COLON))
            throw io::DefaultException("':' character");
        if (!nextTokenCheck(TOKEN_NAME))
            throw io::DefaultException("semantic");
        
        Arg.Semantic = Tkn_->Str;
    }
    catch (const io::DefaultException &Err)
    {
        return exitWithError(
            "Unexpected token in entry-point argument-list (expected " +
            io::stringc(Err.what()) + ")"
        );
    }
    
    return true;
}


/*
 * SInputArgument structure
 */

ShaderPreProcessor::SInputArgument::SInputArgument()
{
}
ShaderPreProcessor::SInputArgument::~SInputArgument()
{
}


/*
 * SInternalState structure
 */

ShaderPreProcessor::SInternalState::SInternalState() :
    MaxVertexCount(0)
{
}
ShaderPreProcessor::SInternalState::~SInternalState()
{
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================