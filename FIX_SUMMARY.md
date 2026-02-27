# Vulnerability Fixes and CI Improvements Summary

This document summarizes the security patches and build system improvements implemented for the `ttya` project.

## 1. Security Fixes

### Path Traversal Vulnerability
*   **Issue**: The application was vulnerable to path traversal attacks via the `/download_token` and `/upload` endpoints. Malicious actors could access or overwrite files outside the intended directory using sequences like `../../`.
*   **Fix**: Implemented a `check_path` utility function in `src/utils.c`.
    *   This function uses `realpath` (or `_fullpath` on Windows) to resolve the absolute canonical path of the requested file and the server's working directory.
    *   It strictly verifies that the resolved file path starts with the resolved working directory path.
    *   Applied this check to both file download and upload handlers in `src/http.c`.
*   **Verification**: Verified using a custom python exploit script. Attempts to access `/tmp/secret.txt` now return `403 Forbidden`, while valid files in the working directory remain accessible.

### Double Free Vulnerability
*   **Issue**: A double free vulnerability existed in `src/http.c` within the `pss_buffer_free` function, potentially causing the server to crash under specific HTTP request conditions.
*   **Fix**: Updated `pss_buffer_free` to set the buffer pointer (`pss->buffer`) to `NULL` immediately after freeing the memory. This prevents subsequent attempts to free the same memory address.
*   **Verification**: Verified that the server remains stable and does not crash during the exploit tests that previously triggered the issue.

## 2. CI/Build System Fixes

### Windows Build Compatibility
*   **Issue**: The initial fix introduced `realpath`, which is a POSIX standard function and not available on Windows (MinGW), causing the Windows CI build to fail.
*   **Fix**: Modified `src/utils.c` to use conditional compilation.
    *   On Windows (`_WIN32`), it now uses `_fullpath`.
    *   On other platforms, it continues to use `realpath`.
    *   Adjusted path separator checks to handle both `/` and `\` on Windows.

### Zlib Version Error
*   **Issue**: The cross-compilation script `scripts/cross-build.sh` was attempting to download zlib version `1.3.1` from `zlib.net`. Since `1.3.1` is no longer the latest version, it was moved to a `fossils/` subdirectory, causing a 404 Not Found error during the CI build.
*   **Fix**: Updated the `ZLIB_VERSION` variable in `scripts/cross-build.sh` to `1.3.2`, which is the current stable release available at the root URL.

### Docker Build Stability
*   **Issue**: The CI build reported slow Docker image pulls or potential timeouts.
*   **Fix**: Pinned the `tsl0922/musl-cross` Docker image to a specific digest (`sha256:a397edeb...`) in `.github/workflows/build.yml`. This improves build reproducibility and eliminates the overhead of checking for image updates on every run.

## 3. Verification

All changes have been verified through:
1.  **Automated Exploit Testing**: Confirmed security fixes works as expected.
2.  **Local Compilation**: Verified that the code compiles successfully on Linux.
3.  **Frontend Verification**: Used Playwright to confirm that the web interface loads correctly and the terminal is functional after the backend changes.
