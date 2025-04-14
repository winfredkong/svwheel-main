// Program to iterate through all strategies to find optimal
// g++ -I .\Eigen\ -o ..\naive_search.exe .\naive_search.cpp
// Sample command: ..\naive_search.exe -t 10 -i 3 -p 0.5 -r 3.5
#include <iostream>
#include <unordered_map>
#include <cmath>
#include "./Eigen"

using namespace std;

float compute_EHT(int* strat, int length, int init, float p, float r_cost){
    /* Computes EHT based on strategy. 
    Inputs:
    - strat: pointer to start of array
    - length: length of array/should be the target wealth as well
    - init: Reset wealth
    - p: probability of winning
    - r_cost: reset cost 
    Output:
    - float: Expected Hitting Time
    */
    Eigen::MatrixXf A = Eigen::MatrixXf::Zero(length, length);
    Eigen::VectorXf b = Eigen::VectorXf::Ones(length);
     
    for (int i=0; i<length; i++){
        if (*(strat+i)==0){ //Reset. x_i = x_init + r_cost
            b(i) = r_cost;
            A(i, i) = 1;
            A(i, init) = -1;
        }
        else{ // x_i = p x_(i+j) {if i+j>=length then drop this term} + (1-p)x_(i-j) + 1 
            A(i, i) = 1;
            if (i+*(strat+i)<length){
                A(i, i+*(strat+i)) = -p;
            } 
            A(i, i-*(strat+i)) = -(1-p);
        }
    }

    // Solve for Ax=b
    Eigen::ColPivHouseholderQR<Eigen::MatrixXf> dec(A);
    Eigen::VectorXf x = dec.solve(b);   
    return x(init); // May want to change this in the future to log more diagnostic data
}

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
    float p = -1.0, r_cost = -1.0;
    int target = -1, init=-1, r=-1;
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
    
    /*  Iterate through possible values of x_r, x_a
    x_r is the largest integer wealth to reset
    x_a is the smallest integer wealth to all in
    */
    float best_score = -1; // Best EHT 
    float score;
    int best_strat[target];
    for (int x_r = 0; x_r < init; x_r++ ){
        // Check for corollary 2.4
        if (x_r + 1>=(int)target / 2 ){
            int strat[target];
            // index i represents decision at wealth i. 0 represents reset

            // Set strats below reset threshold to reset
            for (int u = 0; u <= x_r;  u++){
                strat[u] = 0;
            }
            // Set strats below reset threshold to all-in
            for (int u = x_r+1; u < target; u++){
                strat[u] = target-u;
            }

            // eval EHT
            score = compute_EHT(&strat[0], target, init, p, r_cost);
            if (score<best_score or best_score==-1){
                best_score = score;
                for  (int i=0; i<target; i++){
                    best_strat[i] = strat[i];
                }
            }
            continue;
        }

        int inthalf = static_cast<int>(ceil(target/2));
        for (int x_a = max(x_r+1, inthalf) ; x_a<target ; x_a++){
            int strat[target];
            // index i represents decision at wealth i. 0 represents reset
            // Set strats below reset threshold to reset
            for (int u = 0; u <= x_r;  u++){
                strat[u] = 0;
            }
            strat[x_r + 1] = min(target-x_r-1, x_r+1); // Fixed by lemma 2.3
            // Set strats above all-in threshold to all-in
            for (int u = x_a; u < target; u++){
                strat[u] = target-u;
            }
            if ((x_a == x_r + 1)){
                // Eval EHT
                score = compute_EHT(&strat[0], target, init, p, r_cost);
                if (score<best_score or best_score==-1){
                    best_score = score;
                    for  (int i=0; i<target; i++){
                        best_strat[i] = strat[i];
                    }
                }
            }
            else{
                // Start iterating the middle wealth, then eval EHT
                // Start with largest possible bets then decrement by 1 each time
                for (int i = x_r+1; i < x_a; i++){
                    strat[i] = min(target - i -1, i); // Since below all in threshold
                }

                //eval EHT
                score = compute_EHT(&strat[0], target, init, p, r_cost);
                if (score<best_score or best_score==-1){
                    best_score = score;
                    for  (int i=0; i<target; i++){
                        best_strat[i] = strat[i];
                    }
                }
                /*
                We decrement from the left most position.
                Noting that lemma 2.3, optimal at x_r+1 must be to bet everything
                */
                int current = x_r +2; //current position to decrement
                while (sum_array(&strat[x_r+2], x_a - x_r - 2) > x_a - x_r - 2){
                    // Stops loop only if all positions bet 1, i.e. lowest possible
                    if (strat[current]==1 || current == x_r+1){ 
                        // cannnot decrement anymore. carry over
                        strat[current] = min(target - current -1, current);
                        current = current + 1;
                        continue;
                    }
                    else if (current-strat[current]+1 < x_r){
                        // Recall no need to check suboptimal. So when decrementing skip
                        strat[current] = current - x_r - 1;
                        if (current!=x_r+2){
                            // Reset if carryover
                            current = x_r+2;
                        }
                        // Eval EHT
                        score = compute_EHT(&strat[0], target, init, p, r_cost);
                        if (score<best_score or best_score==-1){
                            best_score = score;
                            for  (int i=0; i<target; i++){
                                best_strat[i] = strat[i];
                            }
                        }


                        cout << "Reset threshold at: " << x_r << ". ";
                        cout << "All-in threshold at: " << x_a << ". ";
                        for (int i=0; i<target;i++){
                            cout << strat[i] << "," ;
                        }
                        cout << endl;
                    }
                    else{
                        // Decrement by one and evaluate
                        strat[current] = strat[current] - 1;
                        if (current!=x_r+2){
                            // Reset if carryover
                            current = x_r+2;
                        }
                        // Eval EHT
                        score = compute_EHT(&strat[0], target, init, p, r_cost);
                        if (score<best_score or best_score==-1){
                            best_score = score;
                            for  (int i=0; i<target; i++){
                                best_strat[i] = strat[i];
                            }
                        }


                        cout << "Reset threshold at: " << x_r << ". ";
                        cout << "All-in threshold at: " << x_a << ". ";
                        for (int i=0; i<target;i++){
                            cout << strat[i] << "," ;
                        }
                        cout << endl;
                    }
                    
                }
            }

        }
    }
    // unordered_map<int, int> umap;
    cout << "The best strat is: ";
    for (int i=0; i<target;i++){
        cout << best_strat[i] << "," ;
    }
    cout << endl << "The EHT is: " << best_score;
    return 0;
}

/*


int test[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    cout << add_array(&test[5], 3) << endl;
*/