#include <cmath>
#include <nvector/nvector_serial.h>
#include <sstream>


double PI = 3.14159;
double BOLTZMAN_CONSTANT = ((8.6173) * (std::pow(10, -(5))));
double MAX_CLUSTER_SIZE = 10;
double flux = ((2.9) * (std::pow(10, -(7))));
double temperature = ((330) + (273.15));
double recombination = 0.3;
double i_bi = 0.5;
double i_tri = 0.2;
double i_quad = 0.06;
double v_bi = 0.06;
double v_tri = 0.03;
double v_quad = 0.02;
double dislocation_density_evolution = 300;
double i_migration = 0.45;
double v_migration = 1.35;
double i_diffusion_0 = ((1) * (std::pow(10, -(3))));
double v_diffusion_0 = 0.6;
double i_formation = 4.1;
double v_formation = 1.7;
double i_binding = 0.6;
double v_binding = 0.5;
double recombination_radius = ((0.7) * (std::pow(10, -(7))));
double i_loop_bias = 63;
double i_dislocation_bias = 0.8;
double i_dislocation_bias_param = 1.1;
double v_loop_bias = 33;
double v_dislocation_bias = 0.65;
double v_dislocation_bias_param = 1;
double dislocation_density_0 = ((10) * (std::pow(10, -(12))));
double grain_size = ((4) * (std::pow(10, -(3))));
double lattice_param = ((3.6) * (std::pow(10, -(8))));
double burgers_vector = ((lattice_param) / (std::sqrt(2)));
double atomic_volume = ((std::pow(lattice_param, 3)) / (4));
double i_diffusion = ((i_diffusion_0) * (std::exp(-(((i_migration) / (((BOLTZMAN_CONSTANT) * (temperature))))))));
double v_diffusion = ((v_diffusion_0) * (std::exp(-(((v_migration) / (((BOLTZMAN_CONSTANT) * (temperature))))))));
double Riv = ((((((4) * (PI))) * (((i_diffusion) + (v_diffusion))))) * (recombination_radius));
double dislocation_gain = 0;

const size_t INDEX_Ci_START = 0;
const size_t INDEX_Ci_SIZE = MAX_CLUSTER_SIZE - 1 + 1;
const size_t INDEX_Cv_START = INDEX_Ci_START + INDEX_Ci_SIZE;
const size_t INDEX_Cv_SIZE = MAX_CLUSTER_SIZE - 1 + 1;

const size_t INDEX_Rho = INDEX_Cv_START + INDEX_Cv_SIZE;
const size_t STATE_SIZE =INDEX_Rho + 1;

double Gi(double n);
double Gv(double n);
double a_i(double n, double* values);
double b_i(double n, double* values);
double c_i(double n, double* values);
double a_v(double n, double* values);
double b_v(double n, double* values);
double c_v(double n, double* values);
double i1_em(double* values);
double i1_cluster_abs(double* values);
double i1_d_abs(double* values);
double i1_gb_abs(double* values);
double v1_em(double* values);
double v1_cluster_abs(double* values);
double v1_d_abs(double* values);
double v1_gb_abs(double* values);
double alpha_ii(double n);
double alpha_vv(double n);
double beta_ii(double n);
double beta_iv(double n);
double beta_vi(double n);
double beta_vv(double n);
double i_bias_factor(double n);
double v_bias_factor(double n);
double i_binding_energy(double n);
double v_binding_energy(double n);
double r(double n);

double __summation_0(double* values) {
	double sum = 0.0;
	for (size_t i = 3; i < MAX_CLUSTER_SIZE; i++) {
		sum += ((alpha_ii((i))) * (values[INDEX_Ci_START + (i) - 1]));
	}
	 return sum;
}

double __summation_1(double* values) {
	double sum = 0.0;
	for (size_t i = 1; i < MAX_CLUSTER_SIZE; i++) {
		sum += ((beta_ii((i))) * (values[INDEX_Ci_START + (i) - 1]));
	}
	 return sum;
}

