/**
 * @file main.cpp
 * @brief Main entry point for the application. Initializes the game, runs the main loop, and handles cleanup.
 */

#include "App.h"

int main(void)
{
    try
    {
        App app;
        app.Initialize();
        return app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Unhandled exception caught in main: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught in main.\n";
        return EXIT_FAILURE;
    }
}
