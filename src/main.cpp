#include <dlib/clustering.h>
#include <dlib/rand.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include "visualize.h"
#include "common_types.h"


// split input string - for point reading
std::vector<std::string> split_string(const std::string& str, const char by = ';') {
    std::vector<std::string> result;

    std::string::size_type start = 0;
    std::string::size_type stop = str.find_first_of(by);
    while (stop != std::string::npos) {
        result.push_back(str.substr(start, stop - start));
        start = stop + 1;
        stop = str.find_first_of(';', start);
    }

    result.push_back(str.substr(start));
    return result;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 4) {
        std::cout << "Execute with 1, 2 or 3 arguments - number of clusters"
                     ", optional kernel_gamma"
                     " and (optional) filename to write diagram" << std::endl;
        std::cout << "E.g.: " << argv[0] << " 3 0.1 diagram.jpg" << std::endl;
        exit(0);
    }

    unsigned long num_clusters = std::stoul(argv[1]);
    double kernel_gamma = 0.1;
    if (argc >= 3)
        kernel_gamma = std::stod(argv[2]);

    // read data
    std::vector<point_t> points;
    {
        std::string current_line;
        point_t point;
        while (getline(std::cin, current_line)) {
            auto point_read = split_string(current_line, ';');
            if (point_read.size() != 2)
                throw std::logic_error("Unknown point format");
            point(0) = std::stod(point_read[0]);
            point(1) = std::stod(point_read[1]);
            points.push_back(point);
        }
    }

    // construct model
    std::vector<point_t> initial_centers;
    dlib::kcentroid<kernel_t> model_k_centroids(kernel_t(kernel_gamma), 0.01);
    dlib::kkmeans<kernel_t> model(model_k_centroids);

    model.set_number_of_centers(num_clusters);
    pick_initial_centers(num_clusters, initial_centers, points, model.get_kernel());

    // train model
    model.train(points, initial_centers);

    // output, format x;y;cluster1
    unsigned long cluster_id;
    for (const auto& point: points) {
        cluster_id = model(point);
        std::cout << point(0) << ";" << point(1) << ";cluster" << cluster_id << "\n";
    }

    // save diagram if necessary
    if (argc == 4)
        create_diagram(model, num_clusters, points, argv[3]);

    return 0;
}
