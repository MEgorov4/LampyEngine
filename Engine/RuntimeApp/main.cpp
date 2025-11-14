#include "RuntimeApplication.h"
#include <string_view>
#include <string>

int main(int argc, char** argv)
{
    RuntimeApplication app;

    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg(argv[i]);
        if ((arg == "--project" || arg == "-p") && i + 1 < argc)
        {
            app.setProjectFileOverride(argv[++i]);
            break;
        }
        else if (!arg.empty() && arg[0] != '-')
        {
            app.setProjectFileOverride(std::string(arg));
            break;
        }
    }

    app.startup();
    return 0;
}

