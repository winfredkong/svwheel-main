// Program to iterate through all strategies to find optimal
// g++ -I .\Eigen\ -o ..\value_iteration.exe .\value_iteration.cpp
// Sample command: ..\value_iteration.exe -t 10 -i 3 -p 0.5 -r 3.5
#include <iostream>
#include <vector>
//#include <unordered_map>
//#include <cmath>
//#include "./Eigen"

using namespace std;

int sum_array(int* array, int length){
    /* Add elements of integer array. Pass by reference.
    Inputs:
    - array: pointer to start of array
    - length: length of array to sum
    Output:
    - integer sum of elements in array
    */
    int sum = 0;
    for (int i=0; i<length; i++){
        sum = sum + *(array+i);
    }
    return sum;
}

int main(int argc,char* argv[]) {
    float p = -1.0, r_cost = -1.0, eps = 0.0001;
    int target = -1, init=-1, r=-1, max_iter=10000;
    // Simple parser
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-t" && i + 1 < argc) {
            target = std::atoi(argv[++i]);
        } else if (arg == "-i" && i + 1 < argc) {
            init = std::atoi(argv[++i]);
        } else if (arg == "-p" && i + 1 < argc) {
            p = std::atof(argv[++i]);
        } else if (arg == "-r" && i + 1 < argc) {
            r_cost = std::atof(argv[++i]);
        } else if (arg == "-e" && i + 1 < argc) {
            eps = std::atof(argv[++i]); // optional arg
        } else if (arg == "-m" && i + 1 < argc) {
            max_iter = std::atoi(argv[++i]); // optional arg
        } else {
            std::cerr << "Unknown or malformed argument: " << arg << '\n';
            return 1;
        }
    }

    // Validate that all are set
    if (target == -1 || init == -1 || p == -1.0 || r_cost == -1) {
        std::cerr << "Usage: program -t <target> -i <init> -p <probability> -r <reset_cost>\n";
        return 1;
    }
    
    /*  Initialise value and strat
    */
    vector<int> strat(target);
    vector<float> value(target, -1);
    int count, iter=0;

    while (1){ // Value iteration Loop. Value = -EHT
        count = 0;
        iter++;
        // 1 Sweep for value
        for (int i=0; i<target; i++){
            float max_val = 1;
            for (int j=1; j<=i; j++){
                // j = bet size
                float est_val;
                if (i+j>=target) est_val = (1-p)*value[i-j]-1;
                else est_val = p*value[i+j] + (1-p)*value[i-j] -1;
                if (max_val==1 or est_val>max_val) max_val = est_val;
            }
            // Check reset
            if (max_val==1 or value[init]-r_cost>max_val) max_val = value[init]-r_cost;
            //Update value
            if (abs(value[i]-max_val) < eps) count++;
            value[i] = max_val;
        }
        if (count==target or iter==max_iter) break; // Stop looping if all values change < eps
    }
    
    // Find optimal Strat
    for (int i=0; i<target; i++){
        float max_val = 1;
        int best_strat = -1;
        for (int j=1; j<=i; j++){
            // j = bet size
            float est_val;
            if (i+j>=target) est_val = (1-p)*value[i-j]-1;
            else est_val = p*value[i+j] + (1-p)*value[i-j]-1;
            if (max_val==1 or est_val>max_val) {
                max_val = est_val;
                best_strat = j;
            }
        }
        // Check reset
        if (max_val==1 or value[init]-r_cost>max_val){
            max_val = value[init]-r_cost;
            best_strat = 0;
        }
        //Update strat
        strat[i] = best_strat;
    }
    
    // unordered_map<int, int> umap;
    cout << "The best strat is: ";
    for (int i=0; i<target;i++){
        cout << strat[i] << "," ;
    } cout << '\n';
    cout << endl << "The expected hitting times are: ";
    for (int i=0; i<target;i++){
        cout << -value[i] << "," ;
    }
    return 0;
}

/*


int test[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    cout << add_array(&test[5], 3) << endl;
*/