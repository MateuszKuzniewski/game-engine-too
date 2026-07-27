#include "application.h"

int main() 
{
    get::application* app = new get::application();
    app->run();
    app->shutdown();
    
    delete app;
    return 0;
}
