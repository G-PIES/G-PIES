#pragma once

#include "model/model.hpp"

class Exporter {
 public:
    virtual void export_model(Model *model) = 0;
};
