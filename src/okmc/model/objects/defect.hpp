#pragma once

#include "../model_object.hpp"
#include "../../vector3.hpp"

enum DefectType {
    Interstitial = 1,
    Vacancy
};

class Defect : public ModelObject {
public:
    int size;
    DefectType type;
    Vector3<double> position;
};
