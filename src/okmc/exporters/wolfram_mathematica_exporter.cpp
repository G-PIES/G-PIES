#include "wolfram_mathematica_exporter.hpp"

#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <algorithm>
#include <vector>

#include "../model/objects/defect.hpp"

void WolframMathematicaExporter::export_model(Model *model) {
    std::stringstream out;
    out << std::fixed << std::setprecision(10);

    std::vector<ModelObject *> objects = model->objects;
    int max_size = std::transform_reduce(
        objects.begin(), objects.end(), 0,
        [](const int a, const int b) { return std::max(a, b); },
        [](ModelObject *object) {
            return object == nullptr ? 0 : static_cast<Defect *>(object)->size;
        });
    int max_type = 2;

    std::vector<Defect *> defects_by_size[max_size][max_type];
    for (ModelObject *object : objects) {
        Defect *defect = static_cast<Defect *>(object);
        if (defect == nullptr) {
            continue;
        }
        defects_by_size[defect->size - 1][defect->type - 1].push_back(defect);
    }

    out << "AppendTo[Objects, {";
    for (int size = 1; size <= max_size; size++) {
        for (int type = 1; type <= max_type; type++) {
            out << "{";
            bool is_first = true;
            for (Defect *defect : defects_by_size[size - 1][type - 1]) {
                if (is_first) {
                    is_first = false;
                } else {
                    out << ",";
                }

                out << "{" << defect->position.x << "," << defect->position.y
                    << "," << defect->position.z << "}";
            }
            out << "}";

            if (type != max_type) {
                out << ",";
            }
        }

        if (size != max_size) {
            out << ",";
        }
    }
    out << "}];" << std::endl;

    out << "AppendTo[PlotStyles, {";
    for (int size = 1; size <= max_size; size++) {
        out << "Directive["
            << "Red"
            << ",PointSize[" << size * 0.01 << "]],";
        out << "Directive["
            << "Blue"
            << ",PointSize[" << size * 0.01 << "]]";
        if (size != max_size) {
            out << ",";
        }
    }
    out << "}];" << std::endl;

    std::cout << out.str();
}
