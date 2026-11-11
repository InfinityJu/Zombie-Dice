#include "../include/setup_interface.hpp"
#include "../include/game_bones.hpp"
#include "../include/utils.hpp"
#include "../include/z_ai.hpp"

#include <vector>
#include <sstream>
#include <algorithm>

/**
 * @fn usage
 * @brief Displays the game instructions/rules or terminates the program with a custom message.
 * 
 * @details Checks the optional `msg` parameter. If it contains text, the function prints it as 
 * a "Game Over" notification and abruptly terminates execution using `std::exit(0)`. 
 * Otherwise, if `msg` is empty, it prints the standardized instructional overview of the 
 * Zombie Dice mechanics, dynamically reading the victory condition threshold (`brains_to_win`) 
 * from the provided `GameConfig` reference.
 *
 * @param cfg A reference to the current game configuration containing the target score to win.
 * @param msg An optional terminating error or game-over message string. Defaults to an empty string.
 */
void usage(GameConfig& cfg, const std::string& msg = ""){
  if (!msg.empty()) {
    std::cout << "Game Over: " << msg << "\n";
    exit(0);
  }

  std::cout << " The object of the jeopardy dice game Zombie Dice is to be the\n"
            << " first to eat " << cfg.brains_to_win << " or more human brains in a turn.\n"
            << " Each player's turn consists of repeatedly rolling 3 dice.\n"
            << " A die may produce 3 outcomes: you ate a brain, the human escaped,\n"
            << " or you were shot!\n"
            << " If you get shot 3 or more times you die and lose all brains you\n"
            << " ate in that turn; then the turn goes to the next player.\n"
            << " However, if you decide to stop before get shot 3 times, you keep\n"
            << " the brains you ate.\n\n";
}

/**
 * @fn showWellcomeScreen
 * @brief Displays the introductory welcome interface and initialization rules to the user.
 * 
 * @details Prints the game's stylized branding header, triggers the sequential 
 * printing of the gameplay instructions via the `usage` function, and guides the 
 * user on how to correctly format the upcoming player name input layout.
 *
 * @param cfg A reference to the active game configuration structure passed down to the usage overview.
 */
void showWellcomeScreen (GameConfig& cfg){
    std::string players;

    std::cout << "         ---> Welcome to the Zombi Dice game (v 0.1) <--\n"
              << "                -- copyright DIMAp/UFRN 2024-2026 --\n\n";
    
    usage(cfg);

    std::cout << " Before we begin, please enter the names of the players\n"
              << " in a single line, separated by comma. For example:\n"
              << " >>> Taylor , Ariana, Sabrina, Olivia\n";
}

/**
 * @fn showGetPlayersMsg
 * @brief Displays the validation constraints and the prompt cursor for player registration.
 * 
 * @details Prints a status message detailing the entry requirements, specifying the absolute 
 * minimum of 2 players and the dynamic upper boundary (`max_players`) retrieved from the config. 
 * It also explicitly warns that submitting an empty line acts as a hard cancellation event, 
 * terminating the game session.
 *
 * @param cfg A reference to the current game configuration containing the maximum player limits.
 */
void showGetPlayersMsg(GameConfig& cfg){
    std::cout << "\nMinimum of 2 players, maximum of " << cfg.max_players << " names.\n"
              << "Providing an empty line will end the game.\n"
              << ">>> ";
}

/**
 * @fn getPlayers
 * @brief Parses a comma-separated string to extract individual player names and build the team list.
 * 
 * @details Tokenizes the input string using a comma (`,`) delimiter. Each extracted segment is 
 * processed through a string trimming mechanism to remove leading and trailing spaces. Valid, non-empty 
 * names are then encapsulated into `PlayerInfo` structures with default metrics and appended to the final list.
 * * Validation guardrails:
 * - If the raw `input` string is completely empty, it triggers a game-over shutdown via the `usage` routine.
 * - The tokenization loop strictly honors the upper capacity boundary (`gc.max_players`) defined in the configuration.
 *
 * @param input The raw input line containing player names separated by commas.
 * @param gc The current game configuration containing structural limits and victory thresholds.
 * @param players The current players in the game if any have been inserted before
 * 
 * @return A vector of initialized `PlayerInfo` objects representing the validated participants.
 */
std::vector<PlayerInfo> getPlayers(std::string input, GameConfig gc, std::vector<PlayerInfo>& players){
    std::stringstream ss(input);
    std::string segment;

    if(input.empty()) usage(gc, "No players for the game!");

    while (std::getline(ss, segment, ',') && players.size() < gc.max_players) {
        std::string name = trim(segment);
        
        // Só adiciona ao vetor se o nome não estiver vazio
        if (!name.empty()) {
            // Cria um novo elemento de PlayerInfo com o nome e valores padrão
            PlayerInfo newplayer;
            newplayer.Name = name;

            players.push_back(newplayer);
        }
    }
    
    return players;
}

