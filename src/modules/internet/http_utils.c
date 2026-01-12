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

#include "http_utils.h"
#include <stdlib.h>
#include <string.h>

/* ========== HTTP Response Body Capture Implementation ========== */

/* Initialize a response body capture structure */
http_response_body* http_create_response_body(void)
{
    http_response_body *body = (http_response_body *)calloc(1, sizeof(http_response_body));
    if (body) {
        body->body[0] = '\0';
        body->body_length = 0;
        body->content_type[0] = '\0';
        body->has_body = 0;
    }
    return body;
}

/* Free a response body capture structure */
void http_free_response_body(http_response_body *body)
{
    if (body) {
        free(body);
    }
}

/* Capture Content-Type header into response body structure */
void http_capture_content_type(CURL *handle, http_response_body *body)
{
    char *content_type = NULL;
    if (body && curl_easy_getinfo(handle, CURLINFO_CONTENT_TYPE,
                                  &content_type) == CURLE_OK && content_type) {
        strncpy(body->content_type, content_type, sizeof(body->content_type) - 1);
        body->content_type[sizeof(body->content_type) - 1] = '\0';
    }
}

/* Generic callback to capture response body to buffer */
size_t http_capture_body_to_buffer(void *buffer, size_t size, size_t nmemb,
                                   http_response_body *body)
{
    size_t total_size = size * nmemb;

    if (!body) return 0; /* Signal error to libcurl */

    /* Calculate available space in buffer */
    size_t space_left = sizeof(body->body) - body->body_length - 1;

    if (space_left > 0) {
        size_t copy_size = (total_size < space_left) ? total_size : space_left;
        memcpy(body->body + body->body_length, buffer, copy_size);
        body->body_length += copy_size;
        body->body[body->body_length] = '\0';
        body->has_body = 1;
    }

    /* Always return total_size even if buffer full (accept all data) */
    return total_size;
}

/* ========== HTTP Status Checking Implementation ========== */

/* Get HTTP status code from CURL handle */
long http_get_status(CURL *handle)
{
    long status = 0;
    if (handle && curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &status) == CURLE_OK) {
        return status;
    }
    return 0;
}

/* Check if HTTP status indicates an error (>= 400) */
int http_is_error_status(long status)
{
    return (status >= 400);
}
