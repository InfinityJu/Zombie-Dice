/**
 * @file simulator.cpp
 * @brief Runs N Zombie Dice games with bots only (one of each type) and
 *        shows which type won each game and the final win percentage.
 * 
 *     g++ -std=c++17 -O2 src/simulator.cpp src/game_controller.cpp src/ai_tree.cpp src/z_ai.cpp -o simulator
 *     ./simulator
 * 
 */

#include "../include/game_bones.hpp"
#include "../include/game_controller.hpp"
#include "../include/ai_tree.hpp"   

#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

constexpr int N_GAMES = 100000;
constexpr unsigned long SEED = 1;
constexpr bool PRINT_EACH_GAME = true;
/// .ini file with the game rules. "" = default rules
/// (13 brains, 6/4/3 dice, etc.).
constexpr const char* CONFIG_FILE = "";
constexpr count_t MAX_TURNS = 2000;

// ============================================================================

namespace {

const std::vector<BotDifficulty> kTypes = {
    BotDifficulty::COWARD,
    BotDifficulty::RECKLESS,
    BotDifficulty::SMART,
    BotDifficulty::GAMBLER,
    BotDifficulty::ZUPERZMART
};

std::string type_name(BotDifficulty d) {
    switch (d) {
        case BotDifficulty::COWARD:     return "Coward";
        case BotDifficulty::RECKLESS:   return "Reckless";
        case BotDifficulty::SMART:      return "Smart";
        case BotDifficulty::GAMBLER:    return "Gambler";
        case BotDifficulty::ZUPERZMART: return "ZuperZmart";
        default:                        return "Undef";
    }
}

/// Decides the current bot's action using the tree: "" (roll) or "H" (hold).
std::string bot_decision(GameController& gc, BotInfo& bot) {
    // Context built directly from the real board (by reference),
    // without going through context_extractor (which copies the GameBoard).
    BotContext ctx{gc.Board, bot, gc.WinPlayers, gc.WinBrains,
                   gc.PlayerNum, static_cast<count_t>(gc.PlayerOrder.size())};
    return tree_processing(ctx);
}

/// Equivalent to GameController::update(), without reading the keyboard.
void step_update(GameController& gc) {
    if (gc.CurrStage == GameStage::Play) {
        if (gc.Board.Shots.size() >= 3) {
            gc.advance_player(true);
            gc.reset_board();
            return;
        }

        auto idx = gc.PlayerOrder[gc.CurrPlayer];
        auto* bot = dynamic_cast<BotInfo*>(gc.Players[idx].get());
        std::string op = bot ? bot_decision(gc, *bot) : "H";

        if (op.empty()) {
            gc.CurrStage = GameStage::Roll;
        } else if (op == "H" || op == "h") {
            gc.advance_player(false);
            gc.reset_board();
        }
    }
    else if (gc.CurrStage == GameStage::Roll || gc.CurrStage == GameStage::Tie) {
        gc.CurrStage = GameStage::Play;
    }
}

/// Runs one game. Returns the winning type, or UNDEF if the turn limit was exceeded.
BotDifficulty play_one_game(const ConfigLoader& loader, std::mt19937& seeder) {
    GameController gc;
    gc.Config = loader.config;
    gc.WinBrains = loader.config.brains_to_win;
    gc.Board.g.seed(seeder());

    // One bot of each type, no human player.
    for (auto type : kTypes) {
        auto bot = std::make_unique<TreeBotInfo>();
        bot->Name = type_name(type);
        bot->botdifficulty = type;
        gc.Players.push_back(std::move(bot));
    }

    gc.PlayerNum = gc.Players.size();
    gc.PlayerOrder.resize(gc.PlayerNum);
    gc.initialize_playerorder();          // randomized turn order
    gc.initialize_dicebag(loader);
    gc.reset_board();

    while (!gc.game_over(false)) {
        if (gc.Turn > MAX_TURNS) return BotDifficulty::UNDEF;
        gc.process_events(loader);
        step_update(gc);
    }

    // Winner = whoever has the most brains at the end.
    const PlayerInfo* best = nullptr;
    for (auto& p : gc.Players)
        if (!best || p->Brains > best->Brains) best = p.get();

    return dynamic_cast<const BotInfo*>(best)->botdifficulty;
}

} // namespace

int main() {
    ConfigLoader loader;
    if (std::string(CONFIG_FILE).size() > 0) loader.load(CONFIG_FILE);

    std::mt19937 seeder(static_cast<unsigned>(SEED));

    std::map<BotDifficulty, int> wins;
    int noWinner = 0;

    std::cout << "Simulating " << N_GAMES << " games (seed " << SEED
              << ", decision tree) with:";
    for (auto type : kTypes) std::cout << " " << type_name(type);
    std::cout << "\n\n";

    for (int i = 1; i <= N_GAMES; i++) {
        BotDifficulty w = play_one_game(loader, seeder);

        if (w == BotDifficulty::UNDEF) {
            noWinner++;
            if (PRINT_EACH_GAME) std::cout << "Game " << i << ": no winner\n";
        } else {
            wins[w]++;
            if (PRINT_EACH_GAME) std::cout << "Game " << i << ": " << type_name(w) << "\n";
        }
    }

    // --- Summary ---
    std::cout << "\n===== RESULT (" << N_GAMES << " games, seed " << SEED << ") =====\n"
              << std::fixed << std::setprecision(2);

    for (auto type : kTypes) {
        double pct = 100.0 * wins[type] / N_GAMES;
        std::cout << std::left << std::setw(12) << type_name(type)
                  << std::right << std::setw(8) << wins[type] << " wins  "
                  << std::setw(7) << pct << "%\n";
    }
    if (noWinner > 0)
        std::cout << "No winner: " << noWinner << " ("
                  << 100.0 * noWinner / N_GAMES << "%)\n";

    BotDifficulty top = kTypes[0];
    for (auto type : kTypes) if (wins[type] > wins[top]) top = type;
    std::cout << "\nType with the most wins: " << type_name(top) << "\n";

    return EXIT_SUCCESS;
}