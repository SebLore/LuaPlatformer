#include "Game/App.h"

int main()
{
    App app;

    if (!app.Initialize())
        return EXIT_FAILURE;

    return app.Run();
}
