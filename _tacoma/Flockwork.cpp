/* 
 * The MIT License (MIT)
 * Copyright (c) 2016, Benjamin Maier
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

#include "Flockwork.h"

using namespace std;

void flockwork_timestep(
                 vector < set < size_t  > * > &G, //Adjacency matrix
                 double Q,       //probability to connect with neighbors of neighbor
                 mt19937_64 & generator, 
                 uniform_real_distribution<double> & distribution
            )
{
    //choose two nodes
    size_t N = G.size();
    size_t i,j;

    uniform_int_distribution<size_t> node1(0,N-1);
    uniform_int_distribution<size_t> node2(0,N-2);

    i = node1(generator);
    j = node2(generator);

    if (j>=i)
        j++;

    //loop through the neighbors of i
    for(auto neigh_i : *G[i] )
    {
        //and erase the link to i
        G[neigh_i]->erase(i);
    } 

    //erase the links from the perspective of i
    G[i]->clear();

    //loop through the neighbors of j
    for(auto neigh_j : *G[j] ) 
    {
        //add as neighbor of i if a random number is smaller than Q
        if ( distribution(generator) < Q )
        {
            G[ neigh_j ]->insert( i );
            G[ i ]->insert( neigh_j );
        }
    }

    //add j as neighbor
    G[ j ]->insert( i );
    G[ i ]->insert( j );
}

void flockwork_P_timestep(
                 vector < set < size_t  > * > &G, //Adjacency matrix
                 double P,       //probability to connect with neighbors of neighbor
                 mt19937_64 & generator, 
                 uniform_real_distribution<double> & distribution
            )
{
    //choose two nodes
    size_t N = G.size();
    size_t i,j;

    uniform_int_distribution<size_t> node1(0,N-1);
    uniform_int_distribution<size_t> node2(0,N-2);

    i = node1(generator);
    j = node2(generator);

    if (j>=i)
        j++;

    //loop through the neighbors of i
    for(auto neigh_i : *G[i] )
    {
        //and erase the link to i
        G[neigh_i]->erase(i);
    } 

    //erase the links from the perspective of i
    G[i]->clear();

    //add as neighbor of i if a random number is smaller than P
    if ( distribution(generator) < P )
    {
        //loop through the neighbors of j
        for(auto neigh_j : *G[j] ) 
        {
            G[ neigh_j ]->insert( i );
            G[ i ]->insert( neigh_j );
        }

        //add j as neighbor
        G[ j ]->insert( i );
        G[ i ]->insert( j );
    }
}

size_t flockwork_P_timestep_monitoring_groups(
                 vector < set < size_t  > * > &G, //Adjacency matrix
                 double P,       //probability to connect with neighbors of neighbor
                 size_t t,
                 vector < size_t > &group_start_times,
                 mt19937_64 & generator, 
                 uniform_real_distribution<double> & distribution
            )
{
    //choose two nodes
    size_t N = G.size();
    size_t i,j;

    uniform_int_distribution<size_t> node1(0,N-1);
    uniform_int_distribution<size_t> node2(0,N-2);

    i = node1(generator);
    j = node2(generator);

    if (j>=i)
        j++;

    size_t lifetime = 0;
    bool is_reconnection_event = (distribution(generator) < P);

    //if ( (G[i]->size()==1) && (is_reconnection_event) && (*(G[i]->begin())==j))
    //    return 0;

    // check if potentially this is the end of a group
    // if node cutting its edges is in a pair and it won't reconnect
    // or if it's in a pair, will reconnect, but not with its former neighbor
    size_t old_neighbor = N;
    if ( G[i]->size() == 1 ) 
    {
        old_neighbor = *(G[i]->begin());
    }
    if ( ( G[i]->size()==1) && 
         ( (!is_reconnection_event) || ( 
                                        (is_reconnection_event) && 
                                        (old_neighbor != j) 
                                       )
         )
       )
    {
        lifetime = t+1 - group_start_times[i];
        group_start_times[i] = 0;
        group_start_times[old_neighbor] = 0;
    }

    //loop through the neighbors of i
    for(auto neigh_i : *G[i] )
    {
        //and erase the link to i
        G[neigh_i]->erase(i);
    } 

    //erase the links from the perspective of i
    G[i]->clear();

    //add as neighbor of i if a random number is smaller than P
    if (is_reconnection_event)
    {

        if ( (G[j]->size() == 0) && (j != old_neighbor))
        {
            group_start_times[i] = t+1;
            group_start_times[j] = t+1;
        }
        else
        {
            group_start_times[i] = group_start_times[j];
            if (group_start_times[i]==0)
            {
                throw domain_error("group got started at time 0 which should not happen"); 
            }
        }

        //loop through the neighbors of j
        for(auto neigh_j : *G[j] ) 
        {
            G[ neigh_j ]->insert( i );
            G[ i ]->insert( neigh_j );
        }

        //add j as neighbor
        G[ j ]->insert( i );
        G[ i ]->insert( j );
    }
    else
    {
        group_start_times[i] = 0;
    }

    return lifetime;
}

void equilibrate_neighborset(
                 vector < set < size_t > * > &G, //neighborset
                 const double PQ,       //probability to connect with neighbors of neighbor
                 mt19937_64 & generator, 
                 uniform_real_distribution<double> & distribution,
                 size_t t_max,
                 const bool use_P_as_Q
                )
{
    size_t N = G.size();

    //if no maximum time is given, calculate analytical estimation
    if (t_max==0)
    {
        t_max = 20.0/(2.0 - 2.0*PQ) * N;
    }

    if (t_max>1000000)
        t_max = 1000000;

  
    //equilibrate
    for(size_t t=0; t<t_max; t++)
    {
        if (use_P_as_Q)
            flockwork_P_timestep(G,PQ,generator,distribution);
        else
            flockwork_timestep(G,PQ,generator,distribution);
    }
}

vector < pair < size_t, size_t > > 
     equilibrate_edgelist_generator(
                 vector < pair < size_t, size_t > > E, //edgelist
                 const size_t N,       //number of nodes
                 const double PQ,       //probability to connect with neighbors of neighbor
                 mt19937_64 & generator, 
                 uniform_real_distribution<double> & distribution,
                 size_t t_max,
                 const bool use_Q_as_P
                 )
{
    //initialize Graph vector
    vector < set < size_t > * > G;

    for(size_t node=0; node<N; node++)
    {
        G.push_back(new set < size_t >);
    }

    //loop through edge list and push neighbors
    for(auto edge: E)
    {
        size_t i = edge.first;
        size_t j = edge.second;
        G[ i ]->insert( j );
        G[ j ]->insert( i );
    }

    equilibrate_neighborset(G,PQ,generator,distribution,t_max,use_Q_as_P);

    //convert back to edge list
    vector < pair < size_t,size_t > > new_E;

    //loop through graph
    for(size_t node=0; node<N; node++)
    {
        //loop through nodes' neighbors and add edge
        for(auto const& neigh: *G[node])
        {
            new_E.push_back( get_sorted_pair(node,neigh) );
        }

        //delete created set
        delete G[node];
    }

    return new_E;
}


vector < pair < size_t, size_t > > 
     equilibrate_edgelist_seed(
                 vector < pair < size_t, size_t > > E, //edgelist
                 const size_t N,       //number of nodes
                 const double PQ,       //probability to connect with neighbors of neighbor
                 const size_t seed,
                 size_t t_max,
                 const bool use_Q_as_P
                 )
{
    //initialize Graph vector
    vector < set < size_t > * > G;

    for(size_t node=0; node<N; node++)
    {
        G.push_back(new set < size_t >);
    }

    //loop through edge list and push neighbors
    for(auto edge: E)
    {
        size_t i = edge.first;
        size_t j = edge.second;
        G[ i ]->insert( j );
        G[ j ]->insert( i );
    }

    //initialize random generators
    mt19937_64 generator;
    seed_engine(generator,seed);
    uniform_real_distribution<double> uni_distribution(0.,1.);

    equilibrate_neighborset(G,PQ,generator,uni_distribution,t_max,use_Q_as_P);

    //convert back to edge list
    vector < pair < size_t,size_t > > new_E;

    //loop through graph
    for(size_t node=0; node<N; node++)
    {
        //loop through nodes' neighbors and add edge
        for(auto const& neigh: *G[node])
        {
            new_E.push_back( get_sorted_pair(node,neigh) );
        }

        //delete created set
        delete G[node];
    }

    return new_E;
}

vector < pair < size_t, size_t > > 
     simulate_flockwork(
                 vector < pair < size_t, size_t > > E, //edgelist
                 const size_t N,       //number of nodes
                 const double Q,       //probability to connect with neighbors of neighbor
                 const size_t seed,
                 size_t num_timesteps
                 )
{
    //initialize Graph vector
    vector < set < size_t > * > G;

    for(size_t node=0; node<N; node++)
    {
        G.push_back(new set < size_t >);
    }

    //loop through edge list and push neighbors
    for(auto edge: E)
    {
        size_t i = edge.first;
        size_t j = edge.second;
        G[ i ]->insert( j );
        G[ j ]->insert( i );
    }

    //initialize random generators
    mt19937_64 generator;
    seed_engine(generator,seed);
    uniform_real_distribution<double> uni_distribution(0.,1.);

    //equilibrate
    for(size_t t=0; t<num_timesteps; t++)
        flockwork_timestep(G,Q,generator,uni_distribution);

    //convert back to edge list
    vector < pair < size_t,size_t > > new_E;

    //loop through graph
    for(size_t node=0; node<N; node++)
    {
        //loop through nodes' neighbors and add edge
        for(auto const& neigh: *G[node])
        {
            new_E.push_back( get_sorted_pair(node,neigh) );
        }

        //delete created set
        delete G[node];
    }

    return new_E;
}

vector < pair < size_t, size_t > > 
     simulate_flockwork_P(
                 vector < pair < size_t, size_t > > E, //edgelist
                 const size_t N,       //number of nodes
                 const double P,       //probability to connect with neighbors of neighbor
                 const size_t seed,
                 size_t num_timesteps
                 )
{
    //initialize Graph vector
    vector < set < size_t > * > G;

    for(size_t node=0; node<N; node++)
    {
        G.push_back(new set < size_t >);
    }

    //loop through edge list and push neighbors
    for(auto edge: E)
    {
        size_t i = edge.first;
        size_t j = edge.second;
        G[ i ]->insert( j );
        G[ j ]->insert( i );
    }

    //initialize random generators
    mt19937_64 generator;
    seed_engine(generator,seed);
    uniform_real_distribution<double> uni_distribution(0.,1.);

    //equilibrate
    for(size_t t=0; t<num_timesteps; t++)
        flockwork_P_timestep(G,P,generator,uni_distribution);

    //convert back to edge list
    vector < pair < size_t,size_t > > new_E;

    //loop through graph
    for(size_t node=0; node<N; node++)
    {
        //loop through nodes' neighbors and add edge
        for(auto const& neigh: *G[node])
        {
            new_E.push_back( get_sorted_pair(node,neigh) );
        }

        //delete created set
        delete G[node];
    }

    return new_E;
}

vector < size_t >
     simulate_flockwork_P_group_life_time(
                 const size_t N,       //number of nodes
                 const double P,       //probability to connect with neighbors of neighbor
                 const size_t seed,
                 size_t num_timesteps
                 )
{
    //initialize Graph vector
    vector < set < size_t > * > G;

    for(size_t node=0; node<N; node++)
    {
        G.push_back(new set < size_t >);
    }

    //get component vector
    vector < size_t > group_start_times;
    for(size_t node=0; node<N; node++)
    {
        group_start_times.push_back(0);
    }

    //initialize random generators
    mt19937_64 generator;
    seed_engine(generator,seed);
    uniform_real_distribution<double> uni_distribution(0.,1.);

    //simulate
    vector < size_t > lifetimes;
    for(size_t t=0; t<num_timesteps; t++)
    {
        size_t lifetime = flockwork_P_timestep_monitoring_groups(G,P,t,group_start_times,generator,uni_distribution);
        if (lifetime>0)
            lifetimes.push_back(lifetime);
    }

    //loop through graph
    for(size_t node=0; node<N; node++)
    {
        //delete created set
        delete G[node];
    }

    return lifetimes;
}

