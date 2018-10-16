#include <dlib/clustering.h>
#include <dlib/rand.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>


typedef dlib::matrix<double, 2, 1> point_t;
typedef dlib::radial_basis_kernel<point_t> kernel_t;

void create_diagram(const std::vector<point_t>& points,
                    const std::vector<unsigned long>& point2cluster,
                    const std::string& filename) {
    const cv::Scalar black_hsv = cv::Scalar(0, 0, 0);
    const cv::Scalar white_hsv = cv::Scalar(0, 0, 255);

    cv::Mat bubble_hist_hsv(520, 520, CV_8UC3, white_hsv);
    cv::arrowedLine(bubble_hist_hsv, cv::Point(5, 260), cv::Point(515, 260), black_hsv,
                    2, /*line_type=*/8, /*shift=*/0, /*tipLength=*/0.02);
    cv::arrowedLine(bubble_hist_hsv, cv::Point(260, 515), cv::Point(260, 5), black_hsv,
                    2, /*line_type=*/8, /*shift=*/0, /*tipLength=*/0.02);
    double max_distance_from_0 = 0;
    for(const auto& point: points) {
        max_distance_from_0 = std::max(max_distance_from_0, std::abs(point(0)));
        max_distance_from_0 = std::max(max_distance_from_0, std::abs(point(1)));
    }
    cv::line(bubble_hist_hsv, cv::Point(20, 258), cv::Point(20, 262), black_hsv, 2);
    cv::line(bubble_hist_hsv, cv::Point(500, 258), cv::Point(500, 262), black_hsv, 2);
    cv::line(bubble_hist_hsv, cv::Point(258, 20), cv::Point(262, 20), black_hsv, 2);
    cv::line(bubble_hist_hsv, cv::Point(258, 500), cv::Point(262, 500), black_hsv, 2);

    unsigned long num_cluters = *std::max_element(point2cluster.cbegin(), point2cluster.cend()) + 1;

    for(int i = 0; i < static_cast<int>(points.size()); i++) {
        auto point = points[i];
        cv::circle(bubble_hist_hsv,
                cv::Point2d(point(0) / max_distance_from_0 * 240 + 260, point(1) / max_distance_from_0 * 240 + + 260),
                2, /*color=*/cv::Scalar(0, 150, 255 / num_cluters * (point2cluster[i] + 1)), /*thickness=*/CV_FILLED);
    }

    cv::Mat bubble_hist;
    cv::cvtColor(bubble_hist_hsv, bubble_hist, cv::COLOR_HSV2BGR);

    cv::imwrite(filename, bubble_hist);
}

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
    if (argc != 2 && argc != 3) {
        std::cout << "Execute with 1 or 2 arguments - number of clusters"
                     " and (optional) filename to write diagram" << std::endl;
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

    std::vector<unsigned long> point2cluster;
    point2cluster.reserve(points.size());
    unsigned long cluster_id;
    for (const auto& point: points) {
        cluster_id = model(point);
        point2cluster.push_back(cluster_id);
        std::cout << point(0) << ";" << point(1) << ";cluster" << cluster_id << "\n";
    }

    if (argc == 3)
        create_diagram(points, point2cluster, argv[2]);


    return 0;
}
