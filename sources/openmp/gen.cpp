#include <iostream>
#include <chrono>
#include <random>
#include <fstream>
using namespace std;

std::mt19937 rnd(std::chrono::high_resolution_clock::now().time_since_epoch().count());

int main(int argc, char *argv[]) {
    if (argc < 3) {
        cout << "Requires <matrix size> <output file>";
        return 0;
    }

    int n = stoi(argv[1]);
    if (n < 0) {
        cout << "Invalid <matrix size> parameter";
        return 0;
    }

    ofstream out(argv[2], ios::out);
    if (!out.is_open()) {
        cout << "Writing in output file failed!";
        return 0;
    }

    out << n << endl;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            float f = rnd();
            float t = rnd();
            f = t == 0 ? f : f / t;
            out << (rnd() % 2 == 0 ? -f : f) << ' ';
        }
        out << endl;
    }

    out.close();
}