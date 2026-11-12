#include "../include/game_bones.hpp"
#include "../include/game_controller.hpp"

#include <vector>
#include <iostream>
#include <string>
#include <iomanip>

// ── Helpers ─────────────────────────────────────────────────────────────

/**
 * @fn result_emoji
 * @brief Converts a dice roll outcome character into its corresponding graphical emoji.
 * 
 * @details Maps single-character identifiers representing game mechanics to visual symbols:
 * - 'b' mapped to a Brain emoji (🧠) representing points.
 * - 's' mapped to an Explosion emoji (💥) representing a shotgun blast.
 * - 'f' mapped to Footprints emoji (👣) representing fleeing/running targets.
 *
 * @param r The result character token ('b', 's', or 'f') obtained from rolling a die.
 * @return A string containing the respective emoji representation, or a single space if unrecognized.
 */
std::string result_emoji(char r) {
    if (r == 'b') return "🧠";
    if (r == 's') return "💥";
    if (r == 'f') return "👣";
    return " ";
}

/**
 * @fn get_winner
 * @brief Identifies and retrieves the name of the player who won the match.
 * 
 * @details Iterates through the list of players inside the game controller to find 
 * who met or exceeded the required target threshold (`WinBrains`).
 *  
 * @note If multiple players cross the threshold in the same turn cycle, this implementation 
 * as written will return the name of the last player in the vector who qualified.
 *
 * @param gc The active game controller instance hosting the current session data and scores.
 * 
 * @return A string containing the name of the winning player, or an empty string if no winner is found.
 */
std::string get_winner(const GameController& gc){
    std::string winner;
    for (const auto& p : gc.Players) {
        if (p->Brains >= gc.WinBrains) {
            winner = p->Name;        
        }
    }

    return winner;
}
/**
 * @fn getNamesWidth
 * @brief Calculates the length of the longest player name in the current session.
 * 
 * @details Iterates through the collection of players inside the game controller to find the 
 * maximum character count among their names. This value is typically used to dynamically align 
 * columns and format text borders during interface rendering.
 *
 * @param gc A reference to the active game controller instance hosting the player collection.
 * 
 * @return The character length of the longest player name as a `count_t`.
 */
count_t getNamesWidth(const GameController& gc) {
    count_t longest {0};

    for (const auto& p : gc.Players)
        if (p->Name.size() > longest) longest = p->Name.size();
    return longest;
}

/**
 * @brief Generates a horizontal divider line for tables.
 *
 * This function creates and returns a string containing the "─" line character
 * repeated a specified number of times. It utilizes memory pre-allocation 
 * (reserve) to optimize loop performance.
 *
 * @param n The number of times the line character should be repeated.
 * 
 * @return std::string The generated line with a length corresponding to 'n'.
 */
std::string msgTableLine(count_t n) {
    std::string sub_line = "─"; 
    std::string line;
    
    line.reserve(sub_line.size() * n);
    for (int i = 0; i < n; ++i) {
        line += sub_line;
    }
    return line;
}


/**
 * @fn print_game_instructions
 * @brief Prints a beautifully formatted ASCII instruction panel for the Zombie Dice game.
 * 
 * @details Renders a structured layout containing structural mechanics, dice outcomes, 
 * victory requirements, and risk management parameters inside a bounded box.
 */
