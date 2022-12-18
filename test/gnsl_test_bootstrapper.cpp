#include <gnsl/gnsl.h>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main()
{
    fs::path test_case_dir(GNSL_TEST_CASE_DIR, fs::path::native_format);
    GnslCompiler compiler;

    GnslCreateCompiler(&compiler);

    for (const auto& entry : fs::directory_iterator{ test_case_dir }) {
        GnslCompilationResult compilation_result;
        std::cout << entry.path().filename() << std::endl;
        GnslCompileFromFile(compiler, GnslShaderType_Fragment, entry.path().generic_string().c_str(), &compilation_result);
        GnslDestroyCompilationResult(compilation_result);
    }

    GnslDestroyCompiler(compiler);

    return 0;
}