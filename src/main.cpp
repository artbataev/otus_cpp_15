#include <dlib/clustering.h>
#include <dlib/rand.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>


typedef dlib::matrix<double, 2, 1> point_t;
typedef dlib::radial_basis_kernel<point_t> kernel_t;

std::vector<double> split_point(const std::string& str) {
    std::vector<double> result;

    std::string::size_type start = 0;
    std::string::size_type stop = str.find_first_of(';');
    while (stop != std::string::npos) {
        result.push_back(std::stod(str.substr(start, stop - start)));
        start = stop + 1;
        stop = str.find_first_of(';', start);
    }

    result.push_back(std::stod(str.substr(start)));
    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Execute with 1 argument - number of clusters" << std::endl;
        std::cout << "E.g.: " << argv[0] << " 3" << std::endl;
        exit(0);
    }

    unsigned long num_clusters = std::stoul(argv[1]);

    dlib::kcentroid<kernel_t> model_k_centroids(kernel_t(0.1), 0.01, 8);
    dlib::kkmeans<kernel_t> model(model_k_centroids);

    std::vector<point_t> points;
    std::vector<point_t> initial_centers;

    {
        std::string current_line;
        point_t point;
        while (getline(std::cin, current_line)) {
            std::vector<double> point_read = split_point(current_line);
            if (point_read.size() != 2)
                throw std::logic_error("Unknown point format");
            point(0) = point_read[0];
            point(1) = point_read[1];
            points.push_back(point);
        }
    }

    model.set_number_of_centers(num_clusters);
    pick_initial_centers(num_clusters, initial_centers, points, model.get_kernel());
    model.train(points, initial_centers);

    for (const auto& point: points) {
        std::cout << point(0) << ";" << point(1) << ";cluster" << model(point) << "\n";
    }

    return 0;
}
