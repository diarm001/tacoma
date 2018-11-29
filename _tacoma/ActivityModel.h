/* 
 * The MIT License (MIT)
 * Copyright (c) 2018, Benjamin Maier
 *
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated documentation 
 * files (the "Software"), to deal in the Software without 
 * restriction, including without limitation the rights to use, 
 * copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall 
 * be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-
 * INFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS 
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN 
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 */

#ifndef __ACTIVITY_MODEL_CLASS_H__
#define __ACTIVITY_MODEL_CLASS_H__

#include "Events.h"
#include "Utilities.h"
#include "ResultClasses.h"
#include "activity_model.h"

//#include <iostream>
//#include <algorithm>
#include <stdexcept>
#include <vector>
#include <set>
#include <utility>
#include <random>
#include <cmath>
#include <numeric>
#include <random>
#include <ctime>
#include <tuple>
#include <assert.h>

using namespace std;

class ActivityModel 
{
    public:
        size_t N;
        double rho;
        double omega;
        double omega_minus;
        double omega_plus;
        size_t seed;
        bool verbose;
        bool save_temporal_network;

        vector < set < size_t > > G;

        mt19937_64 * generator;
        uniform_real_distribution<double> randuni;

        edge_changes edg_chg;

        ActivityModel(
            size_t _N,
            double _rho,
            double _omega,
            bool _save_temporal_network = false,
            size_t _seed = 0,
            bool _verbose = false
        )
        {
            N = _N;
            rho = _rho;
            omega = _omega;
            verbose = _verbose;
            seed = _seed;
            save_temporal_network = _save_temporal_network;

            generator = new mt19937_64;
            randuni = uniform_real_distribution < double > (0.0, 1.0);

            if ((rho<=0.0) or (rho>=1.0))
                throw domain_error("rho has to be 0 < rho < 1.");
            if (omega <= 0.0)
                throw domain_error("omega has to be omega > 0");


            double omega_minus = omega / rho;
            double omega_plus = omega / (1.0 - rho);

            if (verbose)
            {
                cout << "omega = " << omega << endl;
                cout << "rho = " << rho << endl;
                cout << "omega+ = " << omega_plus << endl;
                cout << "omega- = " << omega_minus << endl;
            }

        }

        void reset() 
        {
            // reset observables
            edg_chg.N = N;

            edg_chg.t.push_back(0.0);

            // seed engine
            if (seed == 0)
                randomly_seed_engine(*generator);
            else
                generator->seed(seed);

            G = get_random_graph(N, rho, *generator);


            if (save_temporal_network)
                edgelist_from_graph(edg_chg.edges_initial, G);

            edges_on = 0;
            m_max = (N * (N-1)) / 2;

            for(size_t node=0; node<N; node++)
            {
                size_t this_k = G[node].size();
                k.push_back(this_k);
                complementary_k.push_back(N - 1 - this_k);

                edges_on += this_k;

                if (verbose)
                {
                    cout << "k[node]   = " << this_k << endl;
                    cout << "c_k[node] = " << complementary_k.back() << endl;
                }
            }

            edges_on /= 2;

    
        }

        void get_rates_and_Lambda(vector < double > &rates,
                                  double &Lambda
                                  );

        void make_event(size_t const &event,
                        double t,
                        vector < pair < size_t, size_t > > &e_in,
                        vector < pair < size_t, size_t > > &e_out
                       );

        void print()
        {
        }

        void set_generator(mt19937_64 &_generator)
        {
            generator = &_generator;
        } 

    private:
        vector < size_t > k;
        vector < size_t > complementary_k;
        size_t edges_on;
        size_t m_max;


        void print_internal()
        {
        }



};

#endif
