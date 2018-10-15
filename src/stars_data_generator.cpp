#include <iostream>
#include <string>
#include <fstream>
#include <dlib/rand.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Execute with 1 arguments - filename to write data" << std::endl;
        std::cout << "E.g.: " << argv[0] << " kkmeans_ex.txt" << std::endl;
        exit(0);
    }

    std::string filename{argv[1]};
    std::ofstream file(filename);

    dlib::rand random_generator;

    int num_points = 200;
    double min_value = -100;
    double max_value = 100;
    double half_distance = (max_value - min_value) / 2.0;
    double middle_value = min_value + half_distance;
    {
        double x;
        double y;
        for (int _ = 0; _ < num_points; _++) {
            x = (random_generator.get_random_double() - 0.5) * half_distance + middle_value;
            y = (random_generator.get_random_double() - 0.5) * half_distance + middle_value;
            file << x << ";" << y << "\n";
        }
    }
}
