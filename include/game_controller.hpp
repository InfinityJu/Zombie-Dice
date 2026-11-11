#ifndef GAME_CONTROLLER_HPP
#define GAME_CONTROLLER_HPP

#include "setup_interface.hpp"
#include <algorithm>
#include <random>
#include <sstream>

// --- Helper Function --- //
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
std::string dice_color(Dice d);

#endif