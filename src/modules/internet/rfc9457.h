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

#ifndef RFC9457_H
#define RFC9457_H

#include "http_utils.h"

/*
 * RFC 9457 "Problem Details for HTTP APIs" support
 *
 * This module provides parsing and handling of RFC 9457 structured error
 * responses with Content-Type: application/problem+json
 *
 * Modular dependencies:
 * - http_utils.h: HTTP body capture (http_response_body)
 * - json_utils.h: JSON parsing (json_extract_string)
 */

/* ========== RFC 9457 Error Context ========== */

/* RFC 9457-specific error context */
typedef struct {
    http_response_body *response_body; /* Generic body capture from http_utils */
    char title[257];            /* RFC 9457 "title" field */
    char detail[513];           /* RFC 9457 "detail" field (most important) */
    long status;                /* HTTP status from actual response */
    int is_rfc9457;             /* 0 = not RFC 9457, 1 = RFC 9457 JSON */
} rfc9457_error_context;

/* ========== Lifecycle Functions ========== */

/*
 * Create and initialize an RFC 9457 error context
 * Returns NULL on allocation failure
 */
rfc9457_error_context* rfc9457_create_context(void);

/*
 * Free an RFC 9457 error context and its associated resources
 */
void rfc9457_free_context(rfc9457_error_context *ctx);

/* ========== Parsing Functions ========== */

/*
 * Parse RFC 9457 "Problem Details" error body
 *
 * Checks if response has Content-Type: application/problem+json
 * Extracts 'title' and 'detail' fields using json_extract_string()
 *
 * Parameters:
 *   ctx - RFC 9457 error context containing captured response body
 *
 * Returns:
 *   1 if RFC 9457 format detected and fields extracted
 *   0 if not RFC 9457 format or no useful fields found
 */
int rfc9457_parse_error(rfc9457_error_context *ctx);

#endif /* RFC9457_H */
