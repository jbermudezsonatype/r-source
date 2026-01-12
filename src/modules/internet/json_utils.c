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

#include "json_utils.h"
#include <string.h>
#include <stdio.h>

/* ========== JSON Parsing Implementation ========== */

/*
 * Extract a string field from JSON.
 * Looks for "field_name": "value" pattern.
 * Returns 1 if found and extracted, 0 otherwise.
 */
int json_extract_string(const char *json, const char *field_name,
                        char *output, size_t output_size)
{
    if (!json || !field_name || !output || output_size == 0) return 0;

    /* Look for "field_name" - search for exact field match */
    char search_pattern[128];
    snprintf(search_pattern, sizeof(search_pattern), "\"%s\"", field_name);
    size_t pattern_len = strlen(search_pattern);

    const char *field_pos = json;
    const char *colon_pos = NULL;
    const char *value_start = NULL;

    while ((field_pos = strstr(field_pos, search_pattern)) != NULL) {
        /* Ensure the quote before field_name starts a new key
           Check that it's preceded by: {, comma, or whitespace */
        if (field_pos == json || field_pos[-1] == '{' || field_pos[-1] == ',' ||
            field_pos[-1] == ' ' || field_pos[-1] == '\t' ||
            field_pos[-1] == '\n' || field_pos[-1] == '\r') {
            /* Found valid field, now look for colon */
            colon_pos = strchr(field_pos + pattern_len, ':');
            if (colon_pos) {
                /* Found colon, skip whitespace after it */
                value_start = colon_pos + 1;
                while (*value_start == ' ' || *value_start == '\t' ||
                       *value_start == '\n' || *value_start == '\r') {
                    value_start++;
                }
                /* Check if value is a string (starts with ") */
                if (*value_start == '"') {
                    value_start++; /* Skip opening quote */
                    break; /* Found our field */
                }
            }
        }
        field_pos += pattern_len;
    }

    if (!value_start || value_start[-1] != '"') return 0; /* Field not found or not a string */

    /* Copy until closing quote (handling escaped quotes) */
    size_t i = 0;
    int escaped = 0;
    while (i < output_size - 1 && *value_start != '\0') {
        if (escaped) {
            /* Handle simple escapes */
            switch (*value_start) {
                case 'n': output[i++] = '\n'; break;
                case 't': output[i++] = '\t'; break;
                case 'r': output[i++] = '\r'; break;
                case '"': output[i++] = '"'; break;
                case '\\': output[i++] = '\\'; break;
                default: output[i++] = *value_start;
            }
            escaped = 0;
        } else if (*value_start == '\\') {
            escaped = 1;
        } else if (*value_start == '"') {
            /* Found closing quote */
            output[i] = '\0';
            return 1;
        } else {
            output[i++] = *value_start;
        }
        value_start++;
    }

    output[i] = '\0';
    return 0; /* Didn't find closing quote */
}
