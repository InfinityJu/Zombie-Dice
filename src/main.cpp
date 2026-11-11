#include "../include/setup_interface.hpp"
#include "../include/game_bones.hpp"
#include "../include/game_controller.hpp"

/**
 * @file platform_headers.hpp
 * @brief Conditional preprocessor configuration block for operating system compatibility layers.
 * 
 * @details Detects if the compilation environment targets a Microsoft Windows platform 
 * using the architecture macro `_WIN32`. When active, it safely hooks the core framework header 
 * `<windows.h>` into the scope to provide native access to console manipulation API routines, 
 * terminal window formatting, or specialized system commands.
 */
#ifdef _WIN32
#include <windows.h>
#endif


/**
 * @fn main
 * @brief Application entry point for the Zombie Dice game.
 * 
 * @details Coordinates the execution lifecycle of the game inside a replayable loop. 
 * The system handles platform-specific setups (such as enabling UTF-8 support on Windows) 
 * and processes the game through six defined steps per match session:
 * 1. **Configuration Loading**: Detects an optional command-line argument for a custom configuration 
 * path, defaulting to "zdice.ini", and loads internal rules via `ConfigLoader`.
 * 2. **Game Setup**: Reinitializes properties inside the `GameController` instance (such as target win 
 * scores), prints the greeting elements via `showWellcomeScreen`, and registers player data using `setupGame`.
 * 3. **Component Initialization**: Populates the master vector using `initialize_dicebag` and configures 
 * the shared memory arena with `reset_board`.
 * 4. **User Confirmation**: Forces a temporary execution block via `std::cin.get()`, waiting for an enter 
 * key press to officially draw the layout grid.
 * 5. **Core Gameplay Loop**: Periodically processes game state actions (`process_events`), updates the output 
 * buffer interface (`render`), and transitions active states based on user/AI choices (`update`).
 * 6. **Shutdown and Match Review**: Forces a final screen render frame to show endgame/winner metrics, 
 * followed by a prompt asking the user if they wish to start a new match.
 *
 * @param argc The count of command-line arguments delivered to the process execution context.
 * @param argv The array of null-terminated character strings representing the operational parameters.
 * 
 * @return Returns `EXIT_SUCCESS` upon a normal and complete session lifecycle termination.
 */
int main(int argc, char* argv[]) {
//Makes it possible for the user using windows to see the emojis on the terminal
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #endif

    std::string play{"Y"};

    while(play == "Y"){
        play = "";

        // Cleaning terminal for a better experience
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif

        // 1. loads the Config
        std::string configFile = "";
        if (argc > 1) configFile = argv[1];

        ConfigLoader loader;
        loader.load(configFile);

        // 2. setup (shows the wellcome screen and gets the players names)
        GameController gc;
        gc.Config = loader.config;
        gc.WinBrains = loader.config.brains_to_win;

        std::string players;
        showWellcomeScreen (loader.config);
        setupGame(loader, gc, players);

        // 3. inicializes the dicebag and the order of the player
        gc.initialize_dicebag(loader);
        gc.reset_board();

        // 4. waits for the <enter> to start the game
        std::cin.get();

        gc.render();

        // 5. game loop
        while (!gc.game_over(false)) {
            gc.process_events(loader);
            gc.render();
            gc.update();
        }

        // 6. final screen
        gc.render();

        std::cout << "\nDo you want to play again?\n"
                << "   Press Y + <end> for yes.\n"
                << "   Any other input will end the game.\n"
                << "🧟> ";

        getline(std::cin, play);
    }

    return EXIT_SUCCESS;
}