#pragma once

#include <dlib/clustering.h>

typedef dlib::matrix<double, 2, 1> point_t;
typedef dlib::radial_basis_kernel<point_t> kernel_t;
