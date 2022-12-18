#ifndef GNSL_FRONTEND_H_
#define GNSL_FRONTEND_H_

#include "gnsl.h"
#include <gn/gn_core.h>
#include <vector>
#include <optional>
#include <string_view>
#include <cmath>
#include <iostream>

#define GNSL_EOT(x) ((x) == GnslParseResult_EndOfToken)

struct GnslNode;

typedef enum
{
    GnslTokenType_Invalid = -1,
    GnslTokenType_Undefined,
    GnslTokenType_Identifier,
    GnslTokenType_NumberLiteral,

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
    GnslTokenType_OpLessThan,
    GnslTokenType_OpGreaterThan,
    GnslTokenType_OpLessThanOrEqual,
    GnslTokenType_OpGreaterThanOrEqual,
    GnslTokenType_OpAnd,
    GnslTokenType_OpOr,
    GnslTokenType_OpNot,
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
    GnslTokenType_OpXorAssign,
    GnslTokenType_OpLShiftAssign,
    GnslTokenType_OpRShiftAssign,
    GnslTokenType_OpAndAssign,
    GnslTokenType_OpOrAssign,

    // Symbols
    GnslTokenType_LParen,
    GnslTokenType_RParen,
    GnslTokenType_LBracket,
    GnslTokenType_RBracket,
    GnslTokenType_LCurlyBracket,
    GnslTokenType_RCurlyBracket,
    GnslTokenType_Comma,
    GnslTokenType_Dot,
    GnslTokenType_Ternary,
    GnslTokenType_Colon,
    GnslTokenType_Semicolon,

    // Built-in type keywords
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
    GnslTokenType_Const,
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
    GnslNumberLiteralType_I8,
    GnslNumberLiteralType_I16,
    GnslNumberLiteralType_I32,
    GnslNumberLiteralType_I64,
    GnslNumberLiteralType_U8,
    GnslNumberLiteralType_U16,
    GnslNumberLiteralType_U32,
    GnslNumberLiteralType_U64,
    GnslNumberLiteralType_F32,
    GnslNumberLiteralType_F64,
} GnslNumberLiteralType;

typedef enum
{
    GnslNodeType_Program,
    GnslNodeType_VarDecl,
    GnslNodeType_UniformDecl,
    GnslNodeType_FuncDecl,
    GnslNodeType_ScopeBlock,
} GnslNodeType;

typedef enum
{
    GnslParseResult_Success,
    GnslParseResult_Failed      = -1,
    GnslParseResult_EndOfToken  = -2,
} GnslParseResult;

struct GnslIdentifierToken
{
    const GnslByte* begin_str;
    const GnslByte* end_str;
};

struct GnslNumberLiteralToken
{
    GnslNumberLiteralType   type;

    union
    {
        int64_t             i64;
        uint64_t            u64;
        double              f64;

        uint32_t            u32;
        int32_t             i32;
        float               f32;

        uint8_t             u8;
        uint16_t            u16;

        int8_t              i8;
        int16_t             i16;
    };
};

struct GnslToken
{
    GnslTokenType type;

    union
    {
        GnslIdentifierToken     identifier;
        GnslNumberLiteralToken  number;
    };
};

struct GnslCommonDecl
{
    bool constant;
    GnslTokenType type;
    GnslIdentifierToken type_id;
    GnslIdentifierToken id;
};

struct GnslVarDeclNode
{
    GnslCommonDecl decl_info;
    GnslNode* expr_node;
};

struct GnslFuncDeclNode
{
    GnslCommonDecl decl_info;
    GnslNode* var_decl_node;
    GnslNode* scope_node;
};

struct GnslScopeBlockNode
{
    GnslNode* stmt_node;
};

struct GnslBinaryOp
{
    GnslTokenType type;
    GnslIdentifierToken var_id;
    GnslNode* expr_node;
};

struct GnslNode
{
    GnslNodeType type;
    GnslNode* next_node;

    union
    {
        GnslVarDeclNode         var_decl;
        GnslFuncDeclNode        func_decl;
        GnslScopeBlockNode      scope;
        GnslBinaryOp    assignment;
    };
};