void print_game_instructions(GameConfig Config) {
    std::cout << "┌────────────────────────────────────────────────────────┐\n";
    std::cout << "│                   HOW TO PLAY ZOMBIE DICE              │\n";
    std::cout << "├────────────────────────────────────────────────────────┤\n";
    std::cout << "│ OBJECTIVE:                                             │\n";
    std::cout << "│  Be the first zombie to eat ";

    // Adjusting the table consindering the size of the string
    if (Config.brains_to_win < 10) {
        std::cout << Config.brains_to_win << " or more human brains!    │\n";
    } else std::cout << Config.brains_to_win << " or more human brains!   │\n";


    std::cout << "│                                                        │\n";
    std::cout << "│ GAMEPLAY:                                              │\n";
    std::cout << "│  1. On your turn, you will shake and roll 3 dice.      │\n";
    std::cout << "│  2. Each die has three possible outcomes:              │\n";
    std::cout << "│     🧠 BRAIN   - You ate it! (Points to bank)          │\n";
    std::cout << "│     👣 FOOT    - The human ran away. (Roll them again) │\n";
    std::cout << "│     💥 SHOTGUN - You got hit!                          │\n";
    std::cout << "│                                                        │\n";
    std::cout << "│ RISK & REWARD:                                         │\n";
    std::cout << "│  * You can keep rolling as long as you want to gather  │\n";
    std::cout << "│    more brains.                                        │\n";
    std::cout << "│  * If you choose to HOLD (H), you secure your current  │\n";
    std::cout << "│    turn's brains into your permanent score.            │\n";
    std::cout << "│  * CRITICAL: If you collect 3 SHOTGUNS (💥) in a turn, │\n";
    std::cout << "│    you DIE, lose all brains from this turn, and your   │\n";
    std::cout << "│    turn ends immediately!                              │\n";
    std::cout << "├────────────────────────────────────────────────────────┤\n";
    std::cout << "│            Press <Enter> to return to game...          │\n";
    std::cout << "└────────────────────────────────────────────────────────┘\n";
}

// ── Título ───────────────────────────────────────────────────────────────────

/**
 * @fn render_title
 * @brief Prints the game's stylized title banner to the standard output.
 * 
 * @details Outputs a formatted header text complete with decorative zombie emojis 
 * to establish the game's visual theme at the top of the interface.
 */
void render_title() {
    std::cout << "    →[🧟] Zombie Dice Delux, v 0.1 [🧟]←\n\n";
}

// ── Scoreboard ───────────────────────────────────────────────────────────────

/**
 * @fn render_scoreboard
 * @brief Renders the visual scoreboard with all players' current status, brains collected, and turn metrics.
 * 
 * @details Generates an aligned text interface dynamically scaled according to the configuration settings. 
 * Branches the rendering layout based on the active game state:
 * - **Standard Gameplay (`CurrStage != GameStage::End`)**: Prefixes the active player with an arrow indicator 
 * (`>`) and pads names to match alignment widths using a dynamically computed column padding (`COL_NAME`).
 * - **End Game (`CurrStage == GameStage::End`)**: Highlights the matching winner with a trophy emoji (`🏆`) 
 * instead of turn cursors.
 * - **Help Stage (`CurrStage == GameStage::Help`)**: Completely bypasses score drawing and forwards the flow 
 * to render instructions via `print_game_instructions`.
 * * In both active score states, brain progression is rendered horizontally using a track of brain icons (`🧠`) 
 * and placeholders (`🔸`) up to the victory cap threshold (`SCORE_DOTS`).
 *
 * @param gc A reference to the active game controller instance containing scores, configurations, and state.
 */
void render_scoreboard(const GameController& gc) {
    if(gc.CurrStage != GameStage::Help){
        std::cout << "┌───────────────────────┐\n";
        std::cout << "│      Global Score     │\n";
        std::cout << "└───────────────────────┘\n";

        std::string winner = get_winner(gc);

        count_t SCORE_DOTS = gc.Config.brains_to_win;
        count_t COL_NAME = getNamesWidth(gc) + 1;
        if (COL_NAME < 10) COL_NAME = 10; // align with longest name


        if(gc.CurrStage == GameStage::End){
            for (size_t i = 0; i < gc.Players.size(); i++) {
                const auto& p  = gc.Players[i];

                std::cout << (p->Name == winner ? "🏆 " : "   ");
        
                std::cout << std::left << std::setw(getNamesWidth(gc)) << p->Name << ": ";

                for (count_t d = 0; d < SCORE_DOTS; d++)
                    std::cout << (d < p->Brains ? "🧠 " : "🔸 ");
        
                std::cout << "│ (" << p->Brains << "), # turns played: " << p->turns << "\n";
            }

        } else{
            for (size_t i = 0; i < gc.Players.size(); i++) {
                const auto& p  = gc.Players[i];
                bool is_cur = ((count_t)i == gc.currplayer_idx());

                // Indicates who is playing
                std::cout << (is_cur ? "> " : "  ");
                std::cout << std::left << std::setw(COL_NAME) << (p->Name + ":");
                std::cout << " ";
            
                for (count_t d = 0; d < SCORE_DOTS; d++)
                    std::cout << (d < p->Brains ? "🧠 " : "🔸 ");
                
                std::cout << "│ (" << p->Brains << "), # turns played: " << p->turns << "\n";
            }
        }

        std::cout << "\n";
    } else {
        print_game_instructions(gc.Config);
    }
}

