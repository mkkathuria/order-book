#include "http_client.h"
#include <cstdio>
#include <stdexcept>

std::string http_get(const std::string &url)
{
#ifdef _WIN32
    // Windows: curl needs different quoting, and we must explicitly handle stderr
    std::string cmd = "curl -s --fail \"" + url + "\" 2>nul";
#else
    // Linux/Mac: original version
    std::string cmd = "curl -s --fail '" + url + "'";
#endif
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        throw std::runtime_error("popen failed");

    std::string result;
    char buf[4096];
    while (size_t n = fread(buf, 1, sizeof(buf), pipe))
        result.append(buf, n);

    int status = pclose(pipe);
    if (status != 0)
        throw std::runtime_error("curl failed for " + url);

    return result;
}
