#include <iostream>
#include "Editor.h"
int main()
{
    try 
    {
        EditorApplication app;
        app.startup();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}