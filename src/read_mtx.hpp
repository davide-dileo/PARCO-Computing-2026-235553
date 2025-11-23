#pragma once

#include "coo.hpp"
#include <string>

using namespace std;

COOMatrix read_matrix_market(const string& filename);