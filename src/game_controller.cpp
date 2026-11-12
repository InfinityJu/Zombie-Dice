#include "../include/game_bones.hpp"
#include "../include/setup_interface.hpp"
#include "../include/z_ai.hpp"

#include <algorithm>
#include <random>
#include <sstream>

/**
 * @fn printDiceFaces
 * @brief Retrieves the string representation of faces for a given die type based on configurations.
 * 
 * @details Evaluates the provided `Dice` category type and extracts its corresponding layout 
 * string mapping from the active configuration rules container.
 * 
 * @param cur_dice The specific category or difficulty of the die to query (weak, strong, or tough).
 * 
 * @param gcnf The active configuration loader instance holding current face layouts.
 * 
 * @return A string containing the characters representing each face of the die, 
 * or an empty string if the type is unknown.
 */
std::string printDiceFaces(Dice cur_dice, ConfigLoader gcnf) {
    if (cur_dice == Dice::weak)   return gcnf.config.weak_die_faces;
    if (cur_dice == Dice::strong) return gcnf.config.strong_die_faces;
    if (cur_dice == Dice::tough)  return gcnf.config.tough_die_faces;
    return ""; 
}

/**
 * @fn dice_color
 * @brief Retrieves a visual color representation string for a specific die type.
 * 
 * @details Maps each `Dice` difficulty tier to a matching colored square emoji 
 * (Green for weak, Yellow for strong, Red for tough) to assist with user interface rendering.
 *
 * @param d The specific category or difficulty tier of the die.
 * 
 * @return A string containing a colored square emoji representing the die type, 
 * or "?" if the tier is unrecognized.
 */
std::string dice_color(Dice d) {
    if (d == Dice::weak)   return "🟩";
    if (d == Dice::strong) return "🟨";
    if (d == Dice::tough)  return "🟥";
    return "?";
}

/**
 * @fn DiceFaces
 * 
 * @brief Retrieves the face configuration string for a specified die category.
 * 
 * @details Checks the given `Dice` type and returns the matching string pattern 
 * from the game configurations wrapper.
 *
 * @param cur_dice The specific category or difficulty tier of the die.
 * 
 * @param gcnf The configuration loader instance containing the active rule properties.
 * 
 * @return A string with the character-mapped layout of the die faces, 
 * or a single space string " " if the type is invalid.
 */
std::string DiceFaces(Dice cur_dice, ConfigLoader gcnf){
    if (cur_dice == Dice::weak){
        return gcnf.config.weak_die_faces;
    } else if (cur_dice == Dice::strong){
        return gcnf.config.strong_die_faces;
    } else if (cur_dice == Dice::tough){
        return gcnf.config.tough_die_faces;
    }
    return " "; 
}

// --- Game Controller Functions --- //

/**
 * @fn GameController::initialize_dicebag
 * @brief Populates the main dice bag container with the configured number of dice for each difficulty tier.
 * 
 * @details Reads the counts for weak, strong, and tough dice from the provided configuration loader 
 * instance and pushes the corresponding `Dice` enum tokens into the `DiceBag` vector.
 *
 * @param gcnf The loaded configuration container containing the official dice pool counts.
 */
void GameController::initialize_dicebag( ConfigLoader gcnf ){
    for (int i = 0; i < gcnf.config.weak_dice_count; i++){
        DiceBag.push_back(Dice::weak);
    }

    for (int i = 0; i < gcnf.config.strong_dice_count; i++){
        DiceBag.push_back(Dice::strong);
    }

    for (int i = 0; i < gcnf.config.tough_dice_count; i++){
        DiceBag.push_back(Dice::tough);
    }
}

/**
 * @fn GameController::initialize_playerorder
 * @brief Sets up and randomizes the turn sequence for all registered players.
 * 
 * @details Populates the `PlayerOrder` vector with sequential indices corresponding to 
 * the total number of players in the session, and then shuffles them using the 
 * pseudo-random number generator hosted within the `GameBoard` instance.
 * 
 */
void GameController::initialize_playerorder(){
    for (int i = 0; i < Players.size(); i++){
        PlayerOrder[i] = i;
    }

    std::shuffle(PlayerOrder.begin(), PlayerOrder.end(), Board.g);
}

/**
 * @fn GameController::reset_board
 * @brief Resets the board state to prepare for a new turn or player session.
 * 
 * @details Reinitializes the active dice bag with the master list, clears out any 
 * dice remaining on hand, blanks the results array to exactly three empty spaces, 
 * and flushes all accumulated shot counters, brain trackers, and score modifiers.
 */
void GameController::reset_board(){
    Board.cur_DiceBag = DiceBag;
    Board.DiceOnHand.clear();
    Board.DiceResults.assign(3, ' ');
    Board.Shots.clear();
    Board.cur_Brains.clear();
    Board.extraBrains = 0;
};


