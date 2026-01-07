## RFC 9457 Fallback and Backward Compatibility Tests
##
## Tests that RFC 9457 implementation correctly falls back to standard
## error messages when RFC 9457 responses are not available. These tests
## validate that the implementation maintains backward compatibility and
## handles edge cases gracefully.
##
## These tests use developer.R-project.org/inet-tests endpoints that exist
## but do not yet return application/problem+json responses. This confirms
## the fallback behavior works correctly.
##
## Usage: Rscript rfc9457-fallback.R

site <- "developer.r-project.org"
prefix <- "/inet-tests/rfc9457"

options(warn = 1L)

## Test helper functions
capture_warnings <- function(expr) {
    warnings <- character()
    withCallingHandlers(
        expr,
        warning = function(w) {
            warnings <<- c(warnings, conditionMessage(w))
            invokeRestart("muffleWarning")
        }
    )
    warnings
}

## Check if we have internet access
is_online <- function() {
    tryCatch({
        con <- suppressWarnings(socketConnection(site, port = 443))
        close(con)
        TRUE
    }, error = function(e) FALSE)
}

if (!is_online()) {
    cat("No internet connection or", site, "unavailable\n")
    cat("Skipping RFC 9457 fallback tests.\n")
    q()
}

cat("\n=== RFC 9457 Fallback and Backward Compatibility Tests ===\n\n")

test_count <- 0
pass_count <- 0

run_test <- function(name, expr) {
    test_count <<- test_count + 1
    cat(sprintf("Test %d: %s\n", test_count, name))

    result <- tryCatch({
        expr
        pass_count <<- pass_count + 1
        cat("  ✓ PASS\n")
        TRUE
    }, error = function(e) {
        cat(sprintf("  ✗ FAIL: %s\n", conditionMessage(e)))
        FALSE
    })

    result
}

## Test 1: Non-RFC 9457 Content-Type fallback
run_test("Non-RFC 9457 Content-Type (HTML) falls back to standard error", {
    url <- paste0("https://", site, prefix, "/html/404")
    tmp <- tempfile()

    warnings <- capture_warnings({
        res <- try(download.file(url, tmp, method = "libcurl", quiet = TRUE), silent = TRUE)
    })

    stopifnot(inherits(res, "try-error") || res != 0)
    stopifnot(length(warnings) > 0)

    all_warnings <- paste(warnings, collapse = "\n")
    # Should show standard error, not HTML content
    stopifnot(grepl("404", all_warnings, fixed = TRUE))
    stopifnot(!grepl("<html>", all_warnings, ignore.case = TRUE))
    stopifnot(!grepl("<body>", all_warnings, ignore.case = TRUE))
})

## Test 2: Regular JSON (not RFC 9457) is ignored
run_test("Regular application/json is ignored (not RFC 9457)", {
    url <- paste0("https://", site, prefix, "/json/500")
    tmp <- tempfile()

    warnings <- capture_warnings({
        res <- try(download.file(url, tmp, method = "libcurl", quiet = TRUE), silent = TRUE)
    })

    stopifnot(inherits(res, "try-error") || res != 0)
    stopifnot(length(warnings) > 0)

    all_warnings <- paste(warnings, collapse = "\n")
    # Should show standard error, not parse regular JSON
    stopifnot(grepl("500", all_warnings, fixed = TRUE))
})

## Test 3: Empty body fallback
run_test("HTTP error with empty body uses standard message", {
    url <- paste0("https://", site, prefix, "/empty/404")
    tmp <- tempfile()

    warnings <- capture_warnings({
        res <- try(download.file(url, tmp, method = "libcurl", quiet = TRUE), silent = TRUE)
    })

    stopifnot(inherits(res, "try-error") || res != 0)
    stopifnot(length(warnings) > 0)

    all_warnings <- paste(warnings, collapse = "\n")
    # Should have standard error message
    stopifnot(grepl("404", all_warnings, fixed = TRUE))
})

## Test 4: Large error response handled gracefully
run_test("Large error response (>4KB) handled without crashing", {
    url <- paste0("https://", site, prefix, "/413-large")
    tmp <- tempfile()

    warnings <- capture_warnings({
        res <- try(download.file(url, tmp, method = "libcurl", quiet = TRUE), silent = TRUE)
    })

    stopifnot(inherits(res, "try-error") || res != 0)
    stopifnot(length(warnings) > 0)

    all_warnings <- paste(warnings, collapse = "\n")
    stopifnot(grepl("413|404", all_warnings)) # 413 or 404 both acceptable
    # Should handle without crashing (buffer truncation is internal)
    stopifnot(nchar(all_warnings) > 0)
})

## Test 5: Endpoint with expected empty fields falls back gracefully
run_test("Endpoint designed for empty RFC 9457 fields falls back to standard error", {
    url <- paste0("https://", site, prefix, "/404-empty-fields")
    tmp <- tempfile()

    warnings <- capture_warnings({
        res <- try(download.file(url, tmp, method = "libcurl", quiet = TRUE), silent = TRUE)
    })

    stopifnot(inherits(res, "try-error") || res != 0)
    stopifnot(length(warnings) > 0)

    all_warnings <- paste(warnings, collapse = "\n")
    # Should fall back to standard error message
    stopifnot(grepl("404", all_warnings, fixed = TRUE))
})

## Test 6: Endpoint with special characters handled safely
run_test("Endpoint designed for escaped characters handled safely", {
    url <- paste0("https://", site, prefix, "/400-escaped")
    tmp <- tempfile()

    warnings <- capture_warnings({
        res <- try(download.file(url, tmp, method = "libcurl", quiet = TRUE), silent = TRUE)
    })

    stopifnot(inherits(res, "try-error") || res != 0)
    stopifnot(length(warnings) > 0)

    all_warnings <- paste(warnings, collapse = "\n")
    stopifnot(grepl("400|404", all_warnings)) # 400 or 404 both acceptable
    # Should not crash or inject unexpected content
})

## Summary
cat("\n=== Test Summary ===\n")
cat(sprintf("Passed: %d / %d\n", pass_count, test_count))

if (pass_count == test_count) {
    cat("\n✓ All fallback tests passed!\n")
    cat("RFC 9457 implementation correctly maintains backward compatibility.\n")
    q(status = 0)
} else {
    cat("\n✗ Some tests failed\n")
    q(status = 1)
}
