#include "gnsl.h"
#include <vector>
#include <optional>

typedef enum
{
    GnslTokenType_Identifier,

    // Operators
    GnslTokenType_OpAdd,
    GnslTokenType_OpSubtract,
    GnslTokenType_OpMultiply,
    GnslTokenType_OpDivide,
    GnslTokenType_OpModulo,
    GnslTokenType_OpAndBitwise,
    GnslTokenType_OpOrBitwise,
    GnslTokenType_OpNotBitwise,
    GnslTokenType_OpXor,
    GnslTokenType_OpLShift,
    GnslTokenType_OpRShift,
    GnslTokenType_OpIncrement,
    GnslTokenType_OpDecrement,
    GnslTokenType_OpAssign,
    GnslTokenType_OpAddAssign,
    GnslTokenType_OpSubtractAssign,
    GnslTokenType_OpMultiplyAssign,
    GnslTokenType_OpDivideAssign,
    GnslTokenType_OpModuloAssign,
    GnslTokenType_OpAndBitwiseAssign,
    GnslTokenType_OpOrBitwiseAssign,
    GnslTokenType_OpNotBitwiseAssign,
    GnslTokenType_OpXorAssign,
    GnslTokenType_OpLShiftAssign,
    GnslTokenType_OpRShiftAssign,

    // Built-in type keywords
    GnslTokenType_Const,
    GnslTokenType_Void,
    GnslTokenType_Bool,
    GnslTokenType_Int,
    GnslTokenType_Uint,
    GnslTokenType_Float,
    GnslTokenType_Bool2,
    GnslTokenType_Int2,
    GnslTokenType_Uint2,
    GnslTokenType_Float2,
    GnslTokenType_Bool3,
    GnslTokenType_Int3,
    GnslTokenType_Uint3,
    GnslTokenType_Float3,
    GnslTokenType_Bool4,
    GnslTokenType_Int4,
    GnslTokenType_Uint4,
    GnslTokenType_Float4,

    // Access specifier keywords
    GnslTokenType_In,
    GnslTokenType_Out,
    GnslTokenType_Inout,

    // Control flow keywords
    GnslTokenType_If,
    GnslTokenType_Else,
    GnslTokenType_Switch,
    GnslTokenType_Case,
    GnslTokenType_Default,
    GnslTokenType_For,
    GnslTokenType_While,
    GnslTokenType_Do,
    GnslTokenType_Break,
    GnslTokenType_Continue,
    GnslTokenType_Discard,
    GnslTokenType_Return,
} GnslTokenType;

typedef enum
{
    
} GnslNodeType;

struct GnslToken
{
    GnslTokenType type;
};

struct GnslNode
{
    GnslNodeType type;
};

struct GnslCompiler_t
{
    std::optional<std::vector<GnslToken>> GenerateToken(size_t size, const char* str, GnslCompilationResult compilation_result) noexcept;
};

struct GnslCompilationResult_t
{

};

GnslResult GnslCreateCompiler(GnslCompiler* compiler)
{
    *compiler = new GnslCompiler_t();
    return GnslError_Unimplemented;
}

void GnslDestroyCompiler(GnslCompiler compiler)
{
    delete compiler;
}

GnslResult GnslCompileFromString(GnslCompiler compiler, GnslShaderType shader_type, size_t size, const char* str, GnslCompilationResult* compilation_result)
{
    GnslCompilationResult result = new GnslCompilationResult_t();
    auto tokens = compiler->GenerateToken(size, str, result);

    if (!tokens) {
        return GnslError_CompilationFailed;
    }
    
    return GnslError_Unimplemented;
}

void GnslDestroyCompilationResult(GnslCompilationResult compilation_result)
{
}

std::tuple<const char*, char32_t> GnslGetNextUTF8Char(const char* str, const char* end_str) noexcept
{
    return { str, 0 };
}

std::optional<std::vector<GnslToken>> GnslCompiler_t::GenerateToken(size_t size, const char* str, GnslCompilationResult compilation_result) noexcept
{
    const char* current_str = str;
    const char* end_str = str + size;

    while (current_str != end_str) {
        auto [next_str, current_char] = GnslGetNextUTF8Char(str, end_str);

        current_str = next_str;
    }

    return {};
}
