# cv_api_cpp_sdk

이미지 처리를 해주는 cv api 서버를 활용하기 위한 C++ 라이브러리 개발

## 세팅
- 개발 환경은  Window, VS2026, C++20

1. Visual Studio / CMake / vcpkg 준비

```bash
git clone https://github.com/microsoft/vcpkg C:\vcpkg
cd /d C:\vcpkg
bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install curl:x64-windows nlohmann-json:x64-windows
```

2. 프로젝트 열기

- Visual Studio에서 `cv_api_cpp_sdk` 폴더 열기
- 또는 Developer Command Prompt 에서 직접 configure/build 진행

3. CMake configure

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

4. build

```bash
cmake --build build --config Debug
```

## 라이브러리화

### 구조

```text
cv_api_cpp_sdk/
├─ include/
│  ├─ ApiResult.h
│  ├─ Models.h
│  ├─ Operations.h
│  └─ ImageApiClient.h
├─ src/
│  ├─ HttpUtils.h
│  ├─ HttpUtils.cpp
│  ├─ Operations.cpp
│  └─ ImageApiClient.cpp
├─ example/
│  └─ main.cpp
├─ CMakeLists.txt
└─ README.md
```

### 파일 역할

- `ApiResult.h`
  - 성공/실패, 메시지 표현용 결과 타입 정의
- `Models.h`
  - 요청/응답 데이터 구조 정의
- `Operations.h / Operations.cpp`
  - `grayscale`, `blur`, `canny_edge`, `resize` 생성 helper 제공
- `ImageApiClient.h / ImageApiClient.cpp`
  - SDK 사용자가 직접 호출하는 공개 API 제공
- `HttpUtils.h / HttpUtils.cpp`
  - libcurl 콜백, header/body 처리, 공통 HTTP 요청 처리용 내부 구현

### CMakeLists 핵심 설정

```cmake
find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(ImageApiSdk STATIC
    src/ImageApiClient.cpp
    src/HttpUtils.cpp
    src/Operations.cpp
)
```

## 빌드 결과물

빌드 후 아래 파일 생성 확인.

```text
build/Debug/
├─ ImageApiSdk.lib
├─ ImageApiSdkExample.exe
├─ libcurl-d.dll
├─ zlib1d.dll
└─ *.pdb
```

- `ImageApiSdk.lib`
  - 정적 라이브러리 본체
- `ImageApiSdkExample.exe`
  - SDK 사용 예제 실행 파일
- `libcurl-d.dll`, `zlib1d.dll`
  - Debug 실행 시 필요한 DLL

## 주요 기능

### 1. HealthCheck

- 서버 상태 확인용 함수 제공
- 대상 엔드포인트: `GET /health`

### 2. ProcessImage

- 이미지 처리 요청용 핵심 함수 제공
- 대상 엔드포인트: `POST /api/v1/process`

입력 항목

- `image_url`
- `operations`
- `output_format`

출력 항목

- 처리된 이미지 바이트
- content-type
- width / height 정보

### 3. Operation Helper

요청 JSON을 직접 문자열로 만들지 않기 위한 helper 제공

- `MakeGrayscale()`
- `MakeBlur(ksize, sigma)`
- `MakeCannyEdge(threshold1, threshold2)`
- `MakeResize(width, height)`

## 예시 사용법 - [이미지 처리 함수 예시](./FunctionExampleOfUse.md)

### lib(Debug) 언급 및 헤더 포함

```cpp
#pragma comment(lib, "Debug\\ImageApiSdk.lib")
#pragma comment(lib, "Debug\\libcurl-d.lib")
#pragma comment(lib, "Debug\\zlibd.lib") 

#include "ImageApiClient.h"
#include "Operations.h"
```

### 클라이언트 생성

```cpp
image_api::ImageApiClient client("http://127.0.0.1:8080");
```

AWS 서버 테스트 시에는 base URL을 실제 서버 주소로 변경

```cpp
image_api::ImageApiClient client("http://3.106.215.74:8080");
```

### 요청 구성

```cpp
image_api::ProcessImageRequest request;
request.imageUrl = "https://example.com/test.jpg";
request.outputFormat = "png";

request.operations.push_back(image_api::MakeGrayscale());
request.operations.push_back(image_api::MakeBlur(9, 2.0));
request.operations.push_back(image_api::MakeResize(512, 512));
```

### 요청 실행

```cpp
image_api::ProcessImageResponse response;
image_api::ApiResult result = client.ProcessImage(request, response);

if (!result.success)
{
    std::cout << "failed: " << result.message << std::endl;
    return;
}
```

### 결과 확인

```cpp
std::cout << "content-type: " << response.contentType << std::endl;
std::cout << "width: " << response.width << std::endl;
std::cout << "height: " << response.height << std::endl;
std::cout << "bytes: " << response.imageBytes.size() << std::endl;
```

## 다른 프로젝트에서 사용

### 필요한 파일

```text
include/
ImageApiSdk.lib
libcurl-d.dll
zlib1d.dll
libcurl-d.lib
zlib1d.lib
```

### Visual Studio 설정

- 추가 포함 디렉터리
  - `include` 경로 추가
- 추가 라이브러리 디렉터리
  - `ImageApiSdk.lib` 경로 추가
- 추가 종속성
  - `ImageApiSdk.lib` 추가

## 주의 할점

- 수정한 경우 configure/build 다시 진행 필요
- x64 / x86 일치 필요
- Release의 경우 런타임 라이브러리 설정이 `MD(다중스레드 DLL)`로 되어 있으니 활용 프로젝트에서도 같게 필요
- 예제 코드의 base URL은 로컬 서버 기준일 수 있으므로 환경에 맞게 수정 필요
- 서버가 켜져 있어야 실제 요청 가능

## 향후 개선

- `find_package(ImageApiSdk CONFIG REQUIRED)` 형태 지원
- Docker / Linux 환경 빌드 확인
- DLL 버전 확장 검토

## 다운
- [라이브러리 zip파일](https://github.com/seje06/cv_api_cpp_sdk/releases/download/1.0.6/image_api_sdk.zip)