struct GnslAST
{
    GnPool<GnslNode> node_pool;
    GnslNode program_node{};

    GnslAST() :
        node_pool(512)
    {
        program_node.type = GnslNodeType_Program;
    }

    GnslNode* CreateNode(GnslNodeType type) noexcept
    {
        auto node = (GnslNode*)node_pool.allocate();

        if (node == nullptr)
            return nullptr;

        new(node) GnslNode();
        node->type = type;

        return node;
    }

    GnslNode* CreateVarDeclNode() noexcept
    {
        return CreateNode(GnslNodeType_VarDecl);
    }

    GnslNode* CreateFuncDeclNode() noexcept
    {
        return CreateNode(GnslNodeType_FuncDecl);
    }

    GnslNode* CreateScopeBlockNode() noexcept
    {
        return CreateNode(GnslNodeType_ScopeBlock);
    }
};

using TokenStream = std::vector<GnslToken>;
using TokenIter = TokenStream::const_iterator;

struct GnslLexer
{
    std::optional<TokenStream> GenerateToken(size_t size, const GnslByte* str, GnslCompilationResult compilation_result) noexcept;
};

struct GnslParser
{
    GnslAST ast;
    std::vector<std::string> error_msg;
    const TokenStream* token_stream = nullptr;
    GnslNode* current_global_decl = nullptr;

    void SetTokenStream(const TokenStream* token_stream);
    void Parse();
};

std::tuple<const GnslByte*, char32_t> GetNextUTF8Char(const GnslByte* str, const GnslByte* end_str) noexcept
{
    char32_t ch = GNSL_INVALID_CHAR;
    const GnslByte* next_ch = str;

    if (next_ch == end_str)
        return { end_str, U'\0' };

    if (str[0] < 0x80) {
        ch = str[0];
        next_ch += 1;
    }
    else if ((str[0] & 0xe0) == 0xc0) {
        ch = ((char32_t)(str[0] & 0x1f) << 6) | (char32_t)(str[0] & 0x3f);
        next_ch += 2;
    }
    else if ((str[0] & 0xf0) == 0xe0) {
        ch = ((char32_t)(str[0] & 0x0f) << 12) | ((char32_t)(str[1] & 0x3f) << 6) | (char32_t)(str[2] & 0x3f);
        next_ch += 3;
    }
    else if ((str[0] & 0xf8) == 0xf0 && (str[0] <= 0xf4)) {
        ch = ((char32_t)(str[0] & 0x07) << 18) | ((char32_t)(str[0] & 0x3f) << 12) | ((char32_t)(str[1] & 0x3f) << 6) | (char32_t)(str[2] & 0x3f);
        next_ch += 4;
    }
    else
        next_ch += 1;

    if (ch >= 0xd800 && ch <= 0xdfff)
        ch = GNSL_INVALID_CHAR;

    return { next_ch, ch };
}

inline static std::u32string_view GetKeywordString(GnslTokenType token_type) noexcept
{
    switch (token_type) {
        // Built-in type keywords
        case GnslTokenType_Const:       return U"const";
        case GnslTokenType_Void:        return U"void";
        case GnslTokenType_Bool:        return U"bool";
        case GnslTokenType_Int:         return U"int";
        case GnslTokenType_Uint:        return U"uint";
        case GnslTokenType_Float:       return U"float";
        case GnslTokenType_Bool2:       return U"bool2";
        case GnslTokenType_Int2:        return U"int2";
        case GnslTokenType_Uint2:       return U"uint2";
        case GnslTokenType_Float2:      return U"float2";
        case GnslTokenType_Bool3:       return U"bool3";
        case GnslTokenType_Int3:        return U"int3";
        case GnslTokenType_Uint3:       return U"uint3";
        case GnslTokenType_Float3:      return U"float3";
        case GnslTokenType_Bool4:       return U"bool4";
        case GnslTokenType_Int4:        return U"int4";
        case GnslTokenType_Uint4:       return U"uint4";
        case GnslTokenType_Float4:      return U"float4";

        // Access specifier keywords    
        case GnslTokenType_In:          return U"in";
        case GnslTokenType_Out:         return U"out";
        case GnslTokenType_Inout:       return U"inout";

        // Control flow keywords
        case GnslTokenType_If:          return U"if";
        case GnslTokenType_Else:        return U"else";
        case GnslTokenType_Switch:      return U"switch";
        case GnslTokenType_Case:        return U"case";
        case GnslTokenType_Default:     return U"default";
        case GnslTokenType_For:         return U"for";
        case GnslTokenType_While:       return U"while";
        case GnslTokenType_Do:          return U"do";
        case GnslTokenType_Break:       return U"break";
        case GnslTokenType_Continue:    return U"continue";
        case GnslTokenType_Discard:     return U"discard";
        case GnslTokenType_Return:      return U"return";
        default:                        break;
    }

    return {};
}

