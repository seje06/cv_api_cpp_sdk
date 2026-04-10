#include "ImageApiClient.h"

#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

#include "HttpUtils.h"

namespace image_api
{
    using json = nlohmann::json;

    ImageApiClient::ImageApiClient(const std::string& baseUrl)
        : _baseUrl(baseUrl)
    {
    }

    void ImageApiClient::SetBaseUrl(const std::string& baseUrl)
    {
        _baseUrl = baseUrl;
    }

    const std::string& ImageApiClient::GetBaseUrl() const
    {
        return _baseUrl;
    }

    std::string ImageApiClient::BuildUrl(const std::string& path) const
    {
        if (_baseUrl.empty())
        {
            throw std::runtime_error("Base URL is empty");
        }

        if (!path.empty() && path.front() == '/')
        {
            if (!_baseUrl.empty() && _baseUrl.back() == '/')
            {
                return _baseUrl.substr(0, _baseUrl.size() - 1) + path;
            }

            return _baseUrl + path;
        }

        if (!_baseUrl.empty() && _baseUrl.back() == '/')
        {
            return _baseUrl + path;
        }

        return _baseUrl + "/" + path;
    }

    ApiResult ImageApiClient::HealthCheck(HealthCheckResponse& outResponse) const
    {
        try
        {
            internal::HttpResponse response;
            std::string errorMessage;

            if (!internal::HttpGet(BuildUrl("/health"), response, errorMessage))
            {
                return ApiResult::Fail(errorMessage);
            }

            if (response.statusCode != 200)
            {
                return ApiResult::Fail(internal::ParseServerErrorMessage(response));
            }

            std::string bodyText(response.body.begin(), response.body.end());
            json body = json::parse(bodyText);

            if (body.contains("status") && body["status"].is_string())
            {
                outResponse.status = body["status"].get<std::string>();
            }

            if (body.contains("message") && body["message"].is_string())
            {
                outResponse.message = body["message"].get<std::string>();
            }

            return ApiResult::Ok("health check success");
        }
        catch (const std::exception& e)
        {
            return ApiResult::Fail(e.what());
        }
    }

    ApiResult ImageApiClient::ProcessImage(const ProcessImageRequest& request, ProcessImageResponse& outResponse) const
    {
        try
        {
            // 요청값 기초 검증
            if (request.imageUrl.empty())
            {
                return ApiResult::Fail("imageUrl is empty");
            }

            if (request.operations.empty())
            {
                return ApiResult::Fail("operations is empty");
            }

            // 서버가 기대하는 JSON 요청 본문 구성
            json requestBody;
            requestBody["image_url"] = request.imageUrl;
            requestBody["output_format"] = request.outputFormat;
            requestBody["operations"] = json::array();

            for (const Operation& operation : request.operations)
            {
                json op;
                op["type"] = operation.type;

                if (!operation.optionsJson.empty())
                {
                    op["options"] = json::parse(operation.optionsJson);
                }

                requestBody["operations"].push_back(op);
            }

            std::string requestText = requestBody.dump();

            internal::HttpResponse response;
            std::string errorMessage;

            if (!internal::HttpPostJson(BuildUrl("/api/v1/process"), requestText, response, errorMessage))
            {
                return ApiResult::Fail(errorMessage);
            }

            if (response.statusCode != 200)
            {
                return ApiResult::Fail(internal::ParseServerErrorMessage(response));
            }

            // 성공 응답이면 body 전체가 결과 이미지 파일 바이트이다.
            outResponse.imageBytes = std::move(response.body);
            outResponse.contentType = response.contentType;
            outResponse.width = response.imageWidth;
            outResponse.height = response.imageHeight;

            return ApiResult::Ok("image processed successfully");
        }
        catch (const std::exception& e)
        {
            return ApiResult::Fail(e.what());
        }
    }
}
