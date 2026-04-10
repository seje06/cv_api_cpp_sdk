#include "Operations.h"

#include <stdexcept>

#include <nlohmann/json.hpp>

namespace image_api
{
    using json = nlohmann::json;

    namespace
    {
        /*
            ----------------------------------------------------------------
            MakeOperation
            ----------------------------------------------------------------
            type 문자열과 options JSON 객체를 받아 Operation 구조체를 생성하는
            내부 helper 함수이다.
        */
        Operation MakeOperation(const std::string& type, const json& options)
        {
            Operation op;
            op.type = type;

            // options가 비어 있으면 굳이 문자열을 만들지 않는다.
            if (!options.empty())
            {
                op.optionsJson = options.dump();
            }

            return op;
        }
    }

    Operation MakeGrayscale()
    {
        return MakeOperation("grayscale", json::object());
    }

    Operation MakeBlur(int ksize, double sigma)
    {
        json options = {
            { "ksize", ksize },
            { "sigma", sigma }
        };

        return MakeOperation("blur", options);
    }

    Operation MakeCannyEdge(double threshold1, double threshold2)
    {
        json options = {
            { "threshold1", threshold1 },
            { "threshold2", threshold2 }
        };

        return MakeOperation("canny_edge", options);
    }

    Operation MakeResize(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            throw std::invalid_argument("MakeResize requires positive width and height");
        }

        json options = {
            { "width", width },
            { "height", height }
        };

        return MakeOperation("resize", options);
    }
}
