#include "vector_utils.hpp"
#include <random>

vector<double> generate_random_vector(int n, unsigned seed)
{
    mt19937 gen(seed);              // Mersenne Twister deterministic
    uniform_real_distribution<double> dist(0.0, 1.0);

    vector<double> x(n);
    for (int i = 0; i < n; i++)
        x[i] = dist(gen);

    return x;
}