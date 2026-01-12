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

#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <curl/curl.h>
#include <stddef.h>

/* Generic HTTP response body capture structure (reusable, no protocol-specific knowledge) */
typedef struct {
    char body[4097];            /* Response body buffer (4KB + null terminator) */
    size_t body_length;         /* Actual bytes received */
    char content_type[256];     /* Content-Type header value */
    int has_body;               /* 0 = no body, 1 = has body */
} http_response_body;

/* ========== HTTP Response Body Capture Functions ========== */

/* Initialize a response body capture structure */
http_response_body* http_create_response_body(void);

/* Free a response body capture structure */
void http_free_response_body(http_response_body *body);

/* Capture Content-Type header into response body structure */
void http_capture_content_type(CURL *handle, http_response_body *body);

/* Generic callback to capture response body to buffer */
size_t http_capture_body_to_buffer(void *buffer, size_t size, size_t nmemb,
                                   http_response_body *body);

/* ========== HTTP Status Checking Functions ========== */

/* Get HTTP status code from CURL handle */
long http_get_status(CURL *handle);

/* Check if HTTP status indicates an error (>= 400) */
int http_is_error_status(long status);

#endif /* HTTP_UTILS_H */
