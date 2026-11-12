#ifndef GAME_BONES_HPP
#define GAME_BONES_HPP

#include "utils.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <random>
#include <map>
#include <fstream>
#include <memory>

using count_t = unsigned long;

/**
 * @struct GameConfig
 * 
 * @brief Holds the configuration settings for a game session.
 * 
 * @details This structure defines the initial pool of dice, their face configurations,
 * and the core rules governing gameplay limits and win conditions.
 */
struct GameConfig {
    // Number of dices
    count_t weak_dice_count   = 6;
    count_t strong_dice_count = 4;
    count_t tough_dice_count  = 3;

    // Dice faces
    std::string weak_die_faces   = "bbbffs";
    std::string strong_die_faces = "bbffss";
    std::string tough_die_faces  = "bffsss";

    // Gameplay rules
    count_t max_players   = 6;
    count_t brains_to_win = 13;
    count_t max_turns     = 0;
    count_t bot_count     = 0;
};

/**
 * @class ConfigLoader
 * @brief Responsible for parsing an INI-style configuration file and populating a GameConfig instance.
 *
 * @fn ConfigLoader::initMaps
 * @brief Initializes the settings maps, linking configuration string keys directly to
 * their respective memory addresses inside the `config` member variable.
 *
 * @fn ConfigLoader::ConfigLoader
 * @brief Constructs a new ConfigLoader object and sets up the internal pointer maps.
 *
 * @fn ConfigLoader::load
 * @brief Loads and parses configuration parameters from a given file.
 * @details If the file cannot be opened, the existing default configurations in `config` are retained.
 * @param filename Path to the configuration file.
 */
class ConfigLoader{
    std::map<std::string, unsigned long*> numeric_settings; // mapeia as variáveis em unsigned short do arquivo ini
    std::map<std::string, std::string*> string_settings; // mapeia as variáveis em string do arquivo ini

    void initMaps() {
        numeric_settings = {
            {"weak_dice", &config.weak_dice_count},
            {"strong_dice", &config.strong_dice_count},
            {"tough_dice", &config.tough_dice_count},
            {"brains_to_win", &config.brains_to_win},
            {"max_turns", &config.max_turns},
            {"max_players", &config.max_players},
            {"bot_count", &config.bot_count}
        };

        string_settings = {
            {"weak_die_faces", &config.weak_die_faces},
            {"strong_die_faces", &config.strong_die_faces},
            {"tough_die_faces", &config.tough_die_faces}
        };
    }

public:
    GameConfig config;

    ConfigLoader() {initMaps();}

    void load(const std::string& filename){
        std::ifstream file(filename);

        if (!file.is_open()) return; // usa as configurações padrão

        std::string line;
        
        while(std::getline(file, line)){
            size_t commentSign = line.find('#');
            if(commentSign != std::string::npos) line.erase(commentSign); // desconsiderando o comentário se houver algum

            if (line.empty() || line.find('[') != std::string::npos) continue; // pula o nome das categorias das variáveis

            size_t mid = line.find('=');
            if (mid == std::string::npos) continue;

            std::string key = trim(line.substr(0, mid)); // pega o nome da variável
            std::string value = trim(line.substr(mid + 1)); // pega o valor atribuído à variável

            value.erase(std::remove(value.begin(), value.end(), '\"'), value.end()); // remove as aspas das variáveis em strings

            if (numeric_settings.count(key)) {
                *numeric_settings[key] = std::stoul(value); // converte a string para número
            } else if (string_settings.count(key)) {
                *string_settings[key] = value; // altera logo a string
            }
        }
 
    }     
};

/**
 * @class PlayerInfo
 * @brief Stores individual player information and statistics.
 * 
 * @fn bool is_bot
 * @brief Helps the code identify if a player is a human or not
 */
class PlayerInfo{
public:
    std::string Name;       //! Player's name
    count_t Brains{0};      //! Brains conquered by the player
    count_t turns{0};       //! Turns played by the player

    virtual bool is_bot() const { return false; }
};

/**
 * @enum Dice
 * 
 * @brief Represents the type categories of dice available in the game.
 */
enum class Dice{
    weak,
    strong,
    tough,
};

/** 
 * @enum GameStage
 * @brief Represents the various phases or states of the game loop life cycle.
 *
 * @var GameStage::Setup
 * The initialization phase where configurations are loaded and players are registered.
 *
 * @var GameStage::Start
 * The beginning of the game match, setting up initial conditions before the first turn.
 *
 * @var GameStage::Play
 * The active turn phase for a human player, awaiting their decisions.
 *
 * @var GameStage::BotPlay
 * The active turn phase automated and executed by an AI/bot player.
 *
 * @var GameStage::Roll
 * The specific action phase where dice are being shaken and rolled.
 *
 * @var GameStage::Tie
 * A tie-breaker phase triggered when players achieve the winning conditions simultaneously.
 *
 * @var GameStage::End
 * The final phase where the match concludes, scores are finalized, and the winner is announced.
 * 
 * @var GameStage::Help
 * The phase which shows the instructions for the game
 */
enum class GameStage{
    Play,
    Roll,
    Tie,
    End,
    Help,
    ConfirmEnd
};

