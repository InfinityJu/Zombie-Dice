#include "../include/ai_manager.hpp"

std::string bot_processing( GameBoard Board, BotInfo& BotData, count_t WinPlayers, count_t WinBrains,
                            count_t PlayerNum, count_t PlayersPlaying ){
    if (BotData.is_tree_bot()) {
        TreeBotInfo* treeBot = dynamic_cast<TreeBotInfo*>(&BotData);
        if (treeBot) {
            // Extrai o contexto padronizado
            BotContext ctx = context_extractor(Board, BotData, WinPlayers, WinBrains, PlayerNum, PlayersPlaying);
            // Avalia diretamente a árvore armazenada em cache no bot
            return tree_processing(ctx);
        }
    }

    // 2. Fallback: Se for um bot tradicional, executa a lógica clássica baseada em if-else
    return bot_if_else_processing(Board, BotData, WinPlayers, WinBrains, PlayerNum, PlayersPlaying);
}


std::unique_ptr<PlayerInfo> create_zombie_bot( std::mt19937& g ){
    return std::move(create_tree_bot(g));
}