#include "../include/z_ai.hpp"

/**
 * @fn bot_name_picker
 * @brief Selects a random zombie name from a predefined list to assign to an AI bot.
 * 
 * @details Utilizes a uniform integer distribution mapped across the index range of the 
 * global or accessible `ZombieNames` storage pool. The selection relies on an external 
 * Pseudo-Random Number Generator engine (`std::mt19937`) to guarantee unbiased pseudo-randomness 
 * and prevent duplicate pattern sequences across separate instances during initialization.
 *
 * @param g A reference to the active `std::mt19937` random engine used to generate the uniform distribution index.
 * 
 * @return A string containing the randomly selected zombie name.
 */
std::string bot_name_picker(std::mt19937& g){
    std::uniform_int_distribution<size_t> dist(0, ZombieNames.size() - 1);
    
    size_t randomIndex = dist(g);
    return ZombieNames[randomIndex];
}

/**
 * @fn bot_difficulty_picker
 * @brief Selects a random zombie name from a predefined list to assign to an AI bot.
 * 
 * @details Utilizes a string to define the difficulty of a bot.
 *
 * @param g A reference to the active `std::mt19937` random engine used to shuffle the string.
 * 
 * @return 'c' for coward, 'r' for reckless, 's' for smart, 'g' for gambler.
 */
char bot_difficulty_picker(std::mt19937& g){
    std::string modes{"ccccrrrsssg"};

    std::shuffle(modes.begin(), modes.end(), g);

    return modes.front();
}

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
PlayerInfo create_zombie_bot( std::mt19937& g ){
    PlayerInfo NewBot;
    
    NewBot.Name = bot_name_picker(g);
    NewBot.bot = true;
    if (NewBot.Name == "Rogerio"){
        NewBot.botdifficulty = 's';
    } else {
        NewBot.botdifficulty = bot_difficulty_picker(g);
    }

    return NewBot;
}

/**
 * @fn bot_bout_to_lose
 * @brief Assesses whether the bot is currently tracking behind the active lead win condition thresholds.
 * 
 * @details Evaluates the bot's potential score structure (the sum of its banked permanent brains 
 * and the unbanked turn-buffer brains currently held on the board) against the victory parameters. 
 * If another player has already crossed the target threshold (`WinPlayers > 0`) and the bot's maximum 
 * achievable score for this sequence remains strictly less than the victory margin (`WinBrains`), 
 * it flags a high-risk scenario indicating the bot is about to lose unless aggressive risk matches are taken.
 *
 * @param Board The active game board state containing the unbanked turn brain accumulators.
 * @param BotInfo The data profile tracking the bot's globally accumulated metrics.
 * @param WinPlayers The active counter tracking how many participants have already crossed the win threshold.
 * @param WinBrains The target brain score limit required to trigger end-game conditions.
 * 
 * @return True if another player is in a winning position and the bot cannot match or beat the target 
 * score using its current turn assets; false otherwise.
 */
bool bot_bout_to_lose( GameBoard Board, PlayerInfo BotInfo, count_t WinPlayers, count_t WinBrains ){
    if (WinPlayers == 0) return false;

    auto BotBrains = Board.total_brains() + BotInfo.Brains;

    return BotBrains < WinBrains;
}

/**
 * @fn bot_bout_to_win
 * @brief Determines if the bot's potential total score meets or exceeds the victory condition threshold.
 * 
 * @details Evaluates the bot's temporary standing by calculating the sum of its permanently banked brains 
 * (`BotInfo.Brains`) and the unbanked brains collected during its ongoing turn sequence (`Board.total_brains()`). 
 * If this projected tally satisfies the global match target (`WinBrains`), the function returns true, signaling 
 * that the bot is safe to stop rolling and secure the win.
 *
 * @param Board The active game board state hosting the current turn's unbanked brain buffer.
 * @param BotInfo The data structure tracking the bot's permanently accumulated scores and attributes.
 * @param WinPlayers The active tracker counting players who have qualified for victory conditions.
 * @param WinBrains The mandatory point threshold needed to secure a win in the match.
 * @param PlayerNum The number of total players at the begining of the match;
 * @param PlayersPlaying The number of players currently in the match;
 * 
 * @return True if the bot's combined current turn and banked scores match or surpass the victory cap; false otherwise.
 */
bool bot_bout_to_win( GameBoard Board, PlayerInfo BotInfo, count_t WinPlayers, count_t WinBrains,
                    count_t PlayerNum, count_t PlayersPlaying ){
    auto BotBrains = Board.total_brains() + BotInfo.Brains;

    if (WinPlayers > 0 && WinPlayers >= PlayersPlaying - 1){
        return BotBrains > WinBrains;
    } else {
        return BotBrains >= WinBrains;
    }
}

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
 * @param BotInfo The data structure tracking the bot's permanently accumulated scores and profile tags.
 * @param WinPlayers The active tracker counting players who have already qualified for victory.
 * @param WinBrains The mandatory point threshold needed to secure a win in the match.
 * @param PlayerNum The number of total players at the begining of the match;
 * @param PlayersPlaying The number of players currently in the match;
 * 
 * @return A string representing the bot's action: `""` to roll again, or `"H"` to hold and bank its points.
 */
std::string bot_processing( GameBoard Board, PlayerInfo BotInfo, count_t WinPlayers, count_t WinBrains,
                            count_t PlayerNum, count_t PlayersPlaying ){
    if (bot_bout_to_lose(Board, BotInfo, WinPlayers, WinBrains)){
        return "";
    }

    if (bot_bout_to_win(Board, BotInfo, WinPlayers, WinBrains, PlayerNum, PlayersPlaying)){
        return "H";
    }

    auto ShotsTaken = Board.Shots.size();
    auto BrainsToWin = WinBrains - Board.total_brains();

    if (ShotsTaken == 0 || Board.cur_Brains.size() == 0){
        return "";
    } 

    if (BotInfo.botdifficulty == 's'){
        if (ShotsTaken == 1){
            if (BrainsToWin < (WinBrains * 0.66) || BrainsToWin < 3 || Board.total_brains() < 6){
                return "";
            } else {
                return "H";
            }
        } else {
            if (Board.cur_Brains.size() < 3) return "";
            return "H";
        }
    } else if (BotInfo.botdifficulty == 'c'){
        if (ShotsTaken >= 1 || Board.total_brains() >=3){
            return "H";
        }
    } else if (BotInfo.botdifficulty == 'r'){
        if (ShotsTaken == 1 || Board.total_brains() > (BrainsToWin * 0.66)){
            if (BotInfo.riskyrolls < 3){
                BotInfo.riskyrolls++;
                return "";
            } else {
                return "H";
            }
        } else {
            if (BotInfo.riskyrolls < 2 || Board.total_brains() > (WinBrains * 0.5)){
                BotInfo.riskyrolls++;
                return "";
            }
            return "H";
        }
    } else if (BotInfo.botdifficulty == 'g'){
        std::uniform_int_distribution<int> coin_flip(0, 1);

        if (coin_flip(Board.g) == 1){
            return "";
        } else {
            return "H";
        }
    }
    return "";
}