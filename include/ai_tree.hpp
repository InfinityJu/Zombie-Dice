#include "../include/z_ai.hpp"
#include "../include/decision_tree.hpp"


/**
 * @brief Context structure holding the necessary environment and state 
 * for the decision tree evaluation.
 */
struct BotContext {
    GameBoard& board;
    BotInfo& botData;
    count_t winPlayers;
    count_t winBrains;
    count_t playerNum;
    count_t playersPlaying;
};

using BotNodePtr = pdt::NodePtr<BotContext, std::string>;

/**
 * @class TreeBotInfo
 * @brief Extends PlayerInfo and stores individual bot information and statistics, thinks using decision trees.
 */
class TreeBotInfo : public BotInfo{
public:
    bool is_tree_bot() const { return true; }
};


// --- Bot Creation Functions -- //

/**
 * @fn bot_tree_creator
 * @brief Creates a decision tree based on the bot's personality
 *
 * @param botdifficulty The bot's personality.
 * 
 * @return A pointer to the first node of said tree.
 */
BotNodePtr bot_tree_creator( const BotDifficulty& botdifficulty );

/**
 * @fn create_tree_bot
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
std::unique_ptr<PlayerInfo> create_tree_bot( std::mt19937& g );


// --- Methods -- //

BotContext context_extractor(GameBoard Board, BotInfo& BotData, count_t WinPlayers, count_t WinBrains,
                            count_t PlayerNum, count_t PlayersPlaying);

BotNodePtr& get_global_bot_tree();

std::string tree_processing( BotContext& ctx );

// --- Tree makers -- //

BotNodePtr create_coward_tree();
BotNodePtr create_gambler_tree();
BotNodePtr create_reckless_tree();
BotNodePtr create_smart_tree();
BotNodePtr bot_tree_creator();