#include <dlib/clustering.h>
#include <dlib/rand.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <map>
#include <opencv2/opencv.hpp>


typedef dlib::matrix<double, 2, 1> point_t;
typedef dlib::radial_basis_kernel<point_t> kernel_t;

cv::Scalar get_bgr_from_hsv(const cv::Scalar& hsv) {
    cv::Mat img(1, 1, CV_8UC3, hsv);
    cv::cvtColor(img, img, cv::COLOR_HSV2BGR);
    auto result = img.at<cv::Vec3b>(0, 0);
    return result;
}

template<typename T>
std::string to_string_with_precision(const T a_value, const int n = 6) {
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

void create_diagram(const dlib::kkmeans<kernel_t>& model,
                    unsigned long num_clusters,
                    const std::vector<point_t>& points,
                    const std::string& filename) {
    const cv::Scalar black = cv::Scalar(0, 0, 0);
    const cv::Scalar white = cv::Scalar(255, 255, 255);

    cv::Mat bubble_hist(520, 520, CV_8UC3, cv::Scalar(255, 255, 255));

    double max_distance_from_0 = 0;
    for (const auto& point: points) {
        max_distance_from_0 = std::max(max_distance_from_0, std::abs(point(0)));
        max_distance_from_0 = std::max(max_distance_from_0, std::abs(point(1)));
    }

    std::map<int, int> cntpoints;
    std::map<int, double> sumpoints_x;
    std::map<int, double> sumpoints_y;

    for (const auto& point:points) {
        auto cluster = model(point);
        cntpoints[cluster]++;
        sumpoints_x[cluster] += point(0);
        sumpoints_y[cluster] += point(1);
    }


    for (unsigned long i = 0; i < num_clusters; i++) {
        auto radius = static_cast<int>(100.0 / points.size() * cntpoints[i]);

        cv::Mat tmp_img(520, 520, CV_8UC3, white);
        auto color = get_bgr_from_hsv(cv::Scalar(180 / num_clusters * (i + 1), 120, 200));
        auto kernel_points = model.get_kcentroid(i).get_distance_function().get_basis_vectors();
        for (const auto& point: kernel_points) {
            cv::circle(tmp_img,
                       cv::Point2d(point(0) / max_distance_from_0 * 240 + 260,
                                   point(1) / max_distance_from_0 * 240 + 260),
                       radius, /*color=*/color, /*thickness=*/CV_FILLED);
        }
        cv::addWeighted(bubble_hist, 0.8, tmp_img, 0.2, 0, bubble_hist);
    }

    for (const auto& point:points) {
        auto cluster = model(point);
        cntpoints[cluster]++;
        sumpoints_x[cluster] += point(0);
        sumpoints_y[cluster] += point(1);
        auto color = get_bgr_from_hsv(cv::Scalar(180 / num_clusters * (cluster + 1), 150, 255));
        cv::circle(bubble_hist,
                   cv::Point2d(point(0) / max_distance_from_0 * 240 + 260, point(1) / max_distance_from_0 * 240 + 260),
                   2, /*color=*/color, /*thickness=*/CV_FILLED);
    }


    cv::arrowedLine(bubble_hist, cv::Point(5, 260), cv::Point(515, 260), black,
                    2, /*line_type=*/8, /*shift=*/0, /*tipLength=*/0.02);
    cv::arrowedLine(bubble_hist, cv::Point(260, 515), cv::Point(260, 5), black,
                    2, /*line_type=*/8, /*shift=*/0, /*tipLength=*/0.02);

    cv::line(bubble_hist, cv::Point(20, 258), cv::Point(20, 262), black, 2);
    cv::putText(bubble_hist, to_string_with_precision(-max_distance_from_0, 1), cv::Point(5, 275),
                cv::FONT_HERSHEY_SIMPLEX, 0.3, black, 1);

    cv::line(bubble_hist, cv::Point(500, 258), cv::Point(500, 262), black, 2);
    cv::putText(bubble_hist, to_string_with_precision(max_distance_from_0, 1), cv::Point(485, 275),
                cv::FONT_HERSHEY_SIMPLEX, 0.3, black, 1);

    cv::line(bubble_hist, cv::Point(258, 20), cv::Point(262, 20), black, 2);
    cv::putText(bubble_hist, to_string_with_precision(max_distance_from_0, 1), cv::Point(275, 22),
                cv::FONT_HERSHEY_SIMPLEX, 0.3, black, 1);

    cv::line(bubble_hist, cv::Point(258, 500), cv::Point(262, 500), black, 2);
    cv::putText(bubble_hist, to_string_with_precision(-max_distance_from_0, 1), cv::Point(275, 502),
                cv::FONT_HERSHEY_SIMPLEX, 0.3, black, 1);


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
    if (argc < 2 || argc > 4) {
        std::cout << "Execute with 1, 2 or 3 arguments - number of clusters"
                     ", optional max_dictionary_size"
                     " and (optional) filename to write diagram" << std::endl;
        std::cout << "E.g.: " << argv[0] << " 3" << std::endl;
        exit(0);
    }

    unsigned long num_clusters = std::stoul(argv[1]);
    unsigned long max_dictionary_size = 8;
    if(argc >= 3)
        max_dictionary_size = std::stoul(argv[2]);

    dlib::kcentroid<kernel_t> model_k_centroids(kernel_t(0.1), 0.01, /*max_dictionary_size=*/max_dictionary_size);
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

    if (argc == 4)
        create_diagram(model, num_clusters, points, argv[3]);

    return 0;
}
