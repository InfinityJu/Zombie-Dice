#include "../include/ai_tree.hpp"

// --- Bot Creation Functions -- //

std::unique_ptr<PlayerInfo> create_tree_bot( std::mt19937& g ){
    auto NewBot = std::make_unique<TreeBotInfo>();
    
    NewBot->Name = bot_name_picker(g);

    if (NewBot->Name == "Rogerio"){
        NewBot->botdifficulty = BotDifficulty::SMART;
    } else if (NewBot->Name == "Vilarp"){
        NewBot->botdifficulty = BotDifficulty::COWARD;
    } else {
        NewBot->botdifficulty = bot_difficulty_picker(g);
    }

    return NewBot;
}

// --- Methods -- //

BotContext context_extractor(GameBoard Board, BotInfo& BotData, count_t WinPlayers, count_t WinBrains,
                            count_t PlayerNum, count_t PlayersPlaying){
    return {Board, BotData, WinPlayers, WinBrains, PlayerNum, PlayersPlaying};
}

BotNodePtr& get_global_bot_tree() {
    static BotNodePtr globalTree = bot_tree_creator();
    return globalTree;
}

std::string tree_processing( BotContext& ctx ){
    const auto& tree = get_global_bot_tree();

    if (!tree) return "";
    return tree->evaluate(ctx);
}

// --- Tree makers -- //

BotNodePtr create_coward_tree() {
    using namespace pdt::helpers;
    return decide<BotContext, std::string>(
        [](const BotContext& c) { return c.board.Shots.size() >= 1 || c.board.total_brains() >= 3; },
        leaf<BotContext, std::string>("H"),
        leaf<BotContext, std::string>("")
    );
}

BotNodePtr create_gambler_tree() {
    using namespace pdt::helpers;
    auto gamblerChance = chanceWithRng<BotContext, std::string, std::mt19937>(
        [](BotContext& c) -> std::mt19937& { return c.board.g; }
    );
    gamblerChance->addBranch(1.0, leaf<BotContext, std::string>(""));
    gamblerChance->addBranch(1.0, leaf<BotContext, std::string>("H"));
    return gamblerChance;
}

BotNodePtr create_reckless_tree() {
    using namespace pdt::helpers;

    auto high_risk_branch = decide<BotContext, std::string>(
        [](const BotContext& c) { return c.botData.riskyrolls < 3; },
        leaf<BotContext, std::string>([](BotContext& c) { c.botData.riskyrolls++; return ""; }),
        leaf<BotContext, std::string>("H")
    );

    auto low_risk_branch = decide<BotContext, std::string>(
        [](const BotContext& c) { return c.botData.riskyrolls < 2 || c.board.total_brains() > (c.winBrains * 0.5); },
        leaf<BotContext, std::string>([](BotContext& c) { c.botData.riskyrolls++; return ""; }),
        leaf<BotContext, std::string>("H")
    );

    return decide<BotContext, std::string>(
        [](const BotContext& c) {
            auto brainsToWin = c.winBrains - c.board.total_brains();
            return c.board.Shots.size() == 1 || c.board.total_brains() > (brainsToWin * 0.66);
        },
        std::move(high_risk_branch),
        std::move(low_risk_branch)
    );
}

BotNodePtr create_smart_tree() {
    using namespace pdt::helpers;

    auto one_shot_branch = decide<BotContext, std::string>(
        [](const BotContext& c) { 
            auto brainsToWin = c.winBrains - c.board.total_brains();
            return brainsToWin < (c.winBrains * 0.66) || brainsToWin < 3 || c.board.total_brains() < 6; 
        },
        leaf<BotContext, std::string>(""),
        leaf<BotContext, std::string>("H")
    );

    auto multi_shot_branch = decide<BotContext, std::string>(
        [](const BotContext& c) { return c.board.cur_Brains.size() < 3; },
        leaf<BotContext, std::string>(""),
        leaf<BotContext, std::string>("H")
    );

    return decide<BotContext, std::string>(
        [](const BotContext& c) { return c.board.Shots.size() == 1; },
        std::move(one_shot_branch),
        std::move(multi_shot_branch)
    );
}

BotNodePtr bot_tree_creator() {
    using namespace pdt::helpers;

    // Roteador por SwitchNode baseada na dificuldade
    auto difficultySwitch = switchOn<BotContext, std::string, BotDifficulty>(
        [](const BotContext& c) { return c.botData.botdifficulty; },
        leaf<BotContext, std::string>("")
    );

    difficultySwitch->addCase(BotDifficulty::COWARD, create_coward_tree());
    difficultySwitch->addCase(BotDifficulty::SMART, create_smart_tree());
    difficultySwitch->addCase(BotDifficulty::RECKLESS, create_reckless_tree());
    difficultySwitch->addCase(BotDifficulty::GAMBLER, create_gambler_tree());

    // Regra de segurança inicial (0 tiros ou 0 cérebros)
    auto safeStartCheck = decide<BotContext, std::string>(
        [](const BotContext& c) { return c.board.Shots.size() == 0 || c.board.cur_Brains.size() == 0; },
        leaf<BotContext, std::string>(""),
        std::move(difficultySwitch)
    );

    // Checagem de vitória iminente
    auto winCheck = decide<BotContext, std::string>(
        [](const BotContext& c) { return bot_bout_to_win(c.board, c.botData, c.winPlayers, c.winBrains, c.playersPlaying); },
        leaf<BotContext, std::string>("H"),
        std::move(safeStartCheck)
    );

    // Raiz: Checagem de derrota iminente
    return decide<BotContext, std::string>(
        [](const BotContext& c) { return bot_bout_to_lose(c.board, c.botData, c.winPlayers, c.winBrains); },
        leaf<BotContext, std::string>(""),
        std::move(winCheck)
    );
}