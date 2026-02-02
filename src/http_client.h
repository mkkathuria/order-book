#pragma once

#include <string>

// Perform an HTTPS GET using the curl CLI.
// Returns the response body on success.
// Throws std::runtime_error on failure.
std::string http_get(const std::string& url);
