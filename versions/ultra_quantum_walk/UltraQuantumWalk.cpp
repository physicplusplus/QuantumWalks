//
// Created by Josh Pughe-Sanford on 1/25/16.
//

#include <cstring>
#include "UltraQuantumWalk.h"

/* ARCHITECTURE
 * t is max time = N
 * mat[2(t+1)] = [  cos(theta@x=0)    sin(theta@x=0)    cos(theta@x=1)    sin(theta@x=1)    ...     cos(theta@x=i)    sin(theta@x=i)  ]
 *  idx          [  0               1               2               3                               2i                2i+1            ]
 *                  x=0,            x=0,            x=±1,           x=±1,           ...     x=±m,           x=±m
 *
 * A_j[4(N+1)] = [  aU_0,   aL_0,   aU_1,       aL_1,    ...,    aU_N,  aL_N,   aU_{N+1},   aL_{N+1},   ...,    aU_2N   aL_2N ]
 *               [  x=-N,   x=-N,   x=-N+1,     x=-N+1,  ...,    x=0,   x=0,    x=1,        x=1,        ...,    x=N     x=N   ]
 *  idx          [  0       1       2           3                2N     2N+1    2(N+1)      2(N+1)+1            2(2N)   2(2N)+1 --> psi_U: x = -N+i/2
 *                                                                                                                                  psi_L: x = -N+(i-1)/2
 */

double UltraQuantumWalk::A_r[];
double UltraQuantumWalk::A_i[];
double UltraQuantumWalk::B_r[];
double UltraQuantumWalk::B_i[];
double UltraQuantumWalk::mat[];

UltraQuantumWalk::UltraQuantumWalk(){
    double* IC = this->IC;
    long iter = this->iterations;
    A_r[2*iter] = IC[0];
    A_r[2*iter+1] = IC[1];
    A_i[2*iter] = IC[2];
    A_i[2*iter+1] = IC[3];
}
UltraQuantumWalk::UltraQuantumWalk(double ur, double ui, double lr, double li) {
    double* IC = this->IC;
    long iter = this->iterations;
    IC[0] = A_r[2*iter] = ur;
    IC[1] = A_r[2*iter+1] = lr;
    IC[2] = A_i[2*iter] = ui;
    IC[3] = A_i[2*iter+1] = li;
}
double powerOfTwo = 0;
bool UltraQuantumWalk::log_cond(double t){
    if(t >= pow(2, powerOfTwo)){
        powerOfTwo++;
        return true;
    } else return false;
}
void UltraQuantumWalk::generate_matrix_array(double e){
    long iter = this->iterations;
    int m = log(iter)/log(2);
    mat[0] = sin(M_PI_4);
    mat[1] = cos(M_PI_4);
    int j, k;
    for(int i = 1; i<=m; i++){
        int interval = pow(2, i-1);
        double theta_i = M_PI_4*pow(e, i);
        j=0;
        while(interval*(2*j+1)<=iter){
            k = interval*(2*j+1);
            mat[2*k]   = sin(theta_i);
            mat[2*k+1] = cos(theta_i);
//            std::cout << "X= " << k << " : " << theta_i << std::endl;
            j++;
        }
    }
}

void UltraQuantumWalk::flush() {
    memset(A_r, 0, sizeof(A_r));
    memset(A_i, 0, sizeof(A_i));
    memset(B_r, 0, sizeof(B_r));
    memset(B_i, 0, sizeof(B_i));
    A_r[2*iter] = IC[0];
    A_r[2*iter+1] = IC[1];
    A_i[2*iter] = IC[2];
    A_i[2*iter+1] = IC[3];
    this->_t = 0;
    powerOfTwo = 0;
    this->log_x.clear();
    this->log_t.clear();
}

