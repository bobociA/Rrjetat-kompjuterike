#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <curl/curl.h>

std::string headerBuffer;

size_t HeaderCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    headerBuffer.append((char*)contents, totalSize);
    return totalSize;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    return size * nmemb;
}

void analyzeTarget(const std::string& baseUrl) {
    std::cout << "\n=== Analizë e Target-it: " << baseUrl << " ===\n" << std::flush;

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cout << "Gabim: Nuk mund të inicializohet CURL\n";
        return;
    }

    headerBuffer.clear();

    curl_easy_setopt(curl, CURLOPT_URL, baseUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);           
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "MSc-Security-Scanner/1.0");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);     
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        char* finalUrl = nullptr;
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &finalUrl);

        std::cout << "Status Code     : " << httpCode << "\n";
        if (finalUrl) 
            std::cout << "Final URL       : " << finalUrl << "\n";

        bool isHttps = (finalUrl && std::string(finalUrl).find("https://") == 0);
        std::cout << "Protokolli      : " << (isHttps ? "HTTPS  (i sigurt)" : "HTTP  (i pasigurt)") << "\n";

        std::cout << "\n=== Kontroll i Security Headers ===\n";

        std::map<std::string, std::string> importantHeaders = {
            {"Strict-Transport-Security", "Mbron nga SSL Stripping dhe MITM"},
            {"Content-Security-Policy",   "Mbron nga XSS dhe injeksione"},
            {"X-Frame-Options",           "Mbron nga Clickjacking"},
            {"X-Content-Type-Options",    "Mbron nga MIME Sniffing"},
            {"Referrer-Policy",           "Kontrollon referrer"}
        };

        int found = 0;
        for (const auto& pair : importantHeaders) {
            if (headerBuffer.find(pair.first) != std::string::npos) {
                std::cout << " ✓ " << pair.first << " është i pranishëm\n";
                found++;
            } else {
                std::cout << " ✗ Mungojnë: " << pair.first << " (" << pair.second << ")\n";
            }
        }

        if (headerBuffer.find("Server:") != std::string::npos)
            std::cout << " ⚠ Header 'Server' i pranishëm\n";
        if (headerBuffer.find("X-Powered-By:") != std::string::npos)
            std::cout << " ⚠ Header 'X-Powered-By' i pranishëm\n";

        std::cout << "\nTotal security headers të gjetur: " << found << "/5\n";

    } else {
        std::cout << "Gabim gjatë lidhjes: " << curl_easy_strerror(res) << "\n";
    }

    curl_easy_cleanup(curl);

    std::cout << "\n=== Kontroll bazë i Path-eve të Ekspozuara ===\n";
    std::vector<std::string> commonPaths = {"/admin", "/login", "/phpinfo.php", "/.git", "/config"};

    for (const auto& path : commonPaths) {
        std::string testUrl = baseUrl;
        if (testUrl.back() != '/') testUrl += "/";
        testUrl += path.substr(1);

        CURL* curlPath = curl_easy_init();
        if (curlPath) {
            curl_easy_setopt(curlPath, CURLOPT_URL, testUrl.c_str());
            curl_easy_setopt(curlPath, CURLOPT_NOBODY, 1L);
            curl_easy_setopt(curlPath, CURLOPT_TIMEOUT, 10L);
            CURLcode pathRes = curl_easy_perform(curlPath);
            long pathCode = 0;
            if (pathRes == CURLE_OK) {
                curl_easy_getinfo(curlPath, CURLINFO_RESPONSE_CODE, &pathCode);
                std::cout << " " << testUrl << " → " << pathCode 
                          << (pathCode == 200 ? " (potencialisht i ekspozuar!)" : "") << "\n";
            } else {
                std::cout << " " << testUrl << " → Gabim\n";
            }
            curl_easy_cleanup(curlPath);
        }
    }
}

int main() {
    std::cout << "=== Simple Web Security Scanner (C++ + libcurl) ===\n";
    std::cout << "Projekt edukativ për MSc Siguri Informacioni\n\n";

    std::vector<std::string> targets = {
        "http://testphp.vulnweb.com/",
        "https://demo.owasp-juice.shop/"
    };

    for (const auto& url : targets) {
        analyzeTarget(url);
    }

    std::cout << "\n=== Analiza përfundoi ===\n";
    return 0;
}
