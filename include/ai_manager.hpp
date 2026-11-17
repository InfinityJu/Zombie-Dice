#include "../include/z_ai.hpp"
#include "../include/ai_tree.hpp"

/**
 * @fn bot_processing
 * @brief Evaluates the AI bot's tactical state to determine whether it should roll again or hold.
 * 
 * @details Executes a multi-layered rule engine to direct the automated participant's behavior:
 * - **Imminent Loss Check**: If `bot_bout_to_lose` evaluates to true, the bot ignores accumulated 
 * damage and forces a roll by returning an empty command (`""`).
 * - **Imminent Win Check**: If `bot_bout_to_win` evaluates to true, the bot immediately halts 
 * and secures the victory by returning the hold token (`"H"`).
 * - **General Damage-Based Risk Assessment**:
 * - **0 Shots Taken** or **0 Brains obtained**: Always pushes to roll (`""`).
 * - **Difficulty-Based Risk Assessment**:
 * - - **Smart Bot**:
 * - - **1 Shot Taken**: Evaluates remaining margins. Stays aggressive and rolls if the distance to victory 
 * is short, or if current turn points are beneath standard mathematical safety thresholds; otherwise, holds (`"H"`).
 * - - **2 Shots Taken**: Extreme danger mode. Will aggressively risk a final roll only if it has collected 
 * fewer than 3 brains during the turn buffer; otherwise, plays safe and holds (`"H"`).
 * - - **Coward Bot**:
 * - - **1 or More Shots Taken**: Evaluates nothing, immediately holds;
 * - - **Reckless Bot**:
 * - - Uses a "risky roll" counter to check how many times it has risked itself based on how many shots it has
 * taken and how many brains it has collected.
 * - - **Gambler Bot**:
 * - - Complete gamble. If it has collected at least one brain and collected at least one shot it will decide
 * it's next action through a coinflip.
 *
 * @param Board The active game board state containing current turn metrics (shots and brains buffers).
 * @param BotData The data structure tracking the bot's permanently accumulated scores and profile tags.
 * @param WinPlayers The active tracker counting players who have already qualified for victory.
 * @param WinBrains The mandatory point threshold needed to secure a win in the match.
 * @param PlayerNum The number of total players at the begining of the match;
 * @param PlayersPlaying The number of players currently in the match;
 * 
 * @return A string representing the bot's action: `""` to roll again, or `"H"` to hold and bank its points.
 */
std::string bot_processing( GameBoard Board, BotInfo& BotData, count_t WinPlayers, count_t WinBrains,
                            count_t PlayerNum, count_t PlayersPlaying );

/**
 * @fn create_zombie_bot
 * @brief Instantiates and configures a new AI bot participant for the game match.
 * 
 * @details Allocates a fresh `PlayerInfo` structure, marks its internal controller flags 
 * to establish it as an automated entity (`bot = true`), and assigns a random identifying moniker 
 * by invoking the `bot_name_picker` utility engine.
 *
 * @param g A reference to the uniform pseudo-random number generator used to pick the bot's name.
 * 
 * @return An initialized `PlayerInfo` struct representing the newly generated bot opponent.
 */
std::unique_ptr<PlayerInfo> create_zombie_bot( std::mt19937& g );