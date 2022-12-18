#ifndef GNSL_IMPL_H_
#define GNSL_IMPL_H_

#include "gnsl.h"
#include "gnsl_frontend.h"
#include <memory>
#include <cstdio>

struct GnslCompiler_t
{
    GnslResult CompileFromMemory(GnslShaderType shader_type, size_t size, const GnslByte* str, GnslCompilationResult* compilation_result);
};

struct GnslCompilationResult_t
{

};

// ----

GnslResult GnslCreateCompiler(GnslCompiler* compiler)
{
    *compiler = new GnslCompiler_t();
    return GnslSuccess;
}

void GnslDestroyCompiler(GnslCompiler compiler)
{
    delete compiler;
}

GnslResult GnslCompileFromFile(GnslCompiler compiler, GnslShaderType shader_type, const char* path, GnslCompilationResult* compilation_result)
{
    std::FILE* file = std::fopen(path, "rb");

    if (file == nullptr)
        return GnslError_CannotOpenFile;

    std::fseek(file, 0, SEEK_END);
    
    int file_size = std::ftell(file);
    int string_size = file_size + 1;
    GnslByte* str = (GnslByte*)std::malloc(string_size);

    if (str == nullptr)
        return GnslError_CannotOpenFile;
    
    std::memset(str, 0, string_size);
    std::fseek(file, 0, SEEK_SET);
    std::fread(str, 1, sizeof(GnslByte) * file_size, file);
    std::fclose(file);

    GnslResult result = compiler->CompileFromMemory(shader_type, file_size, str, compilation_result);

    std::free(str);

    return result;
}

GnslResult GnslCompileFromMemory(GnslCompiler compiler, GnslShaderType shader_type, size_t size, const GnslByte* str, GnslCompilationResult* compilation_result)
{
    return compiler->CompileFromMemory(shader_type, size, str, compilation_result);
}

void GnslDestroyCompilationResult(GnslCompilationResult compilation_result)
{
}

GnslResult GnslCompiler_t::CompileFromMemory(GnslShaderType shader_type, size_t size, const GnslByte* str, GnslCompilationResult* compilation_result)
{
    std::unique_ptr<GnslCompilationResult_t> result = std::make_unique<GnslCompilationResult_t>();
    GnslLexer lexer;
    auto tokens = lexer.GenerateToken(size, str, result.get());

    if (!tokens) {
        return GnslError_CompilationFailed;
    }

    PrintToken(*tokens);

    GnslParser parser;
    parser.SetTokenStream(&*tokens);
    parser.Parse();

    *compilation_result = result.release();
    return GnslError_Unimplemented;
}

#endif // GNSL_IMPL_H_