/**
 * @fn GameController::advance_player
 * @brief Advances the game turn to the next player, updating scores and win conditions.
 * 
 * @details Increments the active player's turn counter and, if they survived the round 
 * (i.e., did not die), flushes and saves their accumulated brains. It then evaluates 
 * potential changes to the winning threshold, check for round conclusions, and handles 
 * transitions into tie-breaking phases or the next global game cycle.
 *
 * @param dead Flag indicating whether the player lost their accumulated turn score due to a penalty (defaults to false).
 */
void GameController::advance_player( bool dead = false ){
    int idx = currplayer_idx();
    Players[idx]->turns++;

    if(idx == 0  && Players[idx]->turns == 0){
        Players[idx]->turns++;
    }

    if (!dead){
        Players[idx]->Brains += Board.total_brains();
    }

    if (Players[idx]->is_bot()) {
        BotInfo* bot = dynamic_cast<BotInfo*>(Players[idx].get());
        if (bot && bot->riskyrolls > 0) {
            bot->riskyrolls = 0;
        }
    }
    
    if (Players[idx]->Brains == WinBrains){
        WinPlayers++;
    } else if (Players[idx]->Brains > WinBrains){
        WinBrains = Players[idx]->Brains;
        WinPlayers = 1;
    }

    if (CurrPlayer == PlayerOrder.size() - 1){
        if (!win()) Turn++;
        CurrPlayer = 0;
        tie_checker();
    } else {
        CurrPlayer++;
    }
}

/**
 * @fn GameController::currplayer_idx
 * @brief Retrieves the actual internal index of the currently active player.
 * 
 * @details Maps the sequential `CurrPlayer` turn counter to the real player index 
 * stored inside the shuffled `PlayerOrder` vector.
 * 
 * @return The internal index of the current player within the Players vector.
 */
count_t GameController::currplayer_idx() const {
    return PlayerOrder[CurrPlayer];
}

/**
 * @fn GameController::currplayer_name
 * @brief Retrieves the name of the player who is currently taking their turn.
 * 
 * @return A string containing the current player's name.
 */
std::string GameController::currplayer_name() {
    return Players[currplayer_idx()]->Name;
}

/**
 * @fn GameController::active_players
 * @brief Returns the total number of players currently participating in the turn cycle.
 * 
 * @return The size of the player order sequence.
 */
count_t GameController::active_players() {
    return PlayerOrder.size();
}

/**
 * @fn GameController::dice_in_bag
 * @brief Checks the amount of dice remaining inside the board's active bag.
 * 
 * @return The number of available dice left to be drawn in this turn.
 */
count_t GameController::dice_in_bag() const {
    return Board.cur_DiceBag.size();
}

/**
 * @fn GameController::rolled_brains
 * @brief Gets the total number of brains rolled by the current player in this turn.
 * 
 * @return The current round's rolled brains count.
 */
count_t GameController::rolled_brains() const{
    return Board.brains_rolled();
}

/**
 * @fn GameController::rolled_shots
 * @brief Gets the total number of shots accumulated by the current player in this turn.
 * 
 * @return The current round's rolled shots count.
 */
count_t GameController::rolled_shots() const{
    return Board.shots_rolled();
}

/**
 * @fn GameController::tie_checker
 * @brief Evaluates the game state at the end of a round to handle potential ties or match victories.
 * 
 * @details Checks if multiple players have crossed the winning threshold simultaneously to transition 
 * the game into a `Tie` stage, or switches directly to the `End` stage if a clear single winner exists.
 */
void GameController::tie_checker(){
    if (WinPlayers > 1){
        CurrStage = GameStage::Tie;
    } else if ( win () ) {
        CurrStage = GameStage::End;
    }
}

/**
 * @fn GameController::game_over
 * @brief Checks if all conditions are met to officially terminate the current match.
 * 
 * @details Evaluates if the loop has explicitly reached the End stage, or validates fairness 
 * by ensuring that if a single player crossed the winning threshold, all other active 
 * participants have completed an equal number of turns for the current round before stopping.
 *
 * @param win Optional flag indicating a standard win evaluation context (defaults to false).
 * 
 * @return True if the game is officially over and should terminate, false otherwise.
 */
bool GameController::game_over( bool win = false ){
    if (CurrStage == GameStage::End) return true;

    if (WinPlayers != 1) return false;

    for (int i = 0; i < PlayerOrder.size(); i++){
        auto idx = PlayerOrder[i];

        if (Players[idx]->turns < Turn){
            return false;
        }
    }

    return true;
}

/**
 * @fn GameController::win
 * @brief Evaluates whether the game has reached a conclusive winning state.
 * 
 * @details Directs the call to the core `game_over` logic with the context parameter 
 * set to true to verify if the overall win and turn conditions have been satisfied.
 *
 * @return True if a final winner has been determined and the match can end, false otherwise.
 */