inline static bool IsWhitespace(char32_t ch) noexcept
{
    return (ch == U'\t' || ch == U'\n' || ch == U'\r' || ch == U' ');
}

inline static bool IsNumber(char32_t ch) noexcept
{
    return (ch >= U'0' && ch <= U'9');
}

inline static bool IsSpecialChar(char32_t ch) noexcept
{
    return (ch == U'+' || ch == U'-' || ch == U'*' ||
            ch == U'/' || ch == U'%' || ch == U'>' ||
            ch == U'<' || ch == U'!' || ch == U'=' ||
            ch == U'&' || ch == U'|' || ch == U'^' ||
            ch == U'~' || ch == U'?' || ch == U':' ||
            ch == U'(' || ch == U')' || ch == U'[' ||
            ch == U']' || ch == U'{' || ch == U'}' ||
            ch == U',' || ch == U'.' || ch == U';' ||
            ch == U'#' || ch == U'"' || ch == U'\'' ||
            ch == U'\\' || ch == U'#');
}

inline static bool IsIdentifier(char32_t ch) noexcept
{
    return !(IsWhitespace(ch) || IsSpecialChar(ch));
}

const GnslByte* SkipWhitespace(const GnslByte* current_pos, const GnslByte* end_str)
{
    while (current_pos != end_str) {
        auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);
        if (!IsWhitespace(ch))
            break;
        current_pos = next_pos;
    }

    return current_pos;
}

using ParseResult = std::pair<GnslToken, const GnslByte*>;

