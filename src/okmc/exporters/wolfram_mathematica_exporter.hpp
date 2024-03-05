#pragma once

#include "../exporter.hpp"

class WolframMathematicaExporter : public Exporter {
 public:
  void export_model(Model *model) override;
};
