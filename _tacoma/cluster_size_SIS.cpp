/* 
 * The MIT License (MIT)
 * Copyright (c) 2019, Benjamin Maier
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

#include "cluster_size_SIS.h"

using namespace std;
//namespace cluster_size_SIS = cluster_size_SIS;

void cluster_size_SIS::update_network(
                    vector < set < size_t > > &_G,
                    double t
                    )
{
    // map this network to the current network
    G = &_G;

    // compute degree and R0
    size_t number_of_edges_times_two = 0;

    for(auto const &neighbors: *G)
        number_of_edges_times_two += neighbors.size();

    mean_degree = number_of_edges_times_two / (double) N;

    // update infected
    SI_edges.clear();

    // for each infected, check its neighbors.
    // if the neighbor is susceptible, push it to the
    // endangered susceptible vector
    for(auto const &inf: infected)
        for(auto const &neighbor_of_infected: (*G)[inf])
            if (node_status[neighbor_of_infected] == EPI::S)
                SI_edges.push_back( make_pair( inf, neighbor_of_infected ) );

    // update the arrays containing the observables
    update_observables(t);
}

void cluster_size_SIS::update_network(
                    vector < set < size_t > > &_G,
                    vector < pair < size_t, size_t > > &edges_in,
                    vector < pair < size_t, size_t > > &edges_out,
                    double t
                    )
{
    // map this network to the current network
    G = &_G;

    // compute degree and R0
    size_t number_of_edges_times_two = 0;

    for(auto const &neighbors: *G)
        number_of_edges_times_two += neighbors.size();

    mean_degree = number_of_edges_times_two / (double) N;
    
    // make searchable list
    set < pair < size_t, size_t > > set_of_out_edges(edges_out.begin(), edges_out.end());

    // erase all entries from SI which are part of the list of edges leaving
    SI_edges.erase(
            remove_if(
                    SI_edges.begin(), 
                    SI_edges.end(),
                [&set_of_out_edges](const pair < size_t, size_t > & edge) { 
                        size_t u = edge.first;
                        size_t v = edge.second;

                        if (v<u)
                            swap(u,v);

                        return set_of_out_edges.find( make_pair(u,v) ) != set_of_out_edges.end();
                }),
            SI_edges.end() 
            );

    for(auto const &e: edges_in)
    {
        size_t u = e.first;
        size_t v = e.second;

        if ((node_status[u] == EPI::S) and (node_status[v] == EPI::I))
            SI_edges.push_back(  make_pair( v, u ) );
        else if ((node_status[v] == EPI::S) and (node_status[u] == EPI::I))
            SI_edges.push_back(  make_pair( u, v ) );
    }

    // update the arrays containing the observables
    update_observables(t);
}

void cluster_size_SIS::get_rates_and_Lambda(
                    vector < double > &_rates,
                    double &_Lambda
                  )
{
    // delete current rates
    rates.clear();

    // compute rates of infection
    rates.push_back(infection_rate * SI_edges.size());

    // compute rates of recovery
    rates.push_back(recovery_rate * infected.size());

    // return those new rates
    _rates = rates;
    _Lambda = accumulate(rates.begin(),rates.end(),0.0);
}

void cluster_size_SIS::make_event(
                size_t const &event,
                double t
               )
{
    if (event == 0)
        infection_event();
    else if (event == 1)
        recovery_event();
    else
        throw length_error("cluster_size_SIS: chose event larger than rate vector which should not happen.");

    number_of_events++;
    update_observables(t);

}

void cluster_size_SIS::infection_event()
{
    // initialize uniform integer random distribution
    uniform_int_distribution < size_t > random_susceptible(0,SI_edges.size()-1);

    // find the index of the susceptible which will become infected
    size_t this_susceptible_index = random_susceptible(generator);

    // get the node number of this susceptible
    size_t this_susceptible = (SI_edges.begin() + this_susceptible_index)->second;

    // save this node as an infected
    infected.push_back(this_susceptible);

    // change node status of this node
    node_status[this_susceptible] = EPI::I;
    
    // add node to the covered nodes
    covered_nodes.insert(this_susceptible);

    // erase all edges in the SI set where this susceptible is part of
    SI_edges.erase( 
            remove_if( 
                    SI_edges.begin(), 
                    SI_edges.end(),
                [&this_susceptible](const pair < size_t, size_t > & edge) { 
                        return edge.second == this_susceptible;
                }),
            SI_edges.end() 
        );

    // push the new SI edges
    size_t & this_infected = this_susceptible;
    for(auto const &neighbor: (*G)[this_infected])
        if (node_status[neighbor] == EPI::S)
            SI_edges.push_back( make_pair(this_infected, neighbor) );

}

void cluster_size_SIS::recovery_event()
{
    // initialize uniform integer random distribution
    uniform_int_distribution < size_t > random_infected(0,infected.size()-1);

    // find the index of the susceptible which will become infected
    size_t this_infected_index = random_infected(generator);
    auto it_infected = infected.begin() + this_infected_index;

    // get the node id of this infected about to be recovered
    size_t this_infected = *(it_infected);

    // delete this from the infected vector
    infected.erase( it_infected );
    initially_infected.erase( this_infected );

    // change node status of this node
    node_status[this_infected] = EPI::S;

    // erase all edges in the SI set which this infected is part of
    SI_edges.erase( 
            remove_if( 
                    SI_edges.begin(), 
                    SI_edges.end(),
                [&this_infected](const pair < size_t, size_t > & edge) { 
                        return edge.first == this_infected;
                }),
            SI_edges.end() 
        );

    size_t const & this_susceptible = this_infected;

    // push the new SI edges
    for(auto const &neighbor: (*G)[this_susceptible])
        if (node_status[neighbor] == EPI::I)
            SI_edges.push_back( make_pair(neighbor, this_susceptible) );


}

void cluster_size_SIS::update_observables(
                double t
               )
{
    if (sampling_dt > 0.0)
    {
        if (t >= next_sampling_time)
        {
            double _R0 = infection_rate * mean_degree / recovery_rate;
            R0.push_back(_R0);

            // compute SI
            SI.push_back(SI_edges.size());

            // compute I
            I.push_back(infected.size());

            // push back time
            time.push_back(t);

            // advance next sampling time
            do
            {
                next_sampling_time += sampling_dt;
            } while (next_sampling_time < t);
        }
    }
    else if (sampling_dt == 0.0)
    {
        double _R0 = infection_rate * mean_degree / recovery_rate;
        R0.push_back(_R0);

        // compute SI
        SI.push_back(SI_edges.size());

        // compute I
        I.push_back(infected.size());

        // push back time
        time.push_back(t);

    }

    if (simulation_ended())
    {
        lifetime = t;
        coverage = covered_nodes.size();
        cluster_size = infected.size();
    }
}

