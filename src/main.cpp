#include "application.h"

int main() 
{
    get::application app;
    app.run();
    app.shutdown();

    return 0;
}