double __summation_2(double* values) {
	double sum = 0.0;
	for (size_t i = 2; i < MAX_CLUSTER_SIZE; i++) {
		sum += ((beta_vi((i))) * (values[INDEX_Ci_START + (i) - 1]));
	}
	 return sum;
}

double __summation_3(double* values) {
	double sum = 0.0;
	for (size_t i = 1; i < MAX_CLUSTER_SIZE; i++) {
		sum += ((beta_ii((i))) * (values[INDEX_Ci_START + (i) - 1]));
	}
	 return sum;
}

double __summation_4(double* values) {
	double sum = 0.0;
	for (size_t i = 1; i < MAX_CLUSTER_SIZE; i++) {
		sum += ((beta_vi((i))) * (values[INDEX_Cv_START + (i) - 1]));
	}
	 return sum;
}

double __summation_5(double* values) {
	double sum = 0.0;
	for (size_t i = 3; i < MAX_CLUSTER_SIZE; i++) {
		sum += ((alpha_vv((i))) * (values[INDEX_Ci_START + (i) - 1]));
	}
	 return sum;
}

double __summation_6(double* values) {
	double sum = 0.0;
	for (size_t i = 1; i < MAX_CLUSTER_SIZE; i++) {
		sum += ((beta_vv((i))) * (values[INDEX_Cv_START + (i) - 1]));
	}
	 return sum;
}

double __summation_7(double* values) {
	double sum = 0.0;
	for (size_t i = 2; i < MAX_CLUSTER_SIZE; i++) {
		sum += ((beta_iv((i))) * (values[INDEX_Cv_START + (i) - 1]));
	}
	 return sum;
}

double __summation_8(double* values) {
	double sum = 0.0;
	for (size_t i = 1; i < MAX_CLUSTER_SIZE; i++) {
		sum += ((beta_vv((i))) * (values[INDEX_Cv_START + (i) - 1]));
	}
	 return sum;
}

double __summation_9(double* values) {
	double sum = 0.0;
	for (size_t i = 1; i < MAX_CLUSTER_SIZE; i++) {
		sum += ((beta_iv((i))) * (values[INDEX_Ci_START + (i) - 1]));
	}
	 return sum;
}

double Gi(double n)
{
	if (n == (1)) {
		 return ((((((flux) * (recombination))) * (((((1) - (i_tri))) - (i_quad))))) / (atomic_volume));
	}
	if (n == (2)) {
		 return 0;
	}
	if (n == (3)) {
		 return ((((((flux) * (recombination))) * (i_tri))) / (atomic_volume));
	}
	if (n == (4)) {
		 return ((((((flux) * (recombination))) * (i_quad))) / (atomic_volume));
	}
	return 0;
}

double Gv(double n)
{
	if (n == (1)) {
		 return ((((((flux) * (recombination))) * (((((((1) - (v_bi))) - (v_tri))) - (v_quad))))) / (atomic_volume));
	}
	if (n == (2)) {
		 return ((((((flux) * (recombination))) * (v_bi))) / (atomic_volume));
	}
	if (n == (3)) {
		 return ((((((flux) * (recombination))) * (v_tri))) / (atomic_volume));
	}
	if (n == (4)) {
		 return ((((((flux) * (recombination))) * (v_quad))) / (atomic_volume));
	}
	return 0;
}

double a_i(double n, double* values)
{
	return ((((beta_iv((n))) * (values[INDEX_Cv_START + (1) - 1]))) + (alpha_ii((n))));
}

double b_i(double n, double* values)
{
	return ((((((beta_iv((n))) * (values[INDEX_Cv_START + (1) - 1]))) + (((beta_ii((n))) * (values[INDEX_Ci_START + (1) - 1]))))) + (alpha_ii((n))));
}

double c_i(double n, double* values)
{
	return ((beta_ii((n))) * (values[INDEX_Ci_START + (1) - 1]));
}

double a_v(double n, double* values)
{
	return ((((beta_vi((n))) * (values[INDEX_Ci_START + (1) - 1]))) + (alpha_vv((n))));
}

