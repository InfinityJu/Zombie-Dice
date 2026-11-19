# 🧟 Zombie Dice Deluxe

A C++17 terminal implementation of the **Zombie Dice** board game, developed in partnership with [@annitaaquino](https://github.com/annitaaquino) and [@ThonyIsNotAvaliable](https://github.com/ThonyIsNotAvaliable).

### Table of Contents

1. [Overview](#overview)
2. [Features & AI System](#features--ai-system)
3. [Gameplay](#gameplay)
4. [Building & Running](#building--running)
5. [Configuration File (`.ini`)](#configuration-file-ini)
6. [Interface & Controls](#interface--controls)
7. [Project Architecture](#project-architecture)
8. [Known Limitations](#known-limitations)
9. [Authors](#authors)

---

## Overview

**Zombie Dice Deluxe** is a terminal-based version of the classic push-your-luck dice game **Zombie Dice**&copy;.

Built with Object-Oriented C++17, smart pointers (`std::unique_ptr`), Mersenne Twister pseudo-random number generation (`std::mt19937`), and a state-machine-driven [Game Loop](https://gameprogrammingpatterns.com/game-loop.html) (`process_events()`, `render()`, and `update()`), the game features an interactive ASCII/UTF-8 interface and a custom Decision Tree AI engine.

---

## Features & AI System

- **Flexible Matchups:** Play with human players, mix humans and AI opponents, or play solo against multiple bots.
- **Decision Tree AI (`decision_tree.hpp`):** Automated players evaluate the game state in real time (`Roll` or `Hold`) using a custom generic decision tree framework supporting binary, switch, chance, and probability-threshold nodes across five personalities:
  - **Coward:** Plays conservatively, banking points as soon as it takes 1 shotgun blast or collects 3 brains.
  - **Smart:** Weighs remaining points to win against current turn risk before deciding to roll again.
  - **Reckless:** Pushes its luck across multiple consecutive risky rolls using an internal risk counter.
  - **Gambler:** Leaves decisions to a 50/50 coin flip powered by the board's RNG engine.
  - **ZuperZmart:** Computes exact survival probabilities using hypergeometric combinations of the remaining dice in the cup and dynamically adjusts its risk threshold based on turn score, distance to victory, and opponent standings.
- **Signature Bots:** Specific bot names from the pool automatically receive preset personalities (e.g., `"Rogerio"` plays as `Smart`, `"Vilarp"` plays as `Coward`).
- **Quality-of-Life Controls:** Built-in rules screen (`-h` / `--help`), quit confirmation prompt (`Q`), automatic sudden-death tie-breakers, and seamless match replay.

---

## Gameplay

In Zombie Dice, you and the other players are zombies (🧟) competing to eat **13 human brains** (or the target set in your config file) while avoiding shotgun blasts from humans fighting back.

### Rolling the Dice in a Turn

At the beginning of a turn, you draw three random dice from the **bag of dice** and roll them.

Each die represents a human of a specific difficulty tier. By default, the bag starts with **13 dice**: 6 green, 4 yellow, and 3 red.

| Die Tier | Color | Default Count | Faces | Probabilities |
| :--- | :---: | :---: | :---: | :--- |
| **Weak** | 🟩 Green | 6 | `bbbffs` | 50.0% 🧠 · 33.3% 👣 · 16.7% 💥 |
| **Strong** | 🟨 Yellow | 4 | `bbffss` | 33.3% 🧠 · 33.3% 👣 · 33.3% 💥 |
| **Tough** | 🟥 Red | 3 | `bffsss` | 16.7% 🧠 · 33.3% 👣 · 50.0% 💥 |

Each rolled face is sorted into its corresponding area:

- **Brain (🧠):** You ate your victim's brain. The die is moved to the _Brain Storage Area_ (BSA).
- **Shotgun (💥):** Your victim fought back. The die is moved to the _Shotgun Storage Area_ (SSA).
- **Footprints (👣):** Your victim escaped. The die stays in the _Dice Rolling Area_ (DRA) to be rolled again if you continue your turn.

After sorting the dice, one of two outcomes occurs:

1. **Three or more shotguns (💥):** You are severely wounded and your turn ends immediately with **0 points** for the round. All dice return to the bag and the turn passes to the next player.
2. **Fewer than three shotguns:** You may choose to **(a) stop and score** or **(b) continue rolling**.

### (a) Voluntary Stopping and Scoring

Choosing **Hold (`H`)** permanently banks all brains collected during the turn onto the global **scoreboard**. All dice return to the bag and the turn passes to the next player.

### (b) Continue Rolling

If you choose to keep rolling, any footprint dice (👣) remain in the DRA and new random dice are drawn from the bag until you have 3 dice in hand again.

If the bag runs out of dice mid-turn, your accumulated turn brains are preserved in a reserve counter and the brain dice from the BSA are recycled back into the bag (shotgun dice remain in the SSA).

### Game Over & Breaking a Tie

Once a player reaches **13+ brains**, the current round continues until every player has completed an equal number of turns.

- If a single player holds the highest score at the end of that round, they win the match.
- If two or more players are tied for the highest score, **only the tied players** enter sudden-death tie-breaker rounds until a single winner emerges.

```text
┌─[ Message area ]────────────────────────┐
│ We have a tie!                          │
│ Players in tie break:                   │
│ "Marina" "Vinicius"                     │
│ Press <enter> to start tiebreaker.      │
└─────────────────────────────────────────┘
```

---

## Building & Running

### Prerequisites

- A C++ compiler with **C++17** support (e.g., `g++`).
- A terminal emulator configured with **UTF-8** encoding to properly render emojis and box-drawing characters.

### Compilation

From the project root directory, run:

```bash
g++ src/main.cpp src/game_controller.cpp src/setup_interface.cpp src/render.cpp src/z_ai.cpp src/ai_tree.cpp -I./include -std=c++17 -o zdice
```

### Execution

Run with default settings:

```bash
./zdice
```

Or pass a custom `.ini` configuration file as a command-line argument:

```bash
./zdice zdice.ini
```

### AI Simulator & Benchmarks

To evaluate and compare the five Decision Tree AI personalities (`Coward`, `Reckless`, `Smart`, `Gambler`, and `ZuperZmart`), the project includes a standalone headless simulator (`src/simulator.cpp`) created by **Gabriel Brandão Vilar**. By default, it runs **100,000 automated bot-only matches** (`SEED = 1`, 2,000-turn limit per match) and logs each game's winner alongside overall win percentages.

Compile and run the simulator from the project root:

```bash
g++ src/simulator.cpp src/game_controller.cpp src/ai_tree.cpp src/z_ai.cpp -I./include -std=c++17 -O2 -o simulator
./simulator

---

## Configuration File (`.ini`)

You can customize dice counts, die faces, player limits, and victory thresholds through an `.ini` file. If no file is passed or the path cannot be opened, the game falls back to the default values below:

```ini
[Game]
weak_dice = 6
strong_dice = 4
tough_dice = 3
max_players = 6
brains_to_win = 13
max_turns = 0
bot_count = 0

[Dice]
weak_die_faces   = "bbbffs"
strong_die_faces = "bbffss"
tough_die_faces  = "bffsss"
```

---

## Interface & Controls

### Welcome & Setup Screen

Upon launching, `zdice` displays the rules and prompts for a comma-separated list of player names. If only one name is entered (or fewer than `max_players`), you can add AI bots before shuffling the starting turn order.

```text
$ ./zdice zdice.ini

         ---> Welcome to the Zombie Dice game (v 0.1) <--
                -- copyright DIMAp/UFRN 2024-2026 --

 The object of the jeopardy dice game Zombie Dice is to be the
 first to eat 13 or more human brains in a turn.
 Each player's turn consists of repeatedly rolling 3 dice.
 A die may produce 3 outcomes: you ate a brain, the human escaped,
 or you were shot!
 If you get shot 3 or more times you die and lose all brains you
 ate in that turn; then the turn goes to the next player.
 However, if you decide to stop before get shot 3 times, you keep
 the brains you ate.

 Before we begin, please enter the names of the players
 in a single line, separated by comma. For example:
 >>> Taylor , Ariana, Sabrina, Olivia

Minimum of 2 players, maximum of 6 names.
Providing an empty line will end the game.
>>>
```

### In-Game Commands

| Command | Action |
| :--- | :--- |
| `<Enter>` | Roll the dice (or advance an AI bot's step) |
| `H` + `<Enter>` | Hold turn and bank accumulated brains |
| `-h` / `--help` + `<Enter>` | Display the in-game instructions panel |
| `Q` + `<Enter>` | Open the quit confirmation prompt |

---

## Project Architecture

```text
├── include/
│   ├── game_bones.hpp       # Core structures (GameController, GameBoard, ConfigLoader, PlayerInfo)
│   ├── game_controller.hpp  # Dice color rendering helpers
│   ├── setup_interface.hpp  # Player registration and match setup routines
│   ├── utils.hpp            # String trimming and case-conversion utilities
│   ├── z_ai.hpp             # Base bot definitions (BotInfo, BotDifficulty) and fallback logic
│   ├── decision_tree.hpp    # Generic Decision Tree framework (Binary, Switch, Chance, Threshold nodes)
│   ├── ai_tree.hpp          # Bot context and personality decision tree builders
│   ├── zuper_zmart.hpp      # Hypergeometric probability model and utility function for ZuperZmart
│   └── ai_manager.hpp       # Dispatcher between decision-tree bots and classic fallback logic
├── src/
│   ├── main.cpp             # Entry point, exception handling, and replay loop
│   ├── game_controller.cpp  # Finite state machine, dice mechanics, turn progression, and tie-breaking
│   ├── setup_interface.cpp  # Interactive setup prompts and bot instantiation
│   ├── render.cpp           # ASCII/UTF-8 terminal UI rendering
│   ├── z_ai.cpp             # Imminent win/loss evaluators and classic bot rules
│   ├── ai_tree.cpp          # Global decision tree instantiation and evaluation
│   └── simulator.cpp        # Headless 100,000-game bot benchmark simulator
└── proof.txt                # Benchmark output log with match-by-match winners and AI win percentages
    
```

---

## Known Limitations

- **Accented Characters in Player Names:** Because `std::setw` counts UTF-8 multi-byte characters by byte length rather than visual glyph width, player names containing accented characters (e.g., `Júlia`, `Antônio`) may slightly misalign the right border of the scoreboard and winner message box.

---

## Authors

- **Annita Aquino Freitas** --- Front-end, CLI input parsing, error handling, and ASCII terminal UI rendering.
- **Anthony Gabriel Gomes Rocha** --- Decision tree architecture and the "zuper zmart" AI bot.
- **Gabriel Brandão Vilar** --- Automated headless bot simulator and AI win-rate benchmark validation (`proof.txt`).
- **Júlia de Medeiros Borba** --- Back-end, game state machine, dice mechanics, and bot AI system.