#pragma once

#include "../../vector3.hpp"
#include "../model_object.hpp"

enum DefectType { Interstitial = 1, Vacancy };

class Defect : public ModelObject {
 public:
    int size;
    DefectType type;
    Vector3<double> position;
};
