#pragma once

#include "csr.hpp"
#include "coo.hpp"

CSRMatrix coo_to_csr(const COOMatrix& A);