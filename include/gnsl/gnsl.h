#ifndef GNSL_H_
#define GNSL_H_

#include <stdint.h>
#include <stddef.h>

#define GNSL_INVALID_CHAR (~0U)

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned char GnslByte;

typedef struct GnslCompiler_t* GnslCompiler;
typedef struct GnslCompilationResult_t* GnslCompilationResult;

typedef enum
{
    GnslSuccess,
    GnslError_Unknown           = -1,
    GnslError_Unimplemented     = -2,
    GnslError_CannotOpenFile    = -3,
    GnslError_CompilationFailed = -4
} GnslResult;

typedef enum
{
    GnslShaderType_Auto     = 0,
    GnslShaderType_Vertex   = 1,
    GnslShaderType_Fragment = 2,
    GnslShaderType_Compute  = 3,
} GnslShaderType;

typedef enum
{
    GnslTarget_SPIRV,
} GnslTarget;

GnslResult GnslCreateCompiler(GnslCompiler* compiler);
void GnslDestroyCompiler(GnslCompiler compiler);

GnslResult GnslCompileFromFile(GnslCompiler compiler, GnslShaderType shader_type, const char* path, GnslCompilationResult* compilation_result);
GnslResult GnslCompileFromMemory(GnslCompiler compiler, GnslShaderType shader_type, size_t size, const GnslByte* str, GnslCompilationResult* compilation_result);
void GnslDestroyCompilationResult(GnslCompilationResult compilation_result);

#ifdef __cplusplus
}
#endif

#endif // GNSL_H_