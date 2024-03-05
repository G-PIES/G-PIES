#pragma once

#include "../exporter.hpp"

class NullExporter : public Exporter {
 public:
    void export_model(Model *model) override{};
};
