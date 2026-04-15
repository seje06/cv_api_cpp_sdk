#include "HttpUtils.h"

#include <algorithm>
#include <cctype>
#include <mutex>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace image_api::internal
{
    using json = nlohmann::json;

    namespace
    {
        // libcurl 전역 초기화는 프로세스 전체에서 한 번만 하면 된다.
        std::once_flag g_curlInitOnce;
        bool g_curlInitSucceeded = false;
        std::string g_curlInitError;

        /*
            ----------------------------------------------------------------
            Trim
            ----------------------------------------------------------------
            HTTP 헤더 값 파싱 후 앞뒤 공백과 개행 문자를 제거하기 위한 보조 함수
        */
        std::string Trim(const std::string& value)
        {
            auto isSpace = [](unsigned char ch)
            {
                return std::isspace(ch) != 0;
            };

            auto begin = std::find_if_not(value.begin(), value.end(), isSpace);
            auto end = std::find_if_not(value.rbegin(), value.rend(), isSpace).base();

            if (begin >= end)
                return {};

            return std::string(begin, end);
        }

        /*
            ----------------------------------------------------------------
            WriteBodyCallback
            ----------------------------------------------------------------
            libcurl이 응답 본문(body)을 조각 단위로 넘길 때 호출되는 콜백 함수

            중요한 점:
            - 응답은 한 번에 다 오지 않을 수 있다.
            - 따라서 vector 뒤에 계속 append 해야 전체 body가 완성된다.
        */
        size_t WriteBodyCallback(void* contents, size_t size, size_t nmemb, void* userp)
        {
            size_t totalSize = size * nmemb;

            auto* body = static_cast<std::vector<unsigned char>*>(userp);
            auto* data = static_cast<unsigned char*>(contents);

            body->insert(body->end(), data, data + totalSize);
            return totalSize;
        }

        /*
            ----------------------------------------------------------------
            WriteHeaderCallback
            ----------------------------------------------------------------
            libcurl이 헤더 한 줄을 넘길 때마다 호출되는 콜백 함수

            여기서는 우리가 관심 있는 헤더만 추출한다.
            - Content-Type
            - X-Image-Width
            - X-Image-Height
        */
        size_t WriteHeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata)
        {
            size_t totalSize = size * nitems;
            auto* response = static_cast<HttpResponse*>(userdata);

            std::string headerLine(buffer, totalSize);

            const std::string contentTypePrefix = "Content-Type:";
            const std::string widthPrefix = "X-Image-Width:";
            const std::string heightPrefix = "X-Image-Height:";

            if (headerLine.rfind(contentTypePrefix, 0) == 0)
            {
                response->contentType = Trim(headerLine.substr(contentTypePrefix.size()));
            }
            else if (headerLine.rfind(widthPrefix, 0) == 0)
            {
                response->imageWidth = std::stoi(Trim(headerLine.substr(widthPrefix.size())));
            }
            else if (headerLine.rfind(heightPrefix, 0) == 0)
            {
                response->imageHeight = std::stoi(Trim(headerLine.substr(heightPrefix.size())));
            }

            return totalSize;
        }

        /*
            ----------------------------------------------------------------
            PerformRequest
            ----------------------------------------------------------------
            GET / POST에서 공통으로 사용하는 실제 요청 수행 함수

            역할:
            - CURL easy handle 생성
            - URL, 콜백, 타임아웃 등 공통 옵션 설정
            - 필요 시 POST body / Content-Type 설정
            - 응답 status code 와 body/headers 수집
        */
        bool PerformRequest(const std::string& url,
                            const char* method,
                            const std::string* requestBody,
                            HttpResponse& outResponse,
                            std::string& outErrorMessage)
        {
            CURL* curl = curl_easy_init();
            if (!curl)
            {
                outErrorMessage = "Failed to initialize CURL easy handle";
                return false;
            }

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteBodyCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outResponse.body); // body가 아마 writeBodyCallback에 들어가는 userp
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteHeaderCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &outResponse);

            struct curl_slist* headers = nullptr;

            if (std::string(method) == "POST")
            {
                headers = curl_slist_append(headers, "Content-Type: application/json");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody->c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(requestBody->size()));
            }

            CURLcode result = curl_easy_perform(curl);
            if (result != CURLE_OK)
            {
                outErrorMessage = curl_easy_strerror(result);

                if (headers)
                    curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
                return false;
            }

            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &outResponse.statusCode);

            if (headers)
                curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return true;
        }
    }

    bool EnsureCurlGlobalInit(std::string& outErrorMessage)
    {
        std::call_once(g_curlInitOnce, []()
        {
            CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
            if (code == CURLE_OK)
            {
                g_curlInitSucceeded = true;
            }
            else
            {
                g_curlInitSucceeded = false;
                g_curlInitError = curl_easy_strerror(code);
            }
        });

        if (!g_curlInitSucceeded)
        {
            outErrorMessage = g_curlInitError.empty() ? "curl_global_init failed" : g_curlInitError;
            return false;
        }

        return true;
    }

    bool HttpGet(const std::string& url, HttpResponse& outResponse, std::string& outErrorMessage)
    {
        if (!EnsureCurlGlobalInit(outErrorMessage))
            return false;

        return PerformRequest(url, "GET", nullptr, outResponse, outErrorMessage);
    }

    bool HttpPostJson(const std::string& url,
                      const std::string& jsonBody,
                      HttpResponse& outResponse,
                      std::string& outErrorMessage)
    {
        if (!EnsureCurlGlobalInit(outErrorMessage))
            return false;

        return PerformRequest(url, "POST", &jsonBody, outResponse, outErrorMessage);
    }

    std::string ParseServerErrorMessage(const HttpResponse& response)
    {
        try
        {
            std::string text(response.body.begin(), response.body.end());
            json body = json::parse(text);

            // 서버 에러 응답이 {"error":"...", "detail":"..."} 형태라는 가정
            if (body.contains("error") && body["error"].is_string())
            {
                std::string message = body["error"].get<std::string>();

                if (body.contains("detail") && body["detail"].is_string())
                {
                    message += ": ";
                    message += body["detail"].get<std::string>();
                }

                return message;
            }
        }
        catch (...)
        {
            // JSON 파싱 실패 시 아래 fallback 문자열 사용
        }

        return "HTTP " + std::to_string(response.statusCode) + " request failed";
    }
}