ParseResult ParseSymbol(const GnslByte* current_pos, const GnslByte* end_str)
{
    auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

    switch (ch) {
        case U';': return { { GnslTokenType_Semicolon }, next_pos };
        case U'(': return { { GnslTokenType_LParen }, next_pos };
        case U')': return { { GnslTokenType_RParen }, next_pos };
        case U'[': return { { GnslTokenType_LBracket }, next_pos };
        case U']': return { { GnslTokenType_RBracket }, next_pos };
        case U'{': return { { GnslTokenType_LCurlyBracket }, next_pos };
        case U'}': return { { GnslTokenType_RCurlyBracket }, next_pos };
        case U',': return { { GnslTokenType_Comma }, next_pos };
        case U'.': return { { GnslTokenType_Dot }, next_pos };
        case U'?': return { { GnslTokenType_Ternary }, next_pos };
        case U':': return { { GnslTokenType_Colon }, next_pos };
        case U'~': return { { GnslTokenType_OpNotBitwise }, next_pos };
        case U'!': return { { GnslTokenType_OpNot }, next_pos };

        case U'+':
        {
            auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

            if (ch == U'+')
                return { { GnslTokenType_OpIncrement }, next_pos };
            else if (ch == U'=')
                return { { GnslTokenType_OpAddAssign }, next_pos };

            return { { GnslTokenType_OpAdd }, next_pos };
        }

        case U'-':
        {
            auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

            if (ch == U'-')
                return { { GnslTokenType_OpDecrement }, next_pos };
            else if (ch == U'=')
                return { { GnslTokenType_OpSubtractAssign }, next_pos };

            return { { GnslTokenType_OpSubtract }, next_pos };
        }

        case U'*':
        {
            auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

            if (ch == U'=')
                return { { GnslTokenType_OpMultiplyAssign }, next_pos };

            return { { GnslTokenType_OpMultiply }, next_pos };
        }

        case U'/':
        {
            auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

            if (ch == U'=')
                return { { GnslTokenType_OpDivideAssign }, next_pos };

            return { { GnslTokenType_OpDivide }, next_pos };
        }

        case U'%':
        {
            auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

            if (ch == U'=')
                return { { GnslTokenType_OpModuloAssign }, next_pos };

            return { { GnslTokenType_OpModulo }, next_pos };
        }

        case U'&':
        {
            auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

            if (ch == '=')
                return { { GnslTokenType_OpAndBitwiseAssign }, next_pos };
            else if (ch == '&')
                return { { GnslTokenType_OpAnd }, next_pos };

            return { { GnslTokenType_OpAndBitwise }, next_pos };
        }

        case U'|':
        {
            auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

            if (ch == '=')
                return { { GnslTokenType_OpOrBitwiseAssign }, next_pos };
            else if (ch == '|')
                return { { GnslTokenType_OpOr }, next_pos };

            return { { GnslTokenType_OpOrBitwise }, next_pos };
        }

        case U'^':
        {
            auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

            if (ch == U'=')
                return { { GnslTokenType_OpXorAssign }, next_pos };

            return { { GnslTokenType_OpXor }, next_pos };
        }

        case U'>':
        {
            auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

            if (ch == U'=')
                return { { GnslTokenType_OpGreaterThanOrEqual }, next_pos };
            else if (ch == U'>') {
                auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

                if (ch == U'=')
                    return { { GnslTokenType_OpRShiftAssign }, next_pos };

                return { { GnslTokenType_OpRShift }, next_pos };
            }

            return { { GnslTokenType_OpGreaterThan }, next_pos };
        }

        case U'<':
        {
            auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

            if (ch == U'=')
                return { { GnslTokenType_OpLessThanOrEqual }, next_pos };
            else if (ch == U'<') {
                auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

                if (ch == U'=')
                    return { { GnslTokenType_OpLShiftAssign }, next_pos };

                return { { GnslTokenType_OpLShift }, next_pos };
            }

            return { { GnslTokenType_OpLessThan }, next_pos };
        }
    }

    return { {}, current_pos };
}

ParseResult ParseIdentifier(const GnslByte* current_pos, const GnslByte* end_str)
{
    GnslToken token{};
    const GnslByte* begin_str = current_pos;
    auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

    if (!IsNumber(ch) && IsIdentifier(ch)) {
        current_pos = next_pos;
        token.type = GnslTokenType_Identifier;

        while (current_pos != end_str) {
            auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);
            if (!IsIdentifier(ch))
                break;
            current_pos = next_pos;
        }

        token.identifier.begin_str = begin_str;
        token.identifier.end_str = current_pos;
    }

    return { token, current_pos };
}

// TODO: handle big numbers
ParseResult ParseNumber(const GnslByte* current_pos, const GnslByte* end_str)
{
    uint64_t decimal = 0;
    uint64_t fraction = 0;
    uint64_t exponent = 0;
    uint32_t decimal_digits = 0;
    uint32_t fraction_digits = 0;
    bool is_float = false;
    bool is_negative_exponent = false;
    const GnslByte* last_pos = current_pos;
    GnslToken token{};

    // Decimal part
    while (current_pos != end_str) {
        auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);
        if (!IsNumber(ch))
            break;
        decimal = (decimal * 10) + (uint64_t)(ch - U'0');
        token.type = GnslTokenType_NumberLiteral;
        current_pos = next_pos;
        decimal_digits++;
    }

    auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

    if (ch == U'.') {
        is_float = true;
        current_pos = next_pos;

        // Fraction part
        while (current_pos != end_str) {
            auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);
            if (!IsNumber(ch))
                break;

            fraction = (fraction * 10) + (uint64_t)(ch - U'0');
            token.type = GnslTokenType_NumberLiteral;
            current_pos = next_pos;
            fraction_digits++;
        }
    }

    auto [next_pos2, exp_char] = GetNextUTF8Char(current_pos, end_str);

    if (decimal_digits > 0 && exp_char == U'e' || exp_char == U'E') {
        is_float = true;
        current_pos = next_pos2;
        auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);

        if (ch == U'-') {
            is_negative_exponent = true;
            current_pos = next_pos;
        }
        else if (ch == U'+') {
            current_pos = next_pos;
        }

        token.type = GnslTokenType_Invalid;

        // Exponent part
        while (current_pos != end_str) {
            auto [next_pos, ch] = GetNextUTF8Char(current_pos, end_str);
            if (!(ch >= U'0' && ch <= U'9'))
                return { {}, current_pos };
            exponent = (exponent * 10) + (uint64_t)(ch - U'0');
            token.type = GnslTokenType_NumberLiteral;
            current_pos = next_pos;
        }
    }

    if (is_float) {
        double frac_exp = std::pow(10.0, -(double)fraction_digits);
        double dec_exp = std::pow(10.0, is_negative_exponent ? -(double)exponent : exponent);
        token.number.type = GnslNumberLiteralType_F32;
        token.number.f32 = (float)((decimal + (double)fraction * frac_exp) * dec_exp);
    }
    else {
        token.number.type = GnslNumberLiteralType_I32;
        token.number.i32 = (int32_t)decimal;
    }

    return { token, current_pos };
}