bool GameController::win(){
    return game_over( true );
}

// --- Main Engine Functions --- //

/**
 * @fn GameController::process_events
 * @brief Processes game events and actions depending on the active game stage.
 * 
 * @details Evaluates the current state of `CurrStage` and executes the corresponding logic:
 * - **Play**: Commits the active turn dice by moving them to the scored/shot sections.
 * - **Roll**: Automatically handles drawing missing dice from the bag and rolling them.
 * - **Tie**: Filters the `PlayerOrder` container to remove any players who did not reach 
 * the current maximum winning score threshold (`WinBrains`), leaving only the tied final contenders.
 * - **End**: No actions are performed as the match has concluded.
 *
 * @param gcnf The loaded configuration container passed down to handle dice face mappings during a roll.
 */
void GameController::process_events( ConfigLoader gcnf ){
    //play -> move dice to shotguns and brains sections
    if (CurrStage == GameStage::Play){
        Board.move_dice(false);
    }

    //roll -> draw and roll the dice
    else if (CurrStage == GameStage::Roll){
        Board.draw_dice();
        Board.roll_dice( gcnf );
    }

    //tie -> remove the index of the players that aren't in the tie from playerorder
    else if (CurrStage == GameStage::Tie){
        auto runner = PlayerOrder.begin();

        while (runner != PlayerOrder.end()){
            auto idx = *runner;

            if (Players[idx]->Brains < WinBrains){
                runner = PlayerOrder.erase(runner);
            } else {
                runner++;
            }
        }
    }

    //end -> used only in render, doesn't do anything here
}

/**
 * @fn GameController::update
 * @brief Drives the state machine logic transitions and handles player/bot input for the current frame.
 * 
 * @details Captures input from the standard input stream and processes state updates based on `CurrStage`:
 * - **Play Stage**:
 * - Instantly penalizes and advances the turn if the active player has accumulated 3 or more shots.
 * - Automatically delegates decision-making to `bot_processing` if the current player is an AI.
 * - Evaluates player choices: 
 * - *Empty input*: Advances the stage to `Roll` to roll the dice.
 * - *'H' / 'h' (Hold)*: Saves points via `advance_player` and clears the table with `reset_board`.
 * - *'Q' / 'q' (Quit)*: Forces the game loop transition directly into the `End` stage.
 * - **Roll Stage**: Transition automatically resets back to the `Play` stage after calculations are applied.
 * - **Tie Stage**: Flags adjustments and immediately routes back into the `Play` cycle for tie-breaking rolls.
 * - **End Stage**: Halts updates as the session configuration lifecycle has finished.
 */
void GameController::update(){
    std::string op;
    std::getline(std::cin, op);

    //play -> used to manage the players actions
    if (CurrStage == GameStage::Play){
        if (Board.Shots.size() >= 3){
            advance_player( true );
            reset_board();

            return;
        }

        auto idx = PlayerOrder[CurrPlayer];

        if (Players[idx]->is_bot() && op != "q" && op != "Q") {
            BotInfo* bot = dynamic_cast<BotInfo*>(Players[idx].get());
            if (bot) {
                op = bot_processing( Board, *bot, WinPlayers, WinBrains, PlayerNum, PlayerOrder.size() );
            }
        }

        if (op.empty()){
            CurrStage = GameStage::Roll;
        } else if (op == "h" || op == "H"){
            advance_player();
            reset_board();
        } else if (op == "q" || op == "Q"){
            CurrStage = GameStage::ConfirmEnd;
        }
        else if(op == "-h" || op == "--help"){
            CurrStage = GameStage::Help;
        }
    }

    //roll -> goes back to play
    else if (CurrStage == GameStage::Roll){
        CurrStage = GameStage::Play;
    }
    

    //tie -> used for render, then returns to play
    else if (CurrStage == GameStage::Tie){
        CurrStage = GameStage::Play;
    }

    //help -> used for render, then returns to play
    else if(CurrStage == GameStage::Help){
        if(op == "") CurrStage = GameStage::Play;
    }

    else if(CurrStage == GameStage::ConfirmEnd){
        if(op == "r" | op == "R") CurrStage = GameStage::Play;
        else if(op == "") CurrStage = GameStage::End;
    }

    //end -> used for render, doesn't do anything in this function
}

// --- GameBoard functions --- //

/**
 * @fn GameBoard::draw_dice
 * @brief Draws dice from the active bag until the player has exactly three dice on hand.
 * 
 * @details Checks the current size of `DiceOnHand`. If it is already full (three dice), the 
 * function returns immediately. Otherwise, it shuffles the active `cur_DiceBag` and extracts 
 * the required number of dice. If the bag runs completely empty mid-draw, it dynamically 
 * calls `move_dice(true)` to recycle spent dice back into the bag before continuing the draw.
 */
