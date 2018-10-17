#pragma once

#include <dlib/clustering.h>
#include <dlib/rand.h>
#include "common_types.h"

void create_diagram(const dlib::kkmeans<kernel_t>& model,
                    unsigned long num_clusters,
                    const std::vector<point_t>& points,
                    const std::string& filename);
