/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2024-2026  The R Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  https://www.R-project.org/Licenses/
 */

#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <stddef.h>

/* ========== JSON Parsing Functions ========== */

/*
 * Extract a string field from JSON.
 *
 * Looks for "field_name": "value" pattern and extracts the value.
 * Handles escape sequences: \n, \t, \r, \", \\
 *
 * Parameters:
 *   json        - JSON string to parse
 *   field_name  - Name of the field to extract (without quotes)
 *   output      - Buffer to store the extracted value
 *   output_size - Size of the output buffer
 *
 * Returns:
 *   1 if field found and extracted successfully
 *   0 if field not found, not a string, or parse error
 *
 * Example:
 *   char title[256];
 *   int found = json_extract_string(json, "title", title, sizeof(title));
 */
int json_extract_string(const char *json, const char *field_name,
                        char *output, size_t output_size);

#endif /* JSON_UTILS_H */