void UltraQuantumWalk::step_simulation(long iter) {
    long t = this->_t;
    long I = this->iterations;
    double* mat = this->mat;
    double * src_r;
    double * src_i;
    double * dest_r;
    double * dest_i;
//    if(this->log_cond(t)) {
//        this->print_norm_array(A_r, A_i);
//    }
    long i;
    for(; t<iter; t++) {
//        this->print_norm_array();
//        this->print_amplitude_array();
        if(t%2==0){
            src_r  = A_r; src_i  = A_i;
            dest_r = B_r; dest_i = B_i;
            for(long x=-(t+1); x <= (t+1); x+=2){ // want to go from x=-(t+1) to (t+1) --> -N+i/2 = ±(t+1) --> i = 2N ± 2(t+1)
                i = 2*(I+x);
                //      psiU_{x} = M(x-1)_11*psiU_{x-1}+M(x-1)_12*psiL_{x-1}+M(x+1)_21*psiU_{x+1}+M(x+1)_22*psiL_{x+1}
                // -->  psi_{i} = psiU(x) = M(i/2+t)*psi_{i-2}+M(i/2+t+1)*psi_{i-1}+M(i/2+t+1)*psi_{i+2}-M(i/2+t)*psi_{i+3}
                dest_r[i]   = src_r[i-2]*mat[2*abs(x-1)]+src_r[i-1]*mat[2*abs(x-1)+1];   //real part of upper component
                dest_r[i+1] = src_r[i+2]*mat[2*abs(x+1)+1]-src_r[i+3]*mat[2*abs(x+1)];   //real part of lower component
                dest_i[i]   = src_i[i-2]*mat[2*abs(x-1)]+src_i[i-1]*mat[2*abs(x-1)+1];   //real part of upper component
                dest_i[i+1] = src_i[i+2]*mat[2*abs(x+1)+1]-src_i[i+3]*mat[2*abs(x+1)];   //imag part of lower component
            }
        } else {
            src_r  = B_r; src_i  = B_i;
            dest_r = A_r; dest_i = A_i;
            for(long x=-(t+1); x <= (t+1); x+=2){ // want to go from x=-(t+1) to (t+1) --> -N+i/2 = ±(t+1) --> i = 2N ± 2(t+1)
                i = 2*(I+x);
                dest_r[i]   = src_r[i-2]*mat[2*abs(x-1)]+src_r[i-1]*mat[2*abs(x-1)+1];   //real part of upper component
                dest_r[i+1] = src_r[i+2]*mat[2*abs(x+1)+1]-src_r[i+3]*mat[2*abs(x+1)];   //real part of lower component
                dest_i[i]   = src_i[i-2]*mat[2*abs(x-1)]+src_i[i-1]*mat[2*abs(x-1)+1];   //real part of upper component
                dest_i[i+1] = src_i[i+2]*mat[2*abs(x+1)+1]-src_i[i+3]*mat[2*abs(x+1)];   //imag part of lower component
            }
        }
    }
    this->_t = t;
}

void UltraQuantumWalk::run_simulation() {
    long iter = this->iterations;
    long t = this->_t;
    double* mat = this->mat;
    double * src_r;
    double * src_i;
    double * dest_r;
    double * dest_i;
//    if(this->log_cond(t)) {
//        this->print_norm_array(A_r, A_i);
//    }
    long i;
    for(long j = 0; j<iter; j++) {
//        this->print_norm_array();
//        this->print_amplitude_array();
        if(j%2==0){
            src_r  = A_r; src_i  = A_i;
            dest_r = B_r; dest_i = B_i;
            for(long x=-(t+1); x <= (t+1); x+=2){ // want to go from x=-(t+1) to (t+1) --> -N+i/2 = ±(t+1) --> i = 2N ± 2(t+1)
                i = 2*(iter+x);
                //      psiU_{x} = M(x-1)_11*psiU_{x-1}+M(x-1)_12*psiL_{x-1}+M(x+1)_21*psiU_{x+1}+M(x+1)_22*psiL_{x+1}
                // -->  psi_{i} = psiU(x) = M(i/2+t)*psi_{i-2}+M(i/2+t+1)*psi_{i-1}+M(i/2+t+1)*psi_{i+2}-M(i/2+t)*psi_{i+3}
                dest_r[i]   = src_r[i-2]*mat[2*abs(x-1)]+src_r[i-1]*mat[2*abs(x-1)+1];   //real part of upper component
                dest_r[i+1] = src_r[i+2]*mat[2*abs(x+1)+1]-src_r[i+3]*mat[2*abs(x+1)];   //real part of lower component
                dest_i[i]   = src_i[i-2]*mat[2*abs(x-1)]+src_i[i-1]*mat[2*abs(x-1)+1];   //real part of upper component
                dest_i[i+1] = src_i[i+2]*mat[2*abs(x+1)+1]-src_i[i+3]*mat[2*abs(x+1)];   //imag part of lower component
            }
        } else {
            src_r  = B_r; src_i  = B_i;
            dest_r = A_r; dest_i = A_i;
            for(long x=-(t+1); x <= (t+1); x+=2){ // want to go from x=-(t+1) to (t+1) --> -N+i/2 = ±(t+1) --> i = 2N ± 2(t+1)
                i = 2*(iter+x);
                dest_r[i]   = src_r[i-2]*mat[2*abs(x-1)]+src_r[i-1]*mat[2*abs(x-1)+1];   //real part of upper component
                dest_r[i+1] = src_r[i+2]*mat[2*abs(x+1)+1]-src_r[i+3]*mat[2*abs(x+1)];   //real part of lower component
                dest_i[i]   = src_i[i-2]*mat[2*abs(x-1)]+src_i[i-1]*mat[2*abs(x-1)+1];   //real part of upper component
                dest_i[i+1] = src_i[i+2]*mat[2*abs(x+1)+1]-src_i[i+3]*mat[2*abs(x+1)];   //imag part of lower component
            }
        }
        //update variables
        this->_t = t = t + this->_time_step;
        if(this->log_cond(t)) {
            //this->print_norm_array(); //calling this method is very slow and freezes clion for large t due to the lag of printing to console
            std::cout << "\tnsum:\t"<< this->norm_sum() << std::endl;
            this->generate_statistics(dest_r, dest_i);
        }
    }
}

