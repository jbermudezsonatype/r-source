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

#include "rfc9457.h"
#include "json_utils.h"
#include <stdlib.h>
#include <string.h>

/* ========== Lifecycle Implementation ========== */

/*
 * Create and initialize an RFC 9457 error context
 * Returns NULL on allocation failure
 */
rfc9457_error_context* rfc9457_create_context(void)
{
    rfc9457_error_context *ctx = (rfc9457_error_context *)calloc(1, sizeof(rfc9457_error_context));
    if (!ctx) return NULL;

    /* Allocate generic response body capture */
    ctx->response_body = http_create_response_body();
    if (!ctx->response_body) {
        free(ctx);
        return NULL;
    }

    /* Initialize RFC 9457-specific fields */
    ctx->title[0] = '\0';
    ctx->detail[0] = '\0';
    ctx->status = 0;
    ctx->is_rfc9457 = 0;

    return ctx;
}

/*
 * Free an RFC 9457 error context and its associated resources
 */
void rfc9457_free_context(rfc9457_error_context *ctx)
{
    if (ctx) {
        if (ctx->response_body) {
            http_free_response_body(ctx->response_body);
        }
        free(ctx);
    }
}

/* ========== Parsing Implementation ========== */

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
int rfc9457_parse_error(rfc9457_error_context *ctx)
{
    if (!ctx || !ctx->response_body || !ctx->response_body->has_body ||
        ctx->response_body->body_length == 0) {
        return 0;
    }

    /* Check Content-Type for application/problem+json
       Match at start or after whitespace/semicolon to handle:
       - "application/problem+json"
       - "application/problem+json; charset=utf-8"
       But not:  "text/application/problem+json" */
    const char *ct = ctx->response_body->content_type;
    const char *match = strstr(ct, "application/problem+json");
    if (!match || (match != ct && match[-1] != ' ' && match[-1] != '\t')) {
        /* Not RFC 9457 format - ignore body and fall back to standard message */
        ctx->is_rfc9457 = 0;
        return 0;
    }

    ctx->is_rfc9457 = 1;

    /* Extract "detail" field (most important for users) */
    int has_detail = json_extract_string(ctx->response_body->body, "detail",
                                         ctx->detail, sizeof(ctx->detail));

    /* Extract "title" field (fallback if no detail) */
    int has_title = json_extract_string(ctx->response_body->body, "title",
                                        ctx->title, sizeof(ctx->title));

    return (has_detail || has_title) ? 1 : 0;
}
