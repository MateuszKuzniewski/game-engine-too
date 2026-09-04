#include <cstdlib>
#include "application.h"

int main() 
{
    application* app = new application();
    app->run();
    app->shutdown();
    
    delete app;
    return EXIT_SUCCESS;
}