double UltraQuantumWalk::norm_sum(){
    double sum = 0.0;
    double y,t;
    double c = 0.0;
    long T = this->_t;
    long iter = this->iterations;
    double norm;
    double * src_r; double * src_i;
    long X=-T;
    long i;
    if(T%2==0){src_r  = A_r; src_i  = A_i;} else { src_r  = B_r; src_i  = B_i;}
    for(; X <= T; X+=2){
        i = 2*(iter+X);
        norm = (src_r[i]*src_r[i]+src_i[i]*src_i[i])
               +(src_r[i+1]*src_r[i+1]+src_i[i+1]*src_i[i+1]);
        // Kahan Summation Algorithm: reduces rounding errors in low order bits
        y = norm - c;
        t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    return sum;
}

void UltraQuantumWalk::generate_statistics(double src_r[], double src_i[]){
    double x = 0.0;
    double y,t;
    double c = 0.0;
    long T = this->_t;
    long iter = this->iterations;
    double norm;
    double pos;
    long X=-(T+1)+1;
    long i;
    for(; X <= (T+1); X+=2){
        i = 2*(iter+X);
        pos = X;
        norm = (src_r[i]*src_r[i]+src_i[i]*src_i[i])
               +(src_r[i+1]*src_r[i+1]+src_i[i+1]*src_i[i+1]);
        // Kahan Summation Algorithm: reduces rounding errors in low order bits
        y = (norm * pos * pos) - c;
        t = x + y;
        c = (t - x) - y;
        x = t;
    }
    this->log_x.push_back(x);
    this->log_t.push_back(T);
    std::cout << "\tgenerating statistics\t(x2,t)->\t(" << x << ", " << T <<")"<< std::endl;
}

void UltraQuantumWalk::write_time(std::ofstream &file, bool endl){
    std::vector<double> t = this->log_t;
    for (std::vector<int>::size_type i = 0; i != t.size(); i++) file << t[i] << ",";
    if(endl) file << std::endl;
}

void UltraQuantumWalk::write_mds(std::ofstream &file, bool endl){
    std::vector<double> x = this->log_x;
    for(std::vector<int>::size_type i = 0; i != x.size(); i++) file << x[i] << ",";
    if(endl) file << std::endl;
}

void UltraQuantumWalk::write_norm_array(std::ofstream &file, bool endl){
    long T = this->_t;
    long iter = this->iterations;
    double norm;
    double * src_r; double * src_i;
    long i;
    file << T << ",";
    bool hasSeenNonZero = false;
    long bound = 1;
    if(T%2==0){src_r  = A_r; src_i  = A_i;} else { src_r  = B_r; src_i  = B_i;}
    for(long X=-T; X < (T+1); X+=2){
        i = 2*(iter+X);
        norm = (src_r[i]*src_r[i]+src_i[i]*src_i[i])+(src_r[i+1]*src_r[i+1]+src_i[i+1]*src_i[i+1]);
        if(!hasSeenNonZero){
            if(norm!=0.) {
                file << -X << ",";
                bound = -X;
                file << norm << ",";
                hasSeenNonZero = true;
            }
        } else file << norm << ",";
        if(X==bound) break;
    }
    if(endl) file << std::endl;
}

void UltraQuantumWalk::print_norm_array(){
    long T = this->_t;
    long iter = this->iterations;
    double norm;
    double * src_r; double * src_i;
    long i;
    if(T%2==0){src_r  = A_r; src_i  = A_i;} else { src_r  = B_r; src_i  = B_i;}
    std::cout << "[";
     for(long X=-T; X < (T+1); X+=2){
         i = 2*(iter+X);
         norm = (src_r[i]*src_r[i]+src_i[i]*src_i[i])+(src_r[i+1]*src_r[i+1]+src_i[i+1]*src_i[i+1]);
        std::cout << norm << " ";
    }
    std::cout << "] for t = " << T << std::endl;
}