double b_v(double n, double* values)
{
	return ((((((beta_vi((n))) * (values[INDEX_Cv_START + (1) - 1]))) + (((beta_vv((n))) * (values[INDEX_Cv_START + (1) - 1]))))) + (alpha_vv((n))));
}

double c_v(double n, double* values)
{
	return ((beta_vv((n))) * (values[INDEX_Cv_START + (1) - 1]));
}

double i1_em(double* values)
{
	return ((((__summation_0(values)) + (((((3) * (alpha_ii((3))))) * (values[INDEX_Ci_START + (3) - 1]))))) + (((((((2) * (beta_iv((3))))) * (values[INDEX_Cv_START + (1) - 1]))) * (values[INDEX_Ci_START + (3) - 1]))));
}

double i1_cluster_abs(double* values)
{
	return ((__summation_1(values)) + (__summation_2(values)));
}

double i1_d_abs(double* values)
{
	return ((((values[INDEX_Rho]) * (i_diffusion))) * (i_dislocation_bias));
}

double i1_gb_abs(double* values)
{
	return ((((((6) * (i_diffusion))) * (std::sqrt(((((((values[INDEX_Rho]) * (i_dislocation_bias))) + (__summation_3(values)))) + (__summation_4(values))))))) / (grain_size));
}

double v1_em(double* values)
{
	return ((((__summation_5(values)) + (((((2) * (alpha_vv((2))))) * (values[INDEX_Cv_START + (2) - 1]))))) + (((((beta_vi((2))) * (values[INDEX_Ci_START + (1) - 1]))) * (values[INDEX_Cv_START + (2) - 1]))));
}

double v1_cluster_abs(double* values)
{
	return ((__summation_6(values)) + (__summation_7(values)));
}

double v1_d_abs(double* values)
{
	return ((((values[INDEX_Rho]) * (v_diffusion))) * (v_dislocation_bias));
}

double v1_gb_abs(double* values)
{
	return ((((((6) * (v_diffusion))) * (std::sqrt(((((((values[INDEX_Rho]) * (v_dislocation_bias))) + (__summation_8(values)))) + (__summation_9(values))))))) / (grain_size));
}

double alpha_ii(double n)
{
	return ((((((((((2) * (PI))) * (r((n))))) * (i_bias_factor((n))))) * (((i_diffusion) / (atomic_volume))))) * (std::exp(-(((i_binding_energy((n))) / (((BOLTZMAN_CONSTANT) * (temperature))))))));
}

double alpha_vv(double n)
{
	return ((((((((((2) * (PI))) * (r((n))))) * (v_bias_factor((n))))) * (((v_diffusion) / (atomic_volume))))) * (std::exp(-(((v_binding_energy((n))) / (((BOLTZMAN_CONSTANT) * (temperature))))))));
}

double beta_ii(double n)
{
	return ((((((((2) * (PI))) * (r((n))))) * (i_bias_factor((n))))) * (i_diffusion));
}

double beta_iv(double n)
{
	return ((((((((2) * (PI))) * (r((n))))) * (v_bias_factor((n))))) * (v_diffusion));
}

double beta_vi(double n)
{
	return ((((((((2) * (PI))) * (r((n))))) * (i_bias_factor((n))))) * (i_diffusion));
}

double beta_vv(double n)
{
	return ((((((((2) * (PI))) * (r((n))))) * (v_bias_factor((n))))) * (v_diffusion));
}

double i_bias_factor(double n)
{
	return ((i_dislocation_bias) + (((((std::sqrt(((burgers_vector) / (((((8) * (PI))) * (lattice_param)))))) - (i_dislocation_bias))) * (((1) / (std::pow(n, ((i_dislocation_bias_param) / (2)))))))));
}

double v_bias_factor(double n)
{
	return ((v_dislocation_bias) + (((((std::sqrt(((burgers_vector) / (((((8) * (PI))) * (lattice_param)))))) - (v_dislocation_bias))) * (((1) / (std::pow(n, ((v_dislocation_bias_param) / (2)))))))));
}

double i_binding_energy(double n)
{
	return ((i_formation) + (((((((i_binding) - (i_formation))) / (std::pow(2, ((0.8) - (1)))))) * (std::pow(std::pow(n, ((0.8) - (((n) - (1))))), 0.8)))));
}