void ParseKeywords(std::vector<GnslToken>& tokens)
{
    for (auto& token : tokens) {
        if (token.type == GnslTokenType_Identifier) {
            for (uint32_t i = GnslTokenType_Void; i <= GnslTokenType_Return; i++) {
                GnslTokenType type = (GnslTokenType)i;
                std::u32string_view keyword = GetKeywordString(type);
                const GnslByte* current_pos = token.identifier.begin_str;
                auto keyword_pos = keyword.begin();
                bool match = true;

                while (match && current_pos != token.identifier.end_str && keyword_pos != keyword.end()) {
                    auto [next_pos, ch] = GetNextUTF8Char(current_pos, token.identifier.end_str);

                    if (ch != *keyword_pos) {
                        match = false;
                        break;
                    }
                    
                    keyword_pos++;
                    current_pos = next_pos;
                }

                if (match && current_pos == token.identifier.end_str && keyword_pos == keyword.end()) {
                    token.type = type;
                    token.identifier = {};
                    break;
                }
            }
        }
    }
}

std::optional<TokenStream> GnslLexer::GenerateToken(size_t size, const GnslByte* str, GnslCompilationResult compilation_result) noexcept
{
    const GnslByte* current_pos = str;
    const GnslByte* end_str = str + size;
    TokenStream tokens;

    while (current_pos != end_str) {
        current_pos = SkipWhitespace(current_pos, end_str);

        {
            auto [token, next_pos] = ParseSymbol(current_pos, end_str);

            if (token.type != GnslTokenType_Undefined) {
                tokens.push_back(token);
                current_pos = next_pos;
                continue;
            }
        }

        {
            auto [token, next_pos] = ParseIdentifier(current_pos, end_str);
            
            if (token.type != GnslTokenType_Undefined) {
                tokens.push_back(token);
                current_pos = next_pos;
                continue;
            }
        }

        {
            auto [token, next_pos] = ParseNumber(current_pos, end_str);

            if (token.type != GnslTokenType_Undefined) {
                tokens.push_back(token);
                current_pos = next_pos;
                continue;
            }
        }
    }

    // Transform identifiers that contain reserved keywords.
    ParseKeywords(tokens);

    return { std::move(tokens) };
}

bool IsDataTypeKeyword(GnslTokenType type) noexcept
{
    return (type >= GnslTokenType_Void && type <= GnslTokenType_Float4);
}

void GnslParser::SetTokenStream(const TokenStream* stream)
{
    token_stream = stream;
}

void GnslParser::Parse()
{
    if (token_stream == nullptr)
        return;

    auto current_token = token_stream->begin();
    auto end_token = token_stream->end();

    while (current_token != end_token) {
        
    }
}

// 

