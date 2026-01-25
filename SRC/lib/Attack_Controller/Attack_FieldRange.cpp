/**
 * @file Attack_FieldRange.cpp
 * @brief Field range mappings for Attack_Controller
 *
 * This file provides field range lookup functionality for the impersonation
 * attack module. It delegates to centralized PGN helpers to retrieve the
 * valid minimum and maximum values for each editable field in supported PGNs.

 */

#include "Attack_Controller.h"
#include <PGN_Helpers.h>

/**
 * @brief Gets the valid range for a specific field in a PGN
 *
 * Retrieves the minimum and maximum valid values for a field within a
 * specified PGN. These ranges are used to map the analog sensor input
 * (0-1023) to realistic values for the impersonation attack.
 *
 * @param pgn The PGN number to look up
 * @param fieldIndex The index of the field within the PGN (0-based)
 * @param[out] min Reference to store the minimum valid value
 * @param[out] max Reference to store the maximum valid value
 *
 * @note This function delegates to the centralized getPGNFieldRange helper
 * to maintain consistency across the codebase. Ranges are defined based on
 * NMEA2000 specification limits and practical maritime sensor ranges.
 */
void Attack_Controller::getFieldRange(uint32_t pgn, int fieldIndex, float &min, float &max) {
    // Delegate to centralized helper function
    getPGNFieldRange(pgn, fieldIndex, min, max);
}
