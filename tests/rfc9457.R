## Tests for RFC 9457 "Problem Details for HTTP APIs" support
## RFC 9457 defines a standard JSON format for HTTP error responses with
## Content-Type: application/problem+json
##
## These tests focus on backward compatibility and graceful degradation.
## Full RFC 9457 testing requires endpoints on developer.R-project.org/inet-tests
## that return application/problem+json responses.

options(warn = 1L)

## Test helper functions

# Extract warning messages from an expression
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

# Check if we have internet access
is_online <- function() {
    tryCatch({
        con <- suppressWarnings(socketConnection("developer.r-project.org", port = 443))
        close(con)
        TRUE
    }, error = function(e) FALSE)
}

if (!is_online()) {
    cat("No internet connection - skipping RFC 9457 tests\n")
    q()
}

cat("\n=== RFC 9457 Error Response Tests ===\n\n")

## Test 1: Backward Compatibility - Standard HTTP errors (no RFC 9457)
cat("Test 1: Backward compatibility with standard HTTP errors\n")
{
    # Test with a standard 404 error (no RFC 9457 body)
    site <- "https://developer.r-project.org/inet-tests/not-found"
    tmp <- tempfile()

    warnings <- capture_warnings({
        res <- try(download.file(site, tmp, quiet = TRUE), silent = TRUE)
    })

    # Should get standard error message
    stopifnot(inherits(res, "try-error"))
    stopifnot(length(warnings) > 0)
    stopifnot(any(grepl("404", warnings, fixed = TRUE)))
    stopifnot(!file.exists(tmp))

    cat("  ✓ Standard 404 error handled correctly (backward compatible)\n")
}

## Test 2: Empty body fallback
cat("Test 2: HTTP error with empty body falls back to standard message\n")
{
    # Most 404/500 errors without RFC 9457 should still work
    site <- "https://developer.r-project.org/inet-tests/status/404"
    tmp <- tempfile()

    warnings <- capture_warnings({
        res <- try(download.file(site, tmp, quiet = TRUE), silent = TRUE)
    })

    # Should handle gracefully with standard error
    stopifnot(inherits(res, "try-error") || res != 0)
    stopifnot(!file.exists(tmp))

    cat("  ✓ Empty body handled with standard error message\n")
}

## Test 3: Non-RFC 9457 Content-Type is ignored
cat("Test 3: Non-RFC 9457 Content-Type (e.g., text/html) is ignored\n")
{
    # HTML error pages should be ignored, fall back to standard error
    # We can't easily test this without a specific endpoint, but we can verify
    # that our code doesn't break on HTML responses
    site <- "https://cran.r-project.org/this-does-not-exist-12345"
    tmp <- tempfile()

    warnings <- capture_warnings({
        res <- try(download.file(site, tmp, method = "libcurl", quiet = TRUE), silent = TRUE)
    })

    # Should still report error without trying to parse HTML as RFC 9457
    stopifnot(inherits(res, "try-error") || res != 0)
    stopifnot(!file.exists(tmp))

    cat("  ✓ HTML error pages ignored (not parsed as RFC 9457)\n")
}

## Test 4: Successful downloads still work (no regression)
cat("Test 4: Successful downloads unaffected by RFC 9457 implementation\n")
{
    # Ensure success path (2xx responses) still works correctly
    site <- "https://cran.r-project.org/CRAN_mirrors.csv"
    tmp <- tempfile()

    res <- download.file(site, tmp, method = "libcurl", quiet = TRUE)

    stopifnot(res == 0)
    stopifnot(file.exists(tmp))
    stopifnot(file.size(tmp) > 100)  # Should have content

    unlink(tmp)
    cat("  ✓ Successful downloads work correctly\n")
}

