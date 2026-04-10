#pragma once

#include "Models.h"

namespace image_api
{
    /*
        --------------------------------------------------------------------
        Operation 생성 helper 함수들
        --------------------------------------------------------------------
        사용자가 JSON 문자열을 직접 만들지 않아도 되도록,
        자주 쓰는 작업을 C++ 함수 형태로 제공한다.

        예:
        request.operations.push_back(MakeGrayscale());
        request.operations.push_back(MakeBlur(9, 2.0));
    */

    // 흑백 변환 작업 생성
    Operation MakeGrayscale();

    // 가우시안 블러 작업 생성
    Operation MakeBlur(int ksize = 9, double sigma = 2.0);

    // 캐니 엣지 작업 생성
    Operation MakeCannyEdge(double threshold1 = 100.0, double threshold2 = 200.0);

    // 리사이즈 작업 생성
    Operation MakeResize(int width, int height);
}
