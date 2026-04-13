#include <fstream>
#include <iostream>

#include "ImageApiClient.h"
#include "Operations.h"

int main()
{
    // 로컬 또는 원격 이미지 API 서버 주소
    image_api::ImageApiClient client("http://3.106.215.74:8080");

    // 1) 서버 health check
    image_api::HealthCheckResponse health;
    image_api::ApiResult healthResult = client.HealthCheck(health);

    if (!healthResult.success)
    {
        std::cerr << "HealthCheck failed: " << healthResult.message << '\n';
        return 1;
    }

    std::cout << "Health status: " << health.status << '\n';
    std::cout << "Health message: " << health.message << '\n';

    // 2) 이미지 처리 요청 구성
    image_api::ProcessImageRequest request;
    request.imageUrl = "https://www.geoje.go.kr/upload_data/photodb/thumb/2025010621142591973.jpg";
    request.outputFormat = "jpg";

    // 서버에 순서대로 적용될 작업들
    request.operations.push_back(image_api::MakeGrayscale());
    request.operations.push_back(image_api::MakeBlur(9, 2.0));
    request.operations.push_back(image_api::MakeResize(512, 512));

    // 3) 처리 요청 호출
    image_api::ProcessImageResponse response;
    image_api::ApiResult processResult = client.ProcessImage(request, response);

    if (!processResult.success)
    {
        std::cerr << "ProcessImage failed: " << processResult.message << '\n';
        return 1;
    }

    std::cout << "Process success\n";
    std::cout << "Content-Type: " << response.contentType << '\n';
    std::cout << "Width: " << response.width << '\n';
    std::cout << "Height: " << response.height << '\n';
    std::cout << "Bytes: " << response.imageBytes.size() << '\n';

    // 4) 결과 파일 저장
    std::ofstream output("result_image.png", std::ios::binary);
    output.write(reinterpret_cast<const char*>(response.imageBytes.data()),
                 static_cast<std::streamsize>(response.imageBytes.size()));
    output.close();

    std::cout << "Saved result_image.png\n";
    return 0;
}
