# Introduction
Esse projeto é a nossa versão da implementação do jogo de tabuleiro do ZombieDice.

A estrutura do projeto ficou dividida assim:

main.cpp e Setup: O ponto de entrada que carrega as regras de um arquivo .ini configurável e gerencia o menu inicial, onde validamos os jogadores e permitimos a adição de bots.  

GameController (game_bones.hpp / game_controller.cpp): É a máquina de estados do jogo. Controla de quem é a vez, passa os turnos, verifica empates e valida as condições de vitória.  

GameBoard: Estrutura que cuida da mecânica da rodada em si — como embaralhar e comprar os dados do tubo usando std::mt19937, computar os resultados dos dados rolando na mesa e reciclar os dados quando o tubo esvazia. 

render.cpp: Toda a interface visual via prompt de comando. Ela limpa a tela a cada jogada e monta uma tabela de rolagem estruturada com o placar geral e emojis coloridos para os dados.

*bônus: z_ai.cpp / z_ai.hpp: O sistema de IA dos bots. Criamos uma lógica onde eles calculam o risco com base nos tiros recebidos, jogando no modo "tudo ou nada" se estiverem prestes a perder ou parando se garantirem a vitória. 


# Author(s)
Annita Aquino Freitas
(annitaaquino@gmail.com)

Júlia de Medeiros Borba
(borba.julia06@gmail.com)

# Problems found or limitations
Annita: Eu fiquei com parte do front-end, então toda a parte de parse da linha de comando, mensagens de erro e impressão da interface. A parte em que eu tive mais dificuldade foi em lidar com emojis e corrigir a diferença entre um caracter normal e um emoji de forma que todas as tabelas permaneçam alinhadas independetemente da mensagem apresentada. 
Ademais, na minha parte, haverá uma limitação na impressão da tablea e ela se mostrará se o usuário fornecer algum nome que utilize acento. Nesse caso a tabela que mostra os jagadores, bem como a tabela de mensagens se esse jogador for o vencedor, apresentaram um desalinhamento do fechamento na linha em que o tal acento aparece.

Julia: Fiquei com a parte do back-end e decidi também criar um bot como um desafio para mim mesma. A maior dificuldade que eu encontrei nesse projeto foi o manejamento de todas as variáveis, por serem muitas e com nomes parecidos, e garantir que o bot jogasse quando desse empate (só consegui descobrir o motivo dele não estar jogando quando tinham 5 de nós numa ligação e o motivo foi porque eu troquei duas variáveis de lugar sem querer).
Acerca de limitações, creio que não tem nenhuma. A não ser, claro, as configurações (max de cérebros e jogadores), mas aí cabe ao usuário decidir.

# Grading

Preencha os itens que você implementou, indicando qual a nota esperada para cada item.

| Item                                                | Valor máximo | Valor esperado |
| --------------------------------------------------- | :----------: | :------------: |
| The project contains at least 2 classes             |      7       |       7        |
| Show help as requested `-h`                         |      4       |       4        |
| Read player names                                   |      10      |       10       |
| Show all 5 regions in user interface                |      12      |       12       |
| Keeps overall scores                                |      5       |       5        |
| Keeps bag of dice consistent                        |      5       |       5        |
| Handles automatic turn hand over                    |      5       |       5        |
| Handles hold requests                               |      5       |       5        |
| Handles roll requests                               |      5       |       5        |
| Handles ties                                        |      12      |       12       |
| Move brains dice back to bag when needed            |      10      |       10       |
| Manages turns correctly                             |      5       |       5        |
| Reads configuration file                            |      15      |       15       |
| Handles user errors                                 |      5       |       5        |
| Show winner correctly                               |      5       |       5        |
| Program working fine                                |      10      |       10       |

# Compiling and Running

Para compilar nosso programa a gente utilizou o g++ com o seguinte comando:

g++ src/main.cpp src/game_controller.cpp src/setup_interface.cpp src/render.cpp src/z_ai.cpp -I./include -std=c++17 -o zdice