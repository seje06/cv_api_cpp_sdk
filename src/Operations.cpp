#include "Operations.h"
#include <stdexcept>
#include <nlohmann/json.hpp>

/*
    ------------------------------------------------------------------------
    Operations.cpp
    ------------------------------------------------------------------------
    서버에서 지원하는 operation을 C++ SDK에서 동일하게 사용할 수 있도록
    helper 함수를 구현한 파일

    구현 방식:
    - Operation 객체의 type 에 실제 서버 operation 이름을 넣음
    - options 는 JSON 문자열로 직렬화해서 optionsJson 에 저장

    이유:
    - SDK 사용자는 JSON 문자열을 직접 만들 필요가 없음
    - 서버와 SDK 사이의 operation 형식을 일관되게 유지 가능
*/

namespace image_api
{
    // nlohmann::json 별칭
    using json = nlohmann::json;

    namespace
    {
        /*
            ----------------------------------------------------------------
            MakeOperation
            ----------------------------------------------------------------
            공통 Operation 생성 유틸리티

            역할:
            - type 문자열 설정
            - options JSON을 문자열로 직렬화하여 optionsJson에 저장

            참고:
            - options가 비어 있어도 {} 형태의 JSON 문자열이 들어간다.
            - 이 함수는 외부로 공개하지 않는 내부 helper 용도이다.
        */
        Operation MakeOperation(const char* type, const json& options = json::object())
        {
            Operation op;
            op.type = type;
            op.optionsJson = options.dump();
            return op;
        }
    }

    /*
        --------------------------------------------------------------------
        grayscale
        --------------------------------------------------------------------
        컬러 이미지를 흑백으로 변환

        서버 쪽에서는 별도 옵션 없이 type만으로 처리 가능
    */
    Operation MakeGrayscale()
    {
        return MakeOperation("grayscale");
    }

    /*
        --------------------------------------------------------------------
        invert
        --------------------------------------------------------------------
        이미지 색상 반전
    */
    Operation MakeInvert()
    {
        return MakeOperation("invert");
    }

    /*
        --------------------------------------------------------------------
        blur
        --------------------------------------------------------------------
        가우시안 블러 적용

        ksize:
        - 블러 커널 크기
        - 보통 홀수 사용
        - 서버 쪽에서 양수 홀수 보정 수행 가능

        sigma:
        - 블러 강도에 영향을 주는 표준편차 값
    */
    Operation MakeBlur(int ksize, double sigma)
    {
        json options;
        options["ksize"] = ksize;
        options["sigma"] = sigma;

        return MakeOperation("blur", options);
    }

    /*
        --------------------------------------------------------------------
        median_blur
        --------------------------------------------------------------------
        중간값 블러 적용

        용도:
        - salt-and-pepper noise 제거
        - 일반적인 가우시안 블러와 다른 특성 제공
    */
    Operation MakeMedianBlur(int ksize)
    {
        json options;
        options["ksize"] = ksize;

        return MakeOperation("median_blur", options);
    }

    /*
        --------------------------------------------------------------------
        bilateral_filter
        --------------------------------------------------------------------
        양방향 필터 적용

        d:
        - 픽셀 주변 이웃 크기

        sigmaColor:
        - 색상 공간 sigma

        sigmaSpace:
        - 좌표 공간 sigma

        특징:
        - 엣지를 비교적 유지하면서 노이즈 감소 가능
    */
    Operation MakeBilateralFilter(int d, double sigmaColor, double sigmaSpace)
    {
        json options;
        options["d"] = d;
        options["sigma_color"] = sigmaColor;
        options["sigma_space"] = sigmaSpace;

        return MakeOperation("bilateral_filter", options);
    }

    /*
        --------------------------------------------------------------------
        sharpen
        --------------------------------------------------------------------
        샤프닝 필터 적용

        현재는 별도 옵션 없는 단순 버전
        필요하면 나중에 강도 옵션을 추가 가능
    */
    Operation MakeSharpen()
    {
        return MakeOperation("sharpen");
    }

