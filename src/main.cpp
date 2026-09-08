#include <cstdlib>
#include "application.h"

int main() 
{
    application* app = new application();
    app->load_data();
    app->run();
    
    delete app;
    return EXIT_SUCCESS;
}
