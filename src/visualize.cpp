#include "visualize.h"
#include <opencv2/opencv.hpp>
#include <sstream>
#include <algorithm>
#include <map>

// convert hsv scalar to bgr
cv::Scalar hsv2bgr(const cv::Scalar& hsv) {
    cv::Mat img(1, 1, CV_8UC3, hsv);
    cv::cvtColor(img, img, cv::COLOR_HSV2BGR);
    auto result = img.at<cv::Vec3b>(0, 0);
    return result;
}

// custom to_string with precision for float/double
template<typename T>
std::string to_string_with_precision(const T value, const int n = 2) {
    std::ostringstream ss;
    ss.precision(n);
    ss << std::fixed << value;
    return ss.str();
}

// create diagram from model
void create_diagram(const dlib::kkmeans<kernel_t>& model,
                    unsigned long num_clusters,
                    const std::vector<point_t>& points,
                    const std::string& filename) {

    const cv::Scalar white = cv::Scalar(255, 255, 255);

    int padding = 20;
    int half_img_width = 240;
    int total_img_width = half_img_width * 2 + padding * 2;
    cv::Mat bubble_hist(total_img_width, total_img_width, CV_8UC3, white);

    // calculate max distance from 0 point - for x, y
    double max_distance_from_0 = 0;
    for (const auto& point: points) {
        max_distance_from_0 = std::max(max_distance_from_0, std::abs(point(0)));
        max_distance_from_0 = std::max(max_distance_from_0, std::abs(point(1)));
    }

    // count number of points in each cluster
    std::map<int, int> points_in_cluster;
    for (const auto& point:points) {
        auto cluster = model(point);
        points_in_cluster[cluster]++;
    }

    // draw circles around each kernel point, radius proportional to count of points in cluster
    for (unsigned long i = 0; i < num_clusters; i++) {
        auto radius = static_cast<int>(100.0 / points.size() * points_in_cluster[i]);

        cv::Mat tmp_img(total_img_width, total_img_width, CV_8UC3, white);
        // construct color, where hue is 179 / num_clusters * (i + 1) - so different colors for each cluster
        auto color = hsv2bgr(cv::Scalar(179 / num_clusters * (i + 1), 122, 200));
        auto kernel_points = model.get_kcentroid(i).get_distance_function().get_basis_vectors();
        for (const auto& point: kernel_points) {
            // -point(1): from top to bottom OpenCV y coordinate
            cv::circle(tmp_img,
                       cv::Point2d(point(0) / max_distance_from_0 * half_img_width + half_img_width + padding,
                                   -point(1) / max_distance_from_0 * half_img_width + half_img_width + padding),
                       radius, /*color=*/color, /*thickness=*/CV_FILLED);
        }
        auto alpha = (num_clusters - 1.0) / num_clusters;
        cv::addWeighted(bubble_hist, alpha, tmp_img, 1 - alpha, 0, bubble_hist);
    }

    // draw points
    for (const auto& point:points) {
        auto cluster = model(point);
        // construct color, where hue is 179 / num_clusters * (i + 1) - so different colors for each cluster
        auto color = hsv2bgr(cv::Scalar(179 / num_clusters * (cluster + 1), 200, 255));
        // -point(1): from top to bottom OpenCV y coordinate
        cv::circle(bubble_hist,
                   cv::Point2d(point(0) / max_distance_from_0 * half_img_width + half_img_width + padding,
                               -point(1) / max_distance_from_0 * half_img_width + half_img_width + padding),
                   2, /*color=*/color, /*thickness=*/CV_FILLED);
    }

    // create grid
    const cv::Scalar black = cv::Scalar(0, 0, 0);
    cv::arrowedLine(bubble_hist,
                    cv::Point(5, half_img_width + padding),
                    cv::Point(total_img_width - 5, half_img_width + padding),
                    black, 2, /*line_type=*/8, /*shift=*/0, /*tipLength=*/0.02);
    cv::arrowedLine(bubble_hist,
                    cv::Point(half_img_width + padding, total_img_width - 5),
                    cv::Point(half_img_width + padding, 5),
                    black, 2, /*line_type=*/8, /*shift=*/0, /*tipLength=*/0.02);

    // left
    cv::line(bubble_hist,
             cv::Point(padding, half_img_width + padding - 3),
             cv::Point(padding, half_img_width + padding + 3),
             black, 2);
    cv::putText(bubble_hist, to_string_with_precision(-max_distance_from_0, 1),
                cv::Point(padding - 15, half_img_width + padding + 15),
                cv::FONT_HERSHEY_SIMPLEX, 0.3, black, 1);

    // right
    cv::line(bubble_hist,
             cv::Point(total_img_width - padding, half_img_width + padding - 3),
             cv::Point(total_img_width - padding, half_img_width + padding + 3),
             black, 2);
    cv::putText(bubble_hist, to_string_with_precision(max_distance_from_0, 1),
                cv::Point(total_img_width - padding - 15, half_img_width + padding + 15),
                cv::FONT_HERSHEY_SIMPLEX, 0.3, black, 1);

    // top
    cv::line(bubble_hist,
             cv::Point(half_img_width + padding - 3, padding),
             cv::Point(half_img_width + padding + 3, padding),
             black, 2);
    cv::putText(bubble_hist, to_string_with_precision(max_distance_from_0, 1),
                cv::Point(half_img_width + padding + 15, 22),
                cv::FONT_HERSHEY_SIMPLEX, 0.3, black, 1);

    // bottom
    cv::line(bubble_hist,
             cv::Point(half_img_width + padding - 3, total_img_width - padding),
             cv::Point(half_img_width + padding + 3, total_img_width - padding),
             black, 2);
    cv::putText(bubble_hist, to_string_with_precision(-max_distance_from_0, 1),
                cv::Point(half_img_width + padding + 15, total_img_width - 18),
                cv::FONT_HERSHEY_SIMPLEX, 0.3, black, 1);

    // save image
    cv::imwrite(filename, bubble_hist);
}