double v_binding_energy(double n)
{
	return ((v_formation) + (((((((v_binding) - (v_formation))) / (std::pow(2, ((0.8) - (1)))))) * (std::pow(std::pow(n, ((0.8) - (((n) - (1))))), 0.8)))));
}

double r(double n)
{
	return std::sqrt(((((((3) * (std::pow(lattice_param, 2)))) * (n))) / (((4) * (PI)))));
}

void get_initial_state(N_Vector state) {
    double* values = N_VGetArrayPointer(state);

    for (size_t n = 1; n <= MAX_CLUSTER_SIZE; ++n)
    {
        values[INDEX_Ci_START + (n - 1)] = ((1) * (std::pow(10, -(5))));
    }

    for (size_t n = 1; n <= MAX_CLUSTER_SIZE; ++n)
    {
        values[INDEX_Cv_START + (n - 1)] = ((1) * (std::pow(10, -(5))));
    }
    values[INDEX_Rho] = dislocation_density_0;
}

std::string get_state_csv_label() {
	std::stringstream str; 
	str << "t (seconds)";
	for (size_t i = 1; i <= MAX_CLUSTER_SIZE; ++i)
	{
		str << ", Ci[" << i << "]";
	}

	for (size_t i = 1; i <= MAX_CLUSTER_SIZE; ++i)
	{
		str << ", Cv[" << i << "]";
	}

	str << ", Rho";
	str << ", extra";
	return str.str();
}

std::string get_csv_line(N_Vector state) {
	std::stringstream str;
	double* values = N_VGetArrayPointer(state);
	for (size_t i = 0; i < STATE_SIZE; ++i) {
		str << ", " << values[i];
	}
	str << ", " << b_i((3), values);
	return str.str();
}

int system(sunrealtype t, N_Vector y, N_Vector ydot, void *user_data) {
    double* values = N_VGetArrayPointer(y);
    double* derivatives = N_VGetArrayPointer(ydot);

    for (size_t n = 1; n <= MAX_CLUSTER_SIZE; ++n)
    {
        derivatives[INDEX_Ci_START + (n - 1)] = ((((Gv((n))) + (((((a_i((((n) + (1))), values)) * (-(b_i((n), values))))) * (values[INDEX_Ci_START + (n) - 1]))))) + (((c_i((((n) - (1))), values)) * (values[INDEX_Ci_START + (((n) - (1))) - 1]))));
    }

    for (size_t n = 1; n <= MAX_CLUSTER_SIZE; ++n)
    {
        derivatives[INDEX_Cv_START + (n - 1)] = ((((Gv((n))) + (((((a_v((((n) + (1))), values)) * (-(b_v((n), values))))) * (values[INDEX_Cv_START + (n) - 1]))))) + (((c_v((((n) - (1))), values)) * (values[INDEX_Cv_START + (((n) - (1))) - 1]))));
    }
    derivatives[INDEX_Rho] = 0;

    derivatives[INDEX_Ci_START + (size_t)(1 - 1)] = ((((((((Gi((1))) - (((((Riv) * (values[INDEX_Ci_START + (1) - 1]))) * (-(i1_d_abs(values))))))) - (i1_gb_abs(values)))) - (i1_cluster_abs(values)))) + (i1_em(values)));
    derivatives[INDEX_Cv_START + (size_t)(1 - 1)] = ((((((((Gv((1))) - (((((Riv) * (values[INDEX_Ci_START + (1) - 1]))) * (-(v1_d_abs(values))))))) - (v1_gb_abs(values)))) - (v1_cluster_abs(values)))) + (v1_em(values)));
    derivatives[INDEX_Ci_START + (size_t)(2 - 1)] = 0;
    derivatives[INDEX_Ci_START + (size_t)(MAX_CLUSTER_SIZE - 1)] = 0;
    derivatives[INDEX_Cv_START + (size_t)(MAX_CLUSTER_SIZE - 1)] = 0;
    return 0;
}