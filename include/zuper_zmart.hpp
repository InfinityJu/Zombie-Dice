#include "../include/ai_tree.hpp"

using namespace pdt::helpers;

struct MightDie {
  double getShot{ 0 };
  double getBrain{ 0 };
  double getFoot{ 0 };
};

constexpr MightDie WEAK{ 1.0 / 6, 3.0 / 6, 2.0 / 6 };
constexpr MightDie STRONG{ 2.0 / 6, 2.0 / 6, 2.0 / 6 };
constexpr MightDie TOUGH{ 3.0 / 6, 1.0 / 6, 2.0 / 6 };

constexpr double nCk(int n, int k) {
  if (k < 0 || k > n)
    return 0.0;
  if (k == 0 || k == n)
    return 1.0;
  if (k == 1 || k == n - 1)
    return static_cast<double>(n);
  if (k == 2 || k == n - 2)
    return static_cast<double>(n * (n - 1)) / 2.0;

  return static_cast<double>(n * (n - 1) * (n - 2)) / 6.0;
}

struct RollMetrics {
  double probSurvive{ 0 };
  double probSurviveProductive{ 0 };
};

RollMetrics evaluate_all_dice(const std::array<MightDie, 3>& d, size_t currentShots) {
  const double p0 = (1.0 - d[0].getShot) * (1.0 - d[1].getShot) * (1.0 - d[2].getShot);

  const double p1 = d[0].getShot * (1.0 - d[1].getShot) * (1.0 - d[2].getShot)
                    + (1.0 - d[0].getShot) * d[1].getShot * (1.0 - d[2].getShot)
                    + (1.0 - d[0].getShot) * (1.0 - d[1].getShot) * d[2].getShot;

  const double probAllFootsteps = d[0].getFoot * d[1].getFoot * d[2].getFoot;

  const double probSurvive = (currentShots <= 1) ? (p0 + p1) : p0;
  const double probSurviveProductive = std::max(0.0, p0 - probAllFootsteps);

  return { probSurvive, probSurviveProductive };
}

constexpr MightDie die_to_odds(Dice d) {
  switch (d) {
  case Dice::weak:
    return WEAK;  // 1 tiro, 3 cérebros, 2 pegadas
  case Dice::strong:
    return STRONG;  // 2 tiros, 2 cérebros, 2 pegadas
  case Dice::tough:
    return TOUGH;  // 3 tiros, 1 cérebro,  2 pegadas
  }
  return STRONG;
}

void count_dice_colors(const std::vector<Dice>& pile, int& green, int& yellow, int& red) {
  green = yellow = red = 0;
  for (Dice d : pile) {
    switch (d) {
    case Dice::weak:
      ++green;
      break;
    case Dice::strong:
      ++yellow;
      break;
    case Dice::tough:
      ++red;
      break;
    }
  }
}

constexpr double clamp_val(double v, double lo, double hi) { return std::max(lo, std::min(v, hi)); }

RollMetrics compute_roll_metrics(const BotContext& c) {
  std::vector<MightDie> fixedHand;
  fixedHand.reserve(3);

  for (Dice die : c.board.DiceOnHand) {
    if (fixedHand.size() < 3) {
      fixedHand.push_back(die_to_odds(die));
    }
  }

  int needed = std::max(0, 3 - static_cast<int>(fixedHand.size()));
  int greenCup = 0, yellowCup = 0, redCup = 0;

  if (static_cast<int>(c.board.cur_DiceBag.size()) >= needed) {
    count_dice_colors(c.board.cur_DiceBag, greenCup, yellowCup, redCup);
  } else {
    for (Dice d : c.board.cur_DiceBag) {
      if (fixedHand.size() < 3) {
        fixedHand.push_back(die_to_odds(d));
      }
    }
    needed = std::max(0, 3 - static_cast<int>(fixedHand.size()));

    count_dice_colors(c.board.cur_Brains, greenCup, yellowCup, redCup);

    if (greenCup + yellowCup + redCup < needed) {
      greenCup = 6;
      yellowCup = 4;
      redCup = 3;
    }
  }

  const int totalCup = greenCup + yellowCup + redCup;
  const double totalWays = nCk(totalCup, needed);
  const size_t currentShots = c.board.Shots.size();

  double expSurvive = 0.0;
  double expProductive = 0.0;

  for (int g = 0; g <= std::min(needed, greenCup); ++g) {
    for (int y = 0; y <= std::min(needed - g, yellowCup); ++y) {
      const int r = needed - g - y;
      if (r < 0 || r > redCup)
        continue;

      const double weight = (nCk(greenCup, g) * nCk(yellowCup, y) * nCk(redCup, r)) / totalWays;
      if (weight <= 0.0)
        continue;

      std::array<MightDie, 3> hand{};
      size_t idx = 0;

      for (const auto& f : fixedHand) {
        if (idx < 3)
          hand[idx++] = f;
      }
      for (int i = 0; i < g && idx < 3; ++i)
        hand[idx++] = WEAK;
      for (int i = 0; i < y && idx < 3; ++i)
        hand[idx++] = STRONG;
      for (int i = 0; i < r && idx < 3; ++i)
        hand[idx++] = TOUGH;

      const auto metrics = evaluate_all_dice(hand, currentShots);
      expSurvive += weight * metrics.probSurvive;
      expProductive += weight * metrics.probSurviveProductive;
    }
  }

  return { clamp_val(expSurvive, 0.0, 1.0), clamp_val(expProductive, 0.0, 1.0) };
}

// --- Tree makers -- //

BotNodePtr create_zuper_zmart_tree() {
  auto two_shots = probThreshold<BotContext, std::string>(
    [](const BotContext& ctx) { return compute_roll_metrics(ctx).probSurviveProductive; },
    0.4,
    leaf<BotContext, std::string>(""),
    leaf<BotContext, std::string>("H"));

  auto almost_dying = decide<BotContext, std::string>(
    [](const BotContext& ctx) { return ctx.board.Shots.size() == 2; },
    std::move(two_shots),
    leaf<BotContext, std::string>(""));

  return probThreshold<BotContext, std::string>(
    [](const BotContext& ctx) { return compute_roll_metrics(ctx).probSurvive; },
    [](const BotContext& ctx) -> double {
      double threshold = 0.62;
      int turnBrains = static_cast<int>(ctx.board.total_brains());
      int totalScore = static_cast<int>(ctx.botData.Brains) + turnBrains;
      int brainsToWin = static_cast<int>(ctx.winBrains) - totalScore;

      if (turnBrains >= 5) {
        threshold += 0.16;
      } else if (turnBrains >= 3) {
        threshold += 0.08;
      } else {
        threshold -= 0.10;
      }

      if (brainsToWin <= 2) {
        threshold -= 0.18;
      } else if (brainsToWin <= 4) {
        threshold -= 0.10;
      }

      if (ctx.winPlayers > 0) {
        threshold -= 0.12;
      }

      return clamp_val(threshold, 0.25, 0.92);
    },
    std::move(almost_dying),
    leaf<BotContext, std::string>("H"));
}