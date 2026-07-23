#pragma once

#include "MathTypes.h"

#include <cmath>

namespace gui {

constexpr int NUM_COLORS = 4;

class color {
public:
    double R = 1.0;
    double G = 1.0;
    double B = 1.0;

    color operator*(double f)
    {
        color c;
        c.R = R * f;
        c.G = G * f;
        c.B = B * f;
        return c;
    }

    void operator=(double f)
    {
        R = G = B = f;
    }

    void getHeatMapColor(double value)
    {
        color arr_col[NUM_COLORS];

        arr_col[0].R = 0.5;
        arr_col[0].G = 0.5;
        arr_col[0].B = 1.0;

        arr_col[1].R = 0.5;
        arr_col[1].G = 1.0;
        arr_col[1].B = 0.5;

        arr_col[2].R = 1.0;
        arr_col[2].G = 1.0;
        arr_col[2].B = 0.5;

        arr_col[3].R = 1.0;
        arr_col[3].G = 0.5;
        arr_col[3].B = 0.5;

        int ind1;
        int ind2;
        double fracIntermedia = 0.0;

        if (value <= 0.0) {
            ind1 = ind2 = 0;
        } else if (value >= 1.0) {
            ind1 = ind2 = 3;
        } else {
            value = value * (NUM_COLORS - 1);
            ind1 = static_cast<int>(std::floor(value));
            ind2 = ind1 + 1;
            fracIntermedia = value - static_cast<double>(ind1);
        }

        R = (arr_col[ind2].R - arr_col[ind1].R) * fracIntermedia + arr_col[ind1].R;
        G = (arr_col[ind2].G - arr_col[ind1].G) * fracIntermedia + arr_col[ind1].G;
        B = (arr_col[ind2].B - arr_col[ind1].B) * fracIntermedia + arr_col[ind1].B;
    }
};

inline Vec3 heatMapColorVec3(double value)
{
    color heatMapColor;
    heatMapColor.getHeatMapColor(value);
    return {
        static_cast<float>(heatMapColor.R),
        static_cast<float>(heatMapColor.G),
        static_cast<float>(heatMapColor.B)
    };
}

}  // namespace gui