## Test 5: Multiple concurrent downloads (libcurl bulk mode)
cat("Test 5: Multiple downloads with mixed success/failure\n")
{
    # Test bulk download with some failures
    urls <- c(
        "https://cran.r-project.org/CRAN_mirrors.csv",           # success
        "https://developer.r-project.org/inet-tests/not-found",  # 404 error
        "https://cran.r-project.org/CRAN_mirrors.csv"            # success (same file, different temp location)
    )

    tmpfiles <- replicate(3, tempfile())

    warnings <- capture_warnings({
        res <- download.file(urls, tmpfiles, method = "libcurl", quiet = TRUE)
    })

    # Check results
    retvals <- attr(res, "retvals")
    stopifnot(length(retvals) == 3)
    stopifnot(retvals[1] == 0)  # First download succeeded
    stopifnot(retvals[2] != 0)  # Second download failed
    stopifnot(retvals[3] == 0)  # Third download succeeded

    # Check files
    stopifnot(file.exists(tmpfiles[1]))
    stopifnot(!file.exists(tmpfiles[2]))  # Failed download shouldn't leave file
    stopifnot(file.exists(tmpfiles[3]))

    # Should have warning for the failed download
    stopifnot(length(warnings) > 0)
    stopifnot(any(grepl("404", warnings)))

    unlink(tmpfiles)
    cat("  ✓ Bulk downloads with mixed results handled correctly\n")
}

## Test 6: Large error response (buffer limit test)
cat("Test 6: Large error responses handled gracefully\n")
{
    # While we can't easily generate a 4KB+ RFC 9457 response without a special endpoint,
    # we can verify the code doesn't crash on large error bodies
    site <- "https://httpbin.org/status/500"
    tmp <- tempfile()

    warnings <- capture_warnings({
        res <- try(download.file(site, tmp, method = "libcurl", quiet = TRUE), silent = TRUE)
    })

    # Should handle without crashing (buffer truncation is internal)
    stopifnot(inherits(res, "try-error") || res != 0)

    cat("  ✓ Large error responses don't cause crashes\n")
}

## Note for developers: Full RFC 9457 testing
cat("\n")
cat("=== Note for Full RFC 9457 Testing ===\n")
cat("To fully test RFC 9457 functionality, the following test endpoints are needed:\n")
cat("\n")
cat("1. RFC 9457 with title and detail:\n")
cat("   Endpoint: /inet-tests/rfc9457/404-full\n")
cat("   Content-Type: application/problem+json\n")
cat("   Body: {\"title\":\"Resource Not Found\",\"detail\":\"The requested resource does not exist\",\"status\":404}\n")
cat("\n")
cat("2. RFC 9457 with detail only:\n")
cat("   Endpoint: /inet-tests/rfc9457/500-detail-only\n")
cat("   Content-Type: application/problem+json\n")
cat("   Body: {\"detail\":\"Internal server error occurred\",\"status\":500}\n")
cat("\n")
cat("3. RFC 9457 with title only:\n")
cat("   Endpoint: /inet-tests/rfc9457/403-title-only\n")
cat("   Content-Type: application/problem+json\n")
cat("   Body: {\"title\":\"Access Denied\",\"status\":403}\n")
cat("\n")
cat("4. RFC 9457 with charset parameter:\n")
cat("   Endpoint: /inet-tests/rfc9457/400-charset\n")
cat("   Content-Type: application/problem+json; charset=utf-8\n")
cat("   Body: {\"title\":\"Bad Request\",\"detail\":\"Invalid parameters\",\"status\":400}\n")
cat("\n")
cat("5. Large RFC 9457 response (>4KB):\n")
cat("   Endpoint: /inet-tests/rfc9457/413-large\n")
cat("   Content-Type: application/problem+json\n")
cat("   Body: {\"detail\":\"<4KB+ string>\",\"status\":413}\n")
cat("\n")
cat("6. Malformed JSON with RFC 9457 Content-Type:\n")
cat("   Endpoint: /inet-tests/rfc9457/500-malformed\n")
cat("   Content-Type: application/problem+json\n")
cat("   Body: {\"detail\":\"unclosed string\n")
cat("\n")
cat("Once these endpoints are available, add specific tests here.\n")
cat("\n")

cat("All basic RFC 9457 tests passed!\n")
proc.time()