// ── Seção 1: info da rodada ───────────────────────────────────────────────────

/**
 * @fn render_turn_info
 * @brief Renders the tracking information header for the active player's ongoing turn.
 * 
 * @details If not in Help stage, prints a single summarized status line containing the current player's name, 
 * the global game turn counter, and the remaining capacity of the active dice bag container.
 *
 * @param gc A reference to the active game controller instance hosting the current player and board states.
 */
void render_turn_info(const GameController& gc) {
    if(gc.CurrStage != GameStage::Help){
        std::cout << "Player: \"" << gc.Players[gc.currplayer_idx()]->Name << "\""
                << " | Turn #: " << gc.Turn
                << " | Bag has: " << gc.dice_in_bag() << " 🎲.\n\n";
    }
}

// ── Seção 2: Rolling Table ────────────────────────────────────────────────────

/**
 * @fn render_rolling_table
 * @brief Renders the textual ASCII grid representation of the rolling table for the three active dice.
 * 
 * @details If not in Help stage, dynamically draws a bordered 3-column table that structures and displays dice information. 
 * The cell formatting maintains fixed character alignments based on specific states:
 * - **Roll Stage (`GameStage::Roll`)**: Displays the randomized face result emoji alongside its matching color token.
 * - **Play Stage (`GameStage::Play`)**: Retains and displays only the held rerollable footprint dice ('👣') 
 * carried over from the previous action.
 * - **Empty Cells / Safety fallbacks**: Automatically fills remaining or unallocated dice slots with measured 
 * blank space padding (13 or 14 spaces depending on the column index) to prevent interface shifting or 
 * structural misalignment with the borders.
 *
 * @param gc A reference to the active game controller instance containing board grids, dice results, and engine stages.
 */
void render_rolling_table(const GameController& gc) {
    if(gc.CurrStage != GameStage::Help){
        // Static title and table
        std::cout << "┌───────────────────────────────────────────┐\n";
        std::cout << "│               Rolling Table               │\n";
        std::cout << "├──────────────┬──────────────┬─────────────┤\n";
        std::cout << "│";

        for (int i = 0; i < 3; i++) {
            // If there are a dice on hand in each cell
            if (i < (int)gc.Board.DiceOnHand.size() && gc.CurrStage == GameStage::Roll) {
                std::string res = " ";
                if (i < (int)gc.Board.DiceResults.size() && gc.Board.DiceResults[i] != ' ') {
                    res = result_emoji(gc.Board.DiceResults[i]);
                }
                std::string col = dice_color(gc.Board.DiceOnHand[i]);

                // CASE 1: The dice has already been rolled and there is a result (Ex: 👣(🟩) -> 2 emojis)
            
                if (i == 2) {
                    // Cell 3: 3 spaces + 2 emojis + paranthesis + 3 spaces = perfectly aligns with the top
                    std::cout << "   " << res << "(" << col << ")    │";
                } else {
                    // Cells 1 and 2: 4 spaces + 2 emojis + paranthesis + 3 spaces = perfectly aligns with the top
                    std::cout << "    " << res << "(" << col << ")    │";
                }

            }
            
            else if (!gc.Board.DiceOnHand.empty() && gc.CurrStage == GameStage::Play) {
                if (i < (int)gc.Board.DiceOnHand.size()) {
                    std::string res = " ";
                    if (i < (int)gc.Board.DiceResults.size() && gc.Board.DiceResults[i] == 'f') {
                    res = result_emoji(gc.Board.DiceResults[i]);
                    }
                    std::string col = dice_color(gc.Board.DiceOnHand[i]);

                    if (i == 2) {
                        // Cell 3: 3 spaces + 2 emojis + paranthesis + 3 spaces = perfectly aligns with the top
                        std::cout << "   " << "👣" << "(" << col << ")    │";
                    } else {
                        // Cells 1 and 2: 4 spaces + 2 emojis + paranthesis + 3 spaces = perfectly aligns with the top
                        std::cout << "    " << "👣" << "(" << col << ")    │";
                    }

            
                } else {
                    // Only prints empty spaces when there isn't any dice to show
                    if (i == 2) {
                        std::cout << "             │"; // 13 espaços
                    } else {
                        std::cout << "              │"; // 14 espaços
                    }
                }
            }
            
            // CASE 2: The cell is totally empty
            else {
                if (i == 2) {
                    std::cout << "             │"; // 13 espaços
                } else {
                    std::cout << "              │"; // 14 espaços
                }
            }
        }
        std::cout << "\n";
        std::cout << "└──────────────┴──────────────┴─────────────┘\n";
    }
}

// ── Seção 3: Brains e Shots ───────────────────────────────────────────────────

/**
 * @fn render_bsa
 * @brief Renders the accumulated scoring pools (Brains and Shots) for the active turn.
 * 
 * @details If not in Help stage, prints two tracking lines representing the player's current turn buffer status:
 * - **Brains Line (`🧠`)**: Iterates through the `cur_Brains` container to display the color token 
 * of each collected brain die, followed by the total turn brains count (including recycled ones).
 * - **Shots Line (`💥`)**: Iterates through the `Shots` container to display the color token of 
 * each shotgun blast received, followed by the current total shot count for the turn.
 *
 * @param gc A reference to the active game controller instance hosting the board scoring pools.
 */
void render_bsa(const GameController& gc) {
    if(gc.CurrStage != GameStage::Help){
        std::cout << "\n";
        std::cout << "🧠: ";
        for(int i = 0; i < gc.Board.cur_Brains.size(); i++){
            std::cout << dice_color(gc.Board.cur_Brains[i]) << " ";
        }
        std::cout << "(" << gc.Board.total_brains() << ")\n";

        std::cout << "💥: ";
        for(int i = 0; i < gc.Board.Shots.size(); i++){
            std::cout << dice_color(gc.Board.Shots[i]) << " ";
        } 
        std::cout << "(" << gc.Board.Shots.size() << ")\n\n";
    }
}


// ── Seção 4: Message area ─────────────────────────────────────────────────────

/**
 * @brief Renders context-sensitive instruction cards and messages inside an aligned ASCII interface box.
 * 
 * @details If the game is not in the Help stage, this function evaluates the intersection between 
 * the current game state (`CurrStage`) and the active entity type (Human vs. Bot AI) to print descriptive 
 * choices, event notifications, or end-game metrics.
 * * Internal lambda helpers (`mline`) standardize text layouts by tracking bounding box widths and 
 * automatically injecting trailing padding spaces to keep the right-hand border straight.
 * * State branches evaluated:
 * - **Play Stage (Human)**: Displays standard turn choices (Roll, Hold, Quit) or a terminal shotgun blast damage warning.
 * - **Play Stage (Bot)**: Shifts options into step-by-step automatic confirmation layouts.
 * - **Roll Stage (Human/Bot)**: Tallies up and highlights immediate successes (`rolled_brains`) and damage (`rolled_shots`).
 * - **ConfirmEnd Stage**: Displays a dynamic prompt verifying if the user truly wishes to abandon progress.
 * - **Tie Stage**: Displays warnings regarding sudden death tie-breaking rule engagement and dynamically adapts box width to fit competing player names.
 * - **End Stage**: Loops across all player configurations to crown and display the definitive match winner(s).
 *
 * @param gc A reference to the active game controller instance directing current phase values and players.
 */
void render_message_area(const GameController& gc) {
    if(gc.CurrStage != GameStage::Help && gc.CurrStage != GameStage::Tie && gc.CurrStage != GameStage::End){
        // Static top
        std::cout << "┌─[ Message area ]──────────────────────────────────┐\n";

        // Calculates the width of inner lines
        auto mline = [](const std::string& text) {
            int total_w = 50; 
            std::cout << "│ " << text;
            int pad = total_w - (int)text.size();
            if (pad > 0) std::cout << std::string(pad, ' ');
            std::cout << "│\n";
        };

        if (gc.CurrStage == GameStage::Play && gc.Players[gc.currplayer_idx()]->is_bot() == false) {
            if (gc.Board.Shots.size() >= 3) {
                mline("You got shot 3 times! You lose all brains.");
                mline("Press <enter> to continue.");
            } else {
                // Messages for before the dice have been rolled
                mline("Ready to play?");
                mline("  <enter> - roll dices");
                mline("  H + <enter> - hold turn");
                mline("  Q + <enter> - quit game");
                mline("  -h or --help + <enter> - help table");
            }
            
        } else if (gc.CurrStage == GameStage::Play && gc.Players[gc.currplayer_idx()]->is_bot() == true){
        if (gc.Board.Shots.size() >= 3) {
                mline("The bot got shot 3 times! It loses all brains.");
                mline("Press <enter> to continue.");
            } else {
                // Messages for before the dice have been rolled if the CurrPlayer is a bot
                mline("Ready to continue?");
                mline("  <enter> - next play");
                mline("  Q + <enter> - quit game");
            } 

        } else if (gc.CurrStage == GameStage::Roll && gc.Players[gc.currplayer_idx()]->is_bot() == false) {
            //Messages for when the dices have been rolled
            mline("Rolling outcome:");
            mline("  # brains you ate: " + std::to_string(gc.rolled_brains()));
            mline("  # shots that hit you: " + std::to_string(gc.rolled_shots()));
            mline("Press <enter> to continue.");

        } else if (gc.CurrStage == GameStage::Roll && gc.Players[gc.currplayer_idx()]->is_bot() == true){
            mline("Rolling outcome:");
            mline("  # brains it ate: " + std::to_string(gc.rolled_brains()));
            mline("  # shots that hit it: " + std::to_string(gc.rolled_shots()));
            mline("Press <enter> to continue.");

        } else if(gc.CurrStage == GameStage::ConfirmEnd) {
            mline("Are you sure you want to quit the game?");
            mline("All progress will be lost!");
            mline("   R + <enter> - return");
            mline("   <enter> - quit");
        }

        // Static bottom align with the top
        std::cout << "└───────────────────────────────────────────────────┘\n";
    } else if(gc.CurrStage == GameStage::Tie || gc.CurrStage == GameStage::End){
        count_t width{34};

        std::string tied_players{""};
        for(int i = 0; i < gc.PlayerOrder.size(); i++){
            int idx = gc.PlayerOrder[i];
            tied_players += " \"" + gc.Players[idx]->Name + "\" ";
        }

        if(tied_players.size() > 30) width = tied_players.size();

        std::cout << "┌─[ Message area ]" << msgTableLine(width) << "┐\n";

        auto mline = [&](const std::string& text) {
            int total_w = width + 15;

            std::cout << "│ " << text;

            int pad = total_w - text.size();
            if (pad > 0) std::cout << std::string(pad, ' ');
            
            std::cout << " │\n";
        };

        if(gc.CurrStage == GameStage::Tie){
            mline("We have a tie!");
            mline("Players in tie break:");
            mline(tied_players);
            mline("Press <enter> to start tiebreaker.");
        } else if(gc.CurrStage == GameStage::End){
            mline("Game over!");
            for (const auto& p : gc.Players) {
                if (p->Brains >= gc.WinBrains) {
                    mline("Winner: \"" + p->Name + "\" with "
                        + std::to_string(p->Brains) + " brain(s)!");
                }
            }
        }

        std::cout << "└" << msgTableLine(width + 17) << "┘\n";
    }
}

// ── Render completo ───────────────────────────────────────────────────────────

/**
 * @fn GameController::render
 * @brief Clears the screen buffer and sequences the rendering layout for the active frame.
 * 
 * @details Performs a platform-dependent clear screen command (`cls` on Windows, `clear` on POSIX 
 * environments) to refresh the display. Once cleared, it builds the entire UI top-to-bottom by 
 * calling the specific modular drawing functions in an exact order:
 * 1. Title Graphic (`render_title`)
 * 2. Standings Layout (`render_scoreboard`)
 * 3. Match metrics header (`render_turn_info`)
 * 4. Current dice state table (`render_rolling_table`)
 * 5. Turn pools indicators (`render_bsa`)
 * 6. Bounded notification board (`render_message_area`)
 * * Finally, appends an active prompt marker (`🧟> `) to indicate input focus if the current active 
 * entity is a human participant.
 */
void GameController::render() {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif

    render_title();
    render_scoreboard(*this);
    render_turn_info(*this);
    render_rolling_table(*this);
    render_bsa(*this);
    render_message_area(*this);

    if(Players[currplayer_idx()]->is_bot() == false && CurrStage != GameStage::End){
        std::cout << "🧟> ";
    }
    else {
        std::cout << "";
    } 
}