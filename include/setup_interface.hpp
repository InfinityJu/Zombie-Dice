#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <iostream>
#include <string>

#include "game_bones.hpp"

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
void showWellcomeScreen (GameConfig& cfg);

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
std::vector<PlayerInfo> getPlayers(std::string input, GameConfig gc, std::vector<PlayerInfo>& players);

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
void printPlayers(const std::vector<PlayerInfo>& players);

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
void setupGame(ConfigLoader loader, GameController& GameCntrl, std::string players);

#endif