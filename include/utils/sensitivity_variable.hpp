#ifndef SENSITIVITY_VARIABLES_HPP
#define SENSITIVITY_VARIABLES_HPP

#include <map>
#include <string>

enum class SensitivityVariable {
  NONE,
  interstitial_migration_ev,
  vacancy_migration_ev,
  interstitial_formation_ev,
  vacancy_formation_ev,
  interstitial_binding_ev,
  vacancy_binding_ev,
  initial_dislocation_density_cm,
  flux_dpa_s,
  temperature_kelvin,
  dislocation_density_evolution,
};

static std::map<std::string, SensitivityVariable> sensitivity_variables{
    {"interstitial-migration-ev",
     SensitivityVariable::interstitial_migration_ev},
    {"vacancy-migration-ev", SensitivityVariable::vacancy_migration_ev},
    {"interstitial-formation-ev",
     SensitivityVariable::interstitial_formation_ev},
    {"vacancy-formation-ev", SensitivityVariable::vacancy_formation_ev},
    {"interstitial-binding-ev", SensitivityVariable::interstitial_binding_ev},
    {"vacancy-binding-ev", SensitivityVariable::vacancy_binding_ev},
    {"initial-dislocation-density-cm^-2",
     SensitivityVariable::initial_dislocation_density_cm},
    {"flux-dpa-s", SensitivityVariable::flux_dpa_s},
    {"temperature-kelvin", SensitivityVariable::temperature_kelvin},
    {"dislocation-density-evolution",
     SensitivityVariable::dislocation_density_evolution}};

#endif  // SENSITIVITY_VARIABLES_HPP
