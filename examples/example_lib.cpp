#include "example_lib.h"

GnExampleApp* GnExampleApp::g_app = nullptr;

int main(int argc, const char* argv[])
{
    if (GnExampleApp::g_app == nullptr)
        return -1;

    GnExampleApp::g_app->Init();

    return GnExampleApp::g_app->Run();
}