#include <iostream>
#include <vector>
#include <fstream>

#include <chrono>
#include <random>

using namespace std;

int threads_count = 0;
double findOMP_det(int &n, vector<vector<float>> &matrix) {
    double determinant = 1;
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            if (matrix[j][i] != 0) {
                if (i != j) {
                    swap(matrix[i], matrix[j]);
                    determinant *= -1;
                }
                break;
            }
        }

        if (matrix[i][i] != 0) {
#pragma omp parallel for num_threads(threads_count) schedule(static)
            for (int j = i + 1; j < n; j++) {
                float f = -matrix[j][i] / matrix[i][i];
                for (int k = 0; k < n; k++) {
                    matrix[j][k] += f * matrix[i][k];
                }
            }
        }
    }

#pragma omp parallel for num_threads(threads_count) reduction(*:determinant) schedule(static)
    for (int i = 0; i < n; i++) {
        determinant *= (double) matrix[i][i];
    }

    return determinant;
}

double findNonOMP_det(int &n, vector<vector<float>> &matrix) {
    double determinant = 1;
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            if (matrix[j][i] != 0) {
                if (i != j) {
                    swap(matrix[i], matrix[j]);
                    determinant *= -1;
                }
                break;
            }
        }

        if (matrix[i][i] != 0) {
            for (int j = i + 1; j < n; j++) {
                float f = -matrix[j][i] / matrix[i][i];
                for (int k = 0; k < n; k++) {
                    matrix[j][k] += f * matrix[i][k];
                }
            }
        }
    }

    for (int i = 0; i < n; i++) {
        determinant *= matrix[i][i];
    }

    return determinant;
}

pair<double, double> find_det(bool usingOMP, int n, vector<vector<float>> matrix) {
    float det;

    auto begin = std::chrono::steady_clock::now();
    det = usingOMP ? findOMP_det(n, matrix) : findNonOMP_det(n, matrix);
    auto end = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

    return {det, elapsed_ms.count()};
}

bool read(const string &file, int &n, vector<vector<float>> &matrix) {
    ifstream in(file, ios::in);
    if (!in.is_open()) {
        return false;
    }

    in >> n;
    matrix.resize(n);
    for (int i = 0; i < n; i++) {
        matrix[i].resize(n);
        for (int j = 0; j < n; j++) {
            in >> matrix[i][j];
        }
    }

    in.close();
    return true;
}

bool write(const string &file, double &ans) {
    ofstream out(file, ios::out);
    if (!out.is_open()) {
        return false;
    }

    out << "Determinant: " << ans << " \n";

    out.close();
    return true;
}

double solve(const int &n, const vector<vector<float>> &matrix) {
    pair<double, double> det = find_det(true, n, matrix);
    pair<double, double> detCheck = find_det(false, n, matrix);

    double delta = fabs(det.first - detCheck.first);
    cerr << (delta / (det.first + detCheck.first) < 0.001 || (isinf(det.first) && isinf(detCheck.first)) ? "OK" : "Delta: " + to_string(delta)) << endl;

    cout << "Time (" << (threads_count == 0 ? "virtual processors parameters" : to_string(threads_count) + " thread(s)") << "): " << det.second << "ms" << endl;
    cout << "Time (without OMP): " << detCheck.second << "ms" << endl;

    return det.first;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        cout << "Requires <threads_count> <input_file> [<output_file>]";
        return 0;
    }

    threads_count = stoi(argv[1]);
    if (threads_count < 0) {
        cout << "Invalid threads count!";
        return 0;
    }
    int n;
    vector<vector<float>> matrix;

    if (!read(argv[2], n, matrix)) {
        cout << "Reading int input file failed!";
        return 0;
    }

    double det = solve(n, matrix);

    if (argc > 3) {
        if (!write(argv[3], det)) {
            cout << "Writing in output file failed!";
            return 0;
        }
    } else {
        cout << "Determinant: " << det << " \n";
    }

    return 0;
}