void GameBoard::draw_dice(){
    if (DiceOnHand.size() == 3){
        return;
    }

    std::shuffle(cur_DiceBag.begin(), cur_DiceBag.end(), g);

    for (int i = DiceOnHand.size(); i < 3; i++){
        if (cur_DiceBag.empty()){
            move_dice( true );
            std::shuffle(cur_DiceBag.begin(), cur_DiceBag.end(), g);
        }
        DiceOnHand.push_back(cur_DiceBag.back());
        cur_DiceBag.pop_back();
    }
}

/**
 * @fn GameBoard::roll_dice
 * @brief Simulates rolling the three dice currently held in the player's hand.
 * 
 * @details Iterates through the `DiceOnHand` vector, fetches the configured layout string 
 * of faces for each die type via `DiceFaces`, and randomizes the outcome by shuffling 
 * the face characters. The topmost character (`faces.front()`) is then saved as the 
 * final roll result in the corresponding index of `DiceResults`.
 *
 * @param gcnf The loaded configuration container holding individual die face layout rules.
 */
void GameBoard::roll_dice( ConfigLoader gcnf ){
    for (int i = 0; i < 3; i++){
        std::string faces = DiceFaces(DiceOnHand[i], gcnf);
        std::shuffle(faces.begin(), faces.end(), g);

        DiceResults[i] = faces.front();
    }
}

/**
 * @fn GameBoard::move_dice
 * @brief Handles the routing and sorting of dice between the player's hand, scoring pools, and the bag.
 * 
 * @details Performs two distinct operations based on the `reverse` flag:
 * - **Standard Mode (`reverse = false`)**: Iterates through the current `DiceOnHand` and checks their 
 * corresponding roll outcomes in `DiceResults`. If a die resulted in a brain ('b') or a shot ('s'), 
 * it is transferred to its respective scoring vector (`cur_Brains` or `Shots`) and removed from the hand. 
 * Dice resulting in a footprint ('f') remain in the player's hand for the next roll.
 * - **Reverse Mode (`reverse = true`)**: Used for recycling when the bag runs dry. It archives the current 
 * turn's brain counts into `extraBrains`, then safely moves all spent brain dice from `cur_Brains` 
 * back into `cur_DiceBag` using move semantics.
 *
 * @param reverse Boolean flag that toggles between scoring sorting (false) and bag recycling (true).
 */
void GameBoard::move_dice( bool reverse = false ){
    if ( reverse ){
        extraBrains += cur_Brains.size();
        cur_DiceBag = std::move(cur_Brains);
        return;
    }

    auto runner = DiceOnHand.begin();
    int i = 0;

    while(runner != DiceOnHand.end() && i < 3){
        if (DiceResults[i] == 'b'){
            cur_Brains.push_back(*runner);
            runner = DiceOnHand.erase(runner);
            DiceResults[i] = 'f';
            i++;
        } else if (DiceResults[i] == 's'){
            Shots.push_back(*runner);
            runner = DiceOnHand.erase(runner);
            DiceResults[i] = 'f';
            i++;
        } else if (DiceResults[i] == 'f'){
            runner++;
            i++;
        }
    }
}

/**
 * @fn GameBoard::total_brains
 * @brief Calculates the total number of brains collected by the player during their entire turn.
 * 
 * @details Sums the active brains currently stored in the `cur_Brains` container with any 
 * brains previously flushed into `extraBrains` due to bag recycling events.
 * 
 * @return The total accumulated brain score for the current turn.
 */
count_t GameBoard::total_brains() const {
    return cur_Brains.size() + extraBrains;
}

/**
 * @fn GameBoard::brains_rolled
 * @brief Counts how many brains were obtained specifically in the last dice roll.
 * 
 * @details Iterates through the three slots of `DiceResults` to check for the 'b' character token.
 * 
 * @return The number of brains rolled in the immediate frame.
 */
count_t GameBoard::brains_rolled() const {
    count_t amount{0};
    for (int i = 0; i < 3; i++){
        if (DiceResults[i] == 'b'){
            amount++;
        }
    }

    return amount;
}

/**
 * @fn GameBoard::shots_rolled
 * @brief Counts how many shotgun blasts were obtained specifically in the last dice roll.
 * 
 * @details Iterates through the three slots of `DiceResults` to check for the 's' character token.
 * 
 * @return The number of shots rolled in the immediate frame.
 */
count_t GameBoard::shots_rolled() const {
    count_t amount{0};
    for (int i = 0; i < 3; i++){
        if (DiceResults[i] == 's'){
            amount++;
        }
    }

    return amount;
}