    /*
        --------------------------------------------------------------------
        canny_edge
        --------------------------------------------------------------------
        캐니 엣지 검출

        threshold1, threshold2:
        - 캐니 알고리즘에서 사용하는 두 개의 임계값
    */
    Operation MakeCannyEdge(double threshold1, double threshold2)
    {
        json options;
        options["threshold1"] = threshold1;
        options["threshold2"] = threshold2;

        return MakeOperation("canny_edge", options);
    }

    /*
        --------------------------------------------------------------------
        threshold
        --------------------------------------------------------------------
        이진화 처리

        thresh:
        - 기준 임계값

        maxValue:
        - threshold를 넘는 픽셀에 부여할 최대값

        thresholdType:
        - OpenCV threshold type 정수값
        - 예:
            0 -> THRESH_BINARY
            1 -> THRESH_BINARY_INV
            2 -> THRESH_TRUNC
            3 -> THRESH_TOZERO
            4 -> THRESH_TOZERO_INV
    */
    Operation MakeThreshold(double thresh, double maxValue, int thresholdType)
    {
        json options;
        options["thresh"] = thresh;
        options["max_value"] = maxValue;
        options["threshold_type"] = thresholdType;

        return MakeOperation("threshold", options);
    }

    /*
        --------------------------------------------------------------------
        resize
        --------------------------------------------------------------------
        이미지 크기 변경

        width / height:
        - 목표 이미지 크기

        interpolation:
        - 보간법
        - 예:
            0 -> INTER_NEAREST
            1 -> INTER_LINEAR
            2 -> INTER_CUBIC
            3 -> INTER_AREA
    */
    Operation MakeResize(int width, int height, int interpolation)
    {
        json options;
        options["width"] = width;
        options["height"] = height;
        options["interpolation"] = interpolation;

        return MakeOperation("resize", options);
    }

    /*
        --------------------------------------------------------------------
        crop
        --------------------------------------------------------------------
        이미지 일부 영역 자르기

        x, y:
        - 잘라낼 시작 좌표

        width, height:
        - 잘라낼 영역 크기
    */
    Operation MakeCrop(int x, int y, int width, int height)
    {
        json options;
        options["x"] = x;
        options["y"] = y;
        options["width"] = width;
        options["height"] = height;

        return MakeOperation("crop", options);
    }

    /*
        --------------------------------------------------------------------
        flip
        --------------------------------------------------------------------
        이미지 반전

        mode:
        - 1  : 좌우 반전
        - 0  : 상하 반전
        - -1 : 좌우 + 상하 모두 반전
    */
    Operation MakeFlip(int mode)
    {
        json options;
        options["mode"] = mode;

        return MakeOperation("flip", options);
    }

    /*
        --------------------------------------------------------------------
        rotate
        --------------------------------------------------------------------
        이미지 회전

        angle:
        - 90
        - 180
        - 270

        현재 SDK에서는 서버 지원 범위에 맞춰 정수 각도만 전달
    */
    Operation MakeRotate(int angle)
    {
        json options;
        options["angle"] = angle;

        return MakeOperation("rotate", options);
    }

    /*
        --------------------------------------------------------------------
        brightness_contrast
        --------------------------------------------------------------------
        밝기 / 대비 조정

        alpha:
        - 대비 계수

        beta:
        - 밝기 보정값
    */
    Operation MakeBrightnessContrast(double alpha, double beta)
    {
        json options;
        options["alpha"] = alpha;
        options["beta"] = beta;

        return MakeOperation("brightness_contrast", options);
    }

    /*
        --------------------------------------------------------------------
        erode
        --------------------------------------------------------------------
        침식 연산

        ksize:
        - 커널 크기

        iterations:
        - 반복 횟수
    */
    Operation MakeErode(int ksize, int iterations)
    {
        json options;
        options["ksize"] = ksize;
        options["iterations"] = iterations;

        return MakeOperation("erode", options);
    }

    /*
        --------------------------------------------------------------------
        dilate
        --------------------------------------------------------------------
        팽창 연산

        ksize:
        - 커널 크기

        iterations:
        - 반복 횟수
    */
    Operation MakeDilate(int ksize, int iterations)
    {
        json options;
        options["ksize"] = ksize;
        options["iterations"] = iterations;

        return MakeOperation("dilate", options);
    }
}