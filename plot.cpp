#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

const vector<string> optimizations = {"O0", "O1", "O2", "O3", "noopt"};
const vector<int> threads = {1, 2};
const vector<int> sizes = {1000, 10000, 100000, 1000000, 10000000};

map<tuple<string, int, int>, double> results; 
map<tuple<string, int, int>, double> vars;   
map<int, double> gp_results;                  
map<int, double> gp_vars;                     

void load_results(const string& results_fl) {
    for (const auto& opt : optimizations) {
        for (int thread : threads) {
            for (int size : sizes) {
                string filename = results_fl + "/result_" + opt + "_" + to_string(thread) + "_" + to_string(size) + ".txt";
                ifstream file(filename);
                if (file) {
                    string line;
                    getline(file, line);
                    istringstream iss(line);
                    string dummy;
                    double time_taken, var;
                    iss >> dummy >> time_taken >> dummy >> var;
                    results[{opt, thread, size}] = time_taken;
                    vars[{opt, thread, size}] = var;
                }
            }
        }
    }

    for (int thread : threads) {
        for (int size : sizes) {
            string filename = results_fl + "/result_noopt_" + to_string(thread) + "_" + to_string(size) + ".txt";
            ifstream file(filename);
            if (file) {
                string line;
                getline(file, line);
                istringstream iss(line);
                string dummy;
                double time_taken, var;
                iss >> dummy >> time_taken >> dummy >> var;
                results[{"noopt", thread, size}] = time_taken;
                vars[{"noopt", thread, size}] = var;
            }
        }
    }

    for (int size : sizes) {
        string filename = "gp_results/result_" + to_string(size) + ".txt";
        ifstream file(filename);
        if (file) {
            string line;
            getline(file, line);
            istringstream iss(line);
            string dummy;
            double time_taken, var;
            iss >> dummy >> time_taken >> dummy >> var;
            gp_results[size] = time_taken;
            gp_vars[size] = var;
        }
    }
}

// Generisanje GNU plota i grafika
void generate_plot() {
    ofstream gnuplot_script("plot_script.gp");

    gnuplot_script << "set terminal png size 1000, 800\n";
    gnuplot_script << "set output 'performance_comparison.png'\n";
    gnuplot_script << "set logscale xy\n";
    gnuplot_script << "set xlabel 'Input Size'\n";
    gnuplot_script << "set ylabel 'Time (s)'\n";
    gnuplot_script << "set title 'Performance Comparison of Convolution Algorithm'\n";
    gnuplot_script << "set key top left\n";
    gnuplot_script << "plot ";

    int counter = 0;

    for (const auto& opt : optimizations) {
        for (int thread : threads) {
            if(opt == "noopt" && thread == 2) continue;
            string filename = "data_" + opt + "_" + to_string(thread) + ".dat";
            ofstream data_file(filename);
            for (int size : sizes) {
                if (results.find({opt, thread, size}) != results.end()) {
                    data_file << size << " " << results[{opt, thread, size}] << "\n";
                }
            }
            data_file.close();

            if (counter++ > 0) gnuplot_script << ", ";
            gnuplot_script << "'" << filename << "' using 1:2 with linespoints title '" 
                           << opt << ", " << thread << " thread(s)'";
        }
    }

    {
        string gp_filename = "data_GP.dat";
        ofstream gp_data(gp_filename);
        for (int size : sizes) {
            if (gp_results.find(size) != gp_results.end()) {
                gp_data << size << " " << gp_results[size] << "\n";
            }
        }
        gp_data.close();

        gnuplot_script << ", '" << gp_filename << "' using 1:2 with linespoints title 'GPU variant'\n";
    }
    

    gnuplot_script << "\n";
    gnuplot_script << "set output 'performance_comparison_with_variance.png'\n";
    gnuplot_script << "set title 'Performance Comparison with Variance'\n";
    gnuplot_script << "set key top left\n";
    gnuplot_script << "plot ";

    counter = 0;
    for (const auto& opt : optimizations) {
        for (int thread : threads) {
            if(opt == "noopt" && thread == 2) continue;
            string filename = "data_var_" + opt + "_" + to_string(thread) + ".dat";
            ofstream data_file(filename);
            for (int size : sizes) {
                if (results.find({opt, thread, size}) != results.end()) {
                    data_file << size << " " << results[{opt, thread, size}] << " " 
                              << vars[{opt, thread, size}] << "\n";
                }
            }
            data_file.close();

            if (counter++ > 0) gnuplot_script << ", ";
            gnuplot_script << "'" << filename << "' using 1:2 with linespoints title '" 
                           << opt << ", " << thread << " thread(s)'";
            gnuplot_script << ", '" << filename << "' using 1:2:(sprintf('%.2e', $3)) with labels"
                           << " offset 0,1 notitle";
        }
    }

    {
        string gp_var_filename = "data_GP_var.dat";
        ofstream gp_var_data(gp_var_filename);
        for (int size : sizes) {
            if (gp_results.find(size) != gp_results.end()) {
                gp_var_data << size << " " << gp_results[size] << " " << gp_vars[size] << "\n";
            }
        }
        gp_var_data.close();


        gnuplot_script << ", '" << gp_var_filename << "' using 1:2 with linespoints title 'GPU variant'";

        gnuplot_script << ", '" << gp_var_filename << "' using 1:2:(sprintf('%.2e', $3)) with labels"
                       << " offset 0,1 notitle\n";
    }


    gnuplot_script.close();
    system("gnuplot plot_script.gp");
}

void save_all_results_in_file() {
    ofstream all_results("all_results.txt");
    for (const auto& opt : optimizations) {
        for (int thread : threads) {
            if(opt == "noopt" && thread == 2) continue;
            for (int size : sizes) {
                all_results << opt << " " << thread << " " << size << " " 
                            << results[{opt, thread, size}] << " " << vars[{opt, thread, size}] << "\n";
            }
        }
    }

    for (int size : sizes) {
        all_results << "GPU " << size << " " << gp_results[size] << " " << gp_vars[size] << "\n";
    }
    all_results.close();
}

int main(int argc, char* argv[]) {
    string results_fl = (argc > 1) ? argv[1] : "results";
    load_results(results_fl);
    generate_plot();
    save_all_results_in_file();
    return 0;
}