void PrintToken(const TokenStream& tokens)
{
    for (const auto& token : tokens) {
        switch (token.type) {
            case GnslTokenType_Invalid:
                std::cout << "INVALID ";
                break;

            case GnslTokenType_Undefined:
                std::cout << "UNDEFINED ";
                break;

            case GnslTokenType_Identifier:
            {
                std::cout << "ID(";
                
                for (auto begin = token.identifier.begin_str; begin != token.identifier.end_str;) {
                    auto [next_pos, ch] = GetNextUTF8Char(begin, token.identifier.end_str);
                    std::cout << (char)ch;
                    begin = next_pos;
                }

                std::cout << ") ";

                break;
            }

            case GnslTokenType_NumberLiteral:
            {
                std::cout << "NUMBER<";

                switch (token.number.type) {
                    case GnslNumberLiteralType_I32:
                        std::cout << "I32";
                        break;
                    case GnslNumberLiteralType_F32:
                        std::cout << "F32";
                        break;
                }

                std::cout << ">(";

                switch (token.number.type) {
                    case GnslNumberLiteralType_I32:
                        std::cout << token.number.i32;
                        break;
                    case GnslNumberLiteralType_F32:
                        std::cout << token.number.f32;
                        break;
                }

                std::cout << ") ";

                break;
            }

            // Operators
            case GnslTokenType_OpAdd:
                std::cout << "OP_ADD ";
                break;

            case GnslTokenType_OpSubtract:
                std::cout << "OP_SUBTRACT ";
                break;

            case GnslTokenType_OpMultiply:
                std::cout << "OP_MULTIPLY ";
                break;

            case GnslTokenType_OpDivide:
                std::cout << "OP_DIVIDE ";
                break;

            case GnslTokenType_OpModulo:
                std::cout << "OP_MODULO ";
                break;

            case GnslTokenType_OpAndBitwise:
                std::cout << "OP_AND_BITWISE ";
                break;

            case GnslTokenType_OpOrBitwise:
                std::cout << "OP_OR_BITWISE ";
                break;

            case GnslTokenType_OpNotBitwise:
                std::cout << "OP_NOT_BITWISE ";
                break;

            case GnslTokenType_OpXor:
                std::cout << "OP_XOR ";
                break;

            case GnslTokenType_OpLShift:
                std::cout << "OP_LSHIFT ";
                break;

            case GnslTokenType_OpRShift:
                std::cout << "OP_RSHIFT ";
                break;

            case GnslTokenType_OpLessThan:
                std::cout << "OP_LT ";
                break;

            case GnslTokenType_OpGreaterThan:
                std::cout << "OP_GT ";
                break;

            case GnslTokenType_OpLessThanOrEqual:
                std::cout << "OP_LE ";
                break;

            case GnslTokenType_OpGreaterThanOrEqual:
                std::cout << "OP_GE ";
                break;

            case GnslTokenType_OpAnd:
                std::cout << "OP_AND ";
                break;

            case GnslTokenType_OpOr:
                std::cout << "OP_OR ";
                break;

            case GnslTokenType_OpNot:
                std::cout << "OP_NOT ";
                break;

            case GnslTokenType_OpIncrement:
                std::cout << "OP_INC ";
                break;

            case GnslTokenType_OpDecrement:
                std::cout << "OP_DEC ";
                break;

            case GnslTokenType_OpAssign:
                std::cout << "OP_ASSIGN ";
                break;

            case GnslTokenType_OpAddAssign:
                std::cout << "OP_ADD_ASSIGN ";
                break;

            case GnslTokenType_OpSubtractAssign:
                std::cout << "OP_SUB_ASSIGN ";
                break;

            case GnslTokenType_OpMultiplyAssign:
                std::cout << "OP_MUL_ASSIGN ";
                break;

            case GnslTokenType_OpDivideAssign:
                std::cout << "OP_DIV_ASSIGN ";
                break;

            case GnslTokenType_OpModuloAssign:
                std::cout << "OP_MOD_ASSIGN ";
                break;

            case GnslTokenType_OpAndBitwiseAssign:
                std::cout << "OP_AND_BITWISE_ASSIGN ";
                break;

            case GnslTokenType_OpOrBitwiseAssign:
                std::cout << "OP_OR_BITWISE_ASSIGN ";
                break;

            case GnslTokenType_OpXorAssign:
                std::cout << "OP_XOR_ASSIGN ";
                break;

            case GnslTokenType_OpLShiftAssign:
                std::cout << "OP_LSHIFT_ASSIGN ";
                break;

            case GnslTokenType_OpRShiftAssign:
                std::cout << "OP_RSHIFT_ASSIGN ";
                break;

            case GnslTokenType_OpAndAssign:
                std::cout << "OP_AND_ASSIGN ";
                break;

            case GnslTokenType_OpOrAssign:
                std::cout << "OP_OR_ASSIGN ";
                break;

            // Symbols
            case GnslTokenType_LParen:
                std::cout << "( ";
                break;

            case GnslTokenType_RParen:
                std::cout << ") ";
                break;

            case GnslTokenType_LBracket:
                std::cout << "[ ";
                break;

            case GnslTokenType_RBracket:
                std::cout << "] ";
                break;

            case GnslTokenType_LCurlyBracket:
                std::cout << "{ ";
                break;

            case GnslTokenType_RCurlyBracket:
                std::cout << "} ";
                break;

            case GnslTokenType_Comma:
                std::cout << ", ";
                break;

            case GnslTokenType_Dot:
                std::cout << ". ";
                break;

            case GnslTokenType_Ternary:
                std::cout << "TERNARY ";
                break;

            case GnslTokenType_Colon:
                std::cout << ": ";
                break;

            case GnslTokenType_Semicolon:
                std::cout << "; ";
                break;

            // Built-in type keywords
            case GnslTokenType_Const:
                std::cout << "CONST ";
                break;

            case GnslTokenType_Void:
                std::cout << "VOID ";
                break;

            case GnslTokenType_Bool:
                std::cout << "BOOL ";
                break;

            case GnslTokenType_Int:
                std::cout << "INT ";
                break;

            case GnslTokenType_Uint:
                std::cout << "UINT ";
                break;

            case GnslTokenType_Float:
                std::cout << "FLOAT ";
                break;

            case GnslTokenType_Bool2:
                std::cout << "BOOL2 ";
                break;

            case GnslTokenType_Int2:
                std::cout << "INT2 ";
                break;

            case GnslTokenType_Uint2:
                std::cout << "UINT2 ";
                break;

            case GnslTokenType_Float2:
                std::cout << "FLOAT2 ";
                break;

            case GnslTokenType_Bool3:
                std::cout << "BOOL3 ";
                break;

            case GnslTokenType_Int3:
                std::cout << "INT3 ";
                break;

            case GnslTokenType_Uint3:
                std::cout << "UINT3 ";
                break;

            case GnslTokenType_Float3:
                std::cout << "FLOAT3 ";
                break;

            case GnslTokenType_Bool4:
                std::cout << "BOOL4 ";
                break;

            case GnslTokenType_Int4:
                std::cout << "INT4 ";
                break;

            case GnslTokenType_Uint4:
                std::cout << "UINT4 ";
                break;

            case GnslTokenType_Float4:
                std::cout << "FLOAT4 ";
                break;

            // Access specifier keywords
            case GnslTokenType_In:
                std::cout << "IN ";
                break;

            case GnslTokenType_Out:
                std::cout << "OUT ";
                break;

            case GnslTokenType_Inout:
                std::cout << "INOUT ";
                break;

            // Control flow keywords
            case GnslTokenType_If:
                std::cout << "IF ";
                break;

            case GnslTokenType_Else:
                std::cout << "ELSE ";
                break;

            case GnslTokenType_Switch:
                std::cout << "SWITCH ";
                break;

            case GnslTokenType_Case:
                std::cout << "CASE ";
                break;

            case GnslTokenType_Default:
                std::cout << "DEFAULT ";
                break;

            case GnslTokenType_For:
                std::cout << "FOR ";
                break;

            case GnslTokenType_While:
                std::cout << "WHILE ";
                break;

            case GnslTokenType_Do:
                std::cout << "DO ";
                break;

            case GnslTokenType_Break:
                std::cout << "BREAK ";
                break;

            case GnslTokenType_Continue:
                std::cout << "CONTINUE ";
                break;

            case GnslTokenType_Discard:
                std::cout << "DISCARD ";
                break;

            case GnslTokenType_Return:
                std::cout << "RETURN ";
                break;
        }
    }

    std::cout << std::endl;
}

#endif // GNSL_FRONTEND_H_