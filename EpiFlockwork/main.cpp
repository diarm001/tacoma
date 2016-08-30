#include "SIS.h"
#include "SIR.h"
#include "ResultClasses.h"

using namespace std;

int main()
{
    size_t N = 500;
    vector < tuple < size_t, size_t  > > E;
    double Q = 0.1;
    size_t t_run_total = 0;
    double recovery_rate = 0.01;
    double rewiring_rate = 1.;
    size_t number_of_vaccinated = 0;
    size_t number_of_infected = 10;
    size_t seed = 1;
    double R0 = 1.;
    double k = 1. / (1.-Q);
    double p = k / (N-1);
    bool use_random_rewiring = true;

    double infection_rate = R0 * recovery_rate / k;

    default_random_engine generator(seed);
    uniform_real_distribution<double> uni_distribution(0.,1.);


    //start with ER network
    for (size_t i=0; i<N-1; i++)
        for (size_t j=i+1; j<N; j++)
        {
            if (uni_distribution(generator)<p) 
            {
                E.push_back( make_tuple(i,j) );
                //cout << get<0>(E.back()) << " " << get<1>(E.back()) << endl;
            }
        }


    SIR_result result;

    result = SIR(E,N,Q,
                 infection_rate,
                 recovery_rate,
                 rewiring_rate,
                 t_run_total,
                 number_of_vaccinated,
                 number_of_infected,
                 use_random_rewiring,
                 seed
             );


    for(auto p: result.I_of_t)
    {
        cout << p.first << " " << p.second << endl;
    }

    for(auto p: result.R_of_t)
    {
        cout << p.first << " " << p.second << endl;
    }

    return 0;
}