/**
 * @fn printPlayers
 * @brief Prints the list of registered players to the standard output.
 * 
 * @details Iterates through the provided vector of `PlayerInfo` objects and displays 
 * each validated player name wrapped in quotation marks. This serves as a visual 
 * confirmation frame right after the registration process concludes.
 *
 * @param players A constant reference to the vector containing the registered player data.
 */
void printPlayers(const std::vector<PlayerInfo>& players) {
    std::cout << "\n>>> The players of the game are:" << std::endl;
    
    // Percorre o vetor de jogadores
    for (const auto& player : players) {
        std::cout << "  \"" << player.Name << "\"" << std::endl;
    }
    std::cout << std::endl;
}

/**
 * @fn setupGame
 * @brief Coordinates the operational player registration phase, optional AI bot inclusion, and turn initialization.
 * 
 * @details Prompts for and processes the initial comma-separated human player names line. 
 * If the resulting team registration boundaries require or allow modification, it branches into 
 * contextual fallback states:
 * - **Single Player Input**: Informs the user of the invalid human count constraint. Offers a menu 
 * to restart the routine via recursion (`"0"`) or seamlessly switch to a solo versus AI bot match (`"1"`).
 * - **Multi-player Sub-optimal Boundary**: Allows groups below the maximum allowed player configuration 
 * threshold to append a user-specified quantity of automated opponents (`create_zombie_bot`).
 * * Any unhandled, empty, or out-of-bounds menu selections systematically invoke a critical application 
 * shutdown through the `usage` component.
 * * After structuring the dynamic player vector, it configures internal structural metrics, randomizes or 
 * handles player execution priority (`initialize_playerorder`), and outputs the starting player's name 
 * while waiting for a confirmation key stroke.
 *
 * @param loader The internal configuration engine handler context.
 * @param GameCntrl A reference to the active game controller instance to mutate, populate, and prepare for play.
 * @param players A mutable string buffer used to temporarily house the extracted user input line.
 */
void setupGame(ConfigLoader loader, GameController& GameCntrl, std::string players){
    showGetPlayersMsg(GameCntrl.Config);
    std::getline(std::cin, players);

    GameCntrl.Players = getPlayers(players, GameCntrl.Config, GameCntrl.Players);

    std::string who_to_play_against;

    if(GameCntrl.Players.size() == 1){
        std::cout << "\nThe number of players is invalid!\n"
                  << "  If you wish to add more players enter \"0\".\n"
                  << "  If you wish to play against bots enter \"1\".\n"
                  << "  Any other argument will end the game.\n"
                  << ">>> ";
        
        std::getline(std::cin, who_to_play_against);

        if(who_to_play_against == "0"){
            setupGame(loader, GameCntrl, players);
        }
        else if(who_to_play_against == "1"){
            std::string number_bots;

            std::cout << "\nHow many bots do you wish to play against?\n"
                      << ">>> ";
            
            std::getline(std::cin, number_bots);

            GameCntrl.Config.bot_count = std::stoi(number_bots);

            if (GameCntrl.Config.bot_count != 0 ){
                for (int i = 0; i < GameCntrl.Config.bot_count; i++){
                    if(GameCntrl.Players.size() < GameCntrl.Config.max_players){
                        auto bot = create_zombie_bot(GameCntrl.Board.g);
                        GameCntrl.Players.push_back(bot);
                    }
                }
            }
        }
        else {
           usage(GameCntrl.Config, "Invalid argument!"); 
        }

    } else if (GameCntrl.Players.size() < GameCntrl.Config.max_players ){
        std::string bots_or_not;

        std::cout << "\nWould you like to play against bots?\n"
                << "  Enter \"0\" for No or \"1\" for Yes.\n"
                << "  Any other argument will end the game.\n"
                << ">>> ";
        
        std::getline(std::cin, bots_or_not);

        if(bots_or_not == "1"){
            std::string number_bots;

            std::cout << "\nHow many bots do you wish to play against?\n"
                    << ">>> ";
            
            std::getline(std::cin, number_bots);

            GameCntrl.Config.bot_count = std::stoi(number_bots);

            if (GameCntrl.Config.bot_count != 0 ){
                for (int i = 0; i < GameCntrl.Config.bot_count; i++){
                    if(GameCntrl.Players.size() < GameCntrl.Config.max_players){
                        auto bot = create_zombie_bot(GameCntrl.Board.g);
                        GameCntrl.Players.push_back(bot);
                    }
                }
            }

        } else if (bots_or_not != "1" && bots_or_not != "0"){
            usage(GameCntrl.Config, "Invalid argument!");
        }

    }

    GameCntrl.PlayerNum = GameCntrl.Players.size();
    GameCntrl.PlayerOrder.resize(GameCntrl.PlayerNum);
    GameCntrl.initialize_playerorder();

    auto idx = GameCntrl.PlayerOrder[0];

    printPlayers(GameCntrl.Players);

    std::cout << ">>> The player who will start the game is \"" << GameCntrl.Players[idx].Name << "\"\n"
              << "Press <Enter> to start the match.\n" << std::endl;
}