/**
 * @struct GameBoard
 * @brief Manages the state of the board, dice pools, and rolling mechanics during a game round.
 *
 * @var GameBoard::g
 * Pseudo-random number generator (Mersenne Twister) initialized with a random device seed.
 *
 * @var GameBoard::cur_DiceBag
 * The current pool of dice remaining in the bag available to be drawn.
 *
 * @var GameBoard::DiceOnHand
 * The specific dice currently held by the player for the active turn.
 *
 * @var GameBoard::DiceResults
 * The face results (e.g., 'b', 'f', 's') obtained from the last roll.
 *
 * @var GameBoard::Shots
 * List of dice that resulted in a "shot" outcome during the current turn.
 *
 * @var GameBoard::cur_Brains
 * List of dice that resulted in a "brain" outcome during the current turn.
 *
 * @var GameBoard::extraBrains
 * Accumulated points or extra brains saved from previous valid actions.
 *
 * @fn GameBoard::move_dice
 * @brief Transfers dice between pools (e.g., from hand back to bag or vice versa).
 * @param reverse If true, reverses the direction of the dice movement.
 *
 * @fn GameBoard::draw_dice
 * @brief Draws a random set of dice from the `cur_DiceBag` into `DiceOnHand`.
 *
 * @fn GameBoard::roll_dice
 * @brief Simulates rolling the dice currently in `DiceOnHand` using faces defined in the configuration.
 * @param gcnf The loaded configuration container containing die face definitions.
 *
 * @fn GameBoard::brains_rolled
 * @brief Calculates the number of brains rolled in the active turn.
 * @return The count of brain outcomes.
 *
 * @fn GameBoard::shots_rolled
 * @brief Calculates the number of shots rolled in the active turn.
 * @return The count of shot outcomes.
 *
 * @fn GameBoard::total_brains
 * @brief Combines active turn brains and extra brains to return the total score.
 * @return Total accumulated brains.
 */
struct GameBoard{
    std::mt19937 g{ std::random_device{}() };

    std::vector<Dice> cur_DiceBag;
    std::vector<Dice> DiceOnHand;
    std::vector<char> DiceResults;

    std::vector<Dice> Shots;
    std::vector<Dice> cur_Brains;
    count_t extraBrains{0};

    // --- Member functions --- //

    void move_dice( bool reverse );
    void draw_dice();
    void roll_dice( ConfigLoader gcnf );

    count_t brains_rolled() const;
    count_t shots_rolled() const;
    count_t total_brains() const;
};


/**
 * @struct GameController
 * @brief Main controller structure that manages the core game loop, players, and rules execution.
 *
 * @fn GameController::initialize_dicebag
 * @brief Populates the master `DiceBag` with weak, strong, and tough dice according to the configuration.
 * @param gcnf The loaded configuration rules wrapper.
 *
 * @fn GameController::initialize_playerorder
 * @brief Sets up or randomizes the sequence in which players will take their turns.
 *
 * @fn GameController::reset_board
 * @brief Clears the current board state, resetting dice pools on hand and outcomes for the next turn/player.
 *
 * @fn GameController::currplayer_name
 * @brief Retrieves the name of the player who is currently active.
 * @return The current player's name string.
 *
 * @fn GameController::active_players
 * @brief Counts how many players are still eligible or participating in the current setup.
 * @return Total count of active players.
 *
 * @fn GameController::currplayer_idx
 * @brief Gets the actual internal index mapping to the current player structure.
 * @return The index of the current player.
 *
 * @fn GameController::dice_in_bag
 * @brief Helper to check how many dice are remaining inside the board's active bag.
 * @return Count of remaining dice in the active bag.
 *
 * @fn GameController::rolled_brains
 * @brief Retrieves the count of brains rolled by the active player on the board.
 * @return Number of brains rolled.
 *
 * @fn GameController::rolled_shots
 * @brief Retrieves the count of shots accumulated by the active player on the board.
 * @return Number of shots rolled.
 *
 * @fn GameController::advance_player
 * @brief Passes the turn to the next player in sequence.
 * @param dead If true, signifies that the current player lost their turn (e.g., accumulated too many shots).
 *
 * @fn GameController::tie_checker
 * @brief Evaluates whether a tie-breaker state is required when multiple players cross the win condition.
 *
 * @fn GameController::process_events
 * @brief Captures and handles inputs, decisions, or system events for the current game frame.
 * @param gcnf The loaded configuration container.
 *
 * @fn GameController::update
 * @brief Executes the game logic transitions based on the current stage and state attributes.
 *
 * @fn GameController::render
 * @brief Draws or outputs the current game state interface to the screen/console.
 *
 * @fn GameController::win
 * @brief Evaluates whether any player has officially met the criteria to win the match.
 * @return True if a player has won, false otherwise.
 *
 * @fn GameController::game_over
 * @brief Handles the cleanup and termination routines when a match ends.
 * @param win Boolean flag explicitly confirming whether the game ended due to a natural win condition.
 * @return True if the game sequence should stop completely.
 */
struct GameController{
    std::vector<std::unique_ptr<PlayerInfo>> Players;
    count_t PlayerNum{1};
    std::vector<count_t> PlayerOrder;
    count_t CurrPlayer{0};
    GameBoard Board;
    GameConfig Config;

    std::vector<Dice> DiceBag;
    count_t Turn{1};
    GameStage CurrStage{GameStage::Play};

    count_t WinBrains{13};
    count_t WinPlayers{0};

    // --- Member functions --- //

    void initialize_dicebag( ConfigLoader gcnf );
    void initialize_playerorder();
    void reset_board();

    std::string currplayer_name();
    count_t active_players();
    count_t currplayer_idx() const;
    count_t dice_in_bag() const;
    count_t rolled_brains() const;
    count_t rolled_shots() const;

    void advance_player( bool dead );
    void tie_checker();

    void process_events( ConfigLoader gcnf );
    void update();
    void render();

    bool win();
    bool game_over( bool win );
};

#endif