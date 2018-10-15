#include <iostream>
#include <string>
#include <fstream>
#include <dlib/rand.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Execute with 1 argument - filename to write data" << std::endl;
        std::cout << "E.g.: " << argv[0] << " kkmeans_ex.txt" << std::endl;
        exit(0);
    }

    std::string filename{argv[1]};
    std::ofstream file(filename);

    dlib::rand random_generator;

    // we will make 50 points from each class
    const int num_points_in_cluster = 50;
    double x, y;
    // make some samples near the origin
    double radius = 0.5;

    for (int i = 0; i < num_points_in_cluster; ++i) {
        double sign = 1;
        if (random_generator.get_random_double() < 0.5)
            sign = -1;
        x = 2 * radius * random_generator.get_random_double() - radius;
        y = sign * sqrt(radius * radius - x * x);

        // write sample to file
        file << x << ";" << y << "\n";
    }

    // make some samples in a circle around the origin but far away
    radius = 10.0;
    for (long i = 0; i < num_points_in_cluster; ++i) {
        double sign = 1;
        if (random_generator.get_random_double() < 0.5)
            sign = -1;
        x = 2 * radius * random_generator.get_random_double() - radius;
        y = sign * sqrt(radius * radius - x * x);

        // write sample to file
        file << x << ";" << y << "\n";
    }

    // make some samples in a circle around the point (25,25)
    radius = 4.0;
    for (long i = 0; i < num_points_in_cluster; ++i) {
        double sign = 1;
        if (random_generator.get_random_double() < 0.5)
            sign = -1;
        x = 2 * radius * random_generator.get_random_double() - radius;
        y = sign * sqrt(radius * radius - x * x);

        // translate this point away from the origin
        x += 25;
        y += 25;

        // write sample to file
        file << x << ";" << y << "\n";
    }
}
