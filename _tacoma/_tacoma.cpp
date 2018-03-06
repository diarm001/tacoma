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

#include <algorithm>
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
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "EqFlockwork.h"
#include "FW_P_varying.h"
#include "ResultClasses.h"
#include "test_varying_rate.h"
#include "ZSBB_model.h"
#include "dyn_RGG.h"
#include "measurements.h"
#include "dtu_week.h"
#include "resampling.h"
#include "social_trajectories.h"
#include "dyn_SIS.h"
#include "dyn_gillespie.h"

using namespace std;
namespace py = pybind11;

PYBIND11_PLUGIN(_tacoma) {
    py::module m("_tacoma", "TemporAl COntact Modeling and Analysis. Provides fast tools to analyze temporal contact networks and simulate Gillespie processes on them.");
    
    m.def("flockwork_P_varying_rates", &flockwork_P_varying_rates, "Simulate a flockwork P-model given an initial state as an edge list with varying rewiring rate and varying P. Returns time points and concurrent edge changes.",
            py::arg("E"),
            py::arg("N"),
            py::arg("P"),
            py::arg("t_run_total"),
            py::arg("rewiring_rate"),
            py::arg("tmax"),
            py::arg("use_random_rewiring") = false,
            py::arg("equilibrate_flockwork") = false,
            py::arg("seed") = 0
         );

    m.def("flockwork_P_varying_rates_neighbor_affinity", &flockwork_P_varying_rates_neighbor_affinity, "Simulate a flockwork P-model given an initial state as an edge list with varying rewiring rate and varying P. Rewiring neighbors are chosen according to a neighbor affinity value. Returns time points and concurrent edge changes.",
            py::arg("E"),
            py::arg("N"),
            py::arg("P"),
            py::arg("t_run_total"),
            py::arg("rewiring_rate"),
            py::arg("neighbor_affinity"),
            py::arg("tmax"),
            py::arg("use_random_rewiring") = false,
            py::arg("equilibrate_flockwork") = false,
            py::arg("use_preferential_node_selection") = false,
            py::arg("use_unweighted_k_for_selection") = false,
            py::arg("seed") = 0
         );

    m.def("equilibrate_flockwork_Q", &equilibrate_edgelist_seed, "Equilibrates a flockwork given an initial state as an edge list. Returns new edge list.",
            py::arg("E"),
            py::arg("N"),
            py::arg("Q"),
            py::arg("seed"),
            py::arg("t_max") = 0,
            py::arg("use_Q_as_P") = false
            );

    m.def("equilibrate_flockwork_P", &equilibrate_edgelist_seed, "Equilibrates a flockwork P-model given an initial state as an edge list. Returns new edge list.",
            py::arg("E"),
            py::arg("N"),
            py::arg("P"),
            py::arg("seed"),
            py::arg("t_max") = 0,
            py::arg("use_Q_as_P") = true
            );

    m.def("simulate_flockwork_Q", &simulate_flockwork, "Simulates a flockwork given an initial state as an edge list. Returns new edge list.",
            py::arg("E"),
            py::arg("N"),
            py::arg("Q"),
            py::arg("seed"),
            py::arg("num_timesteps")
            );

    m.def("simulate_flockwork_P", &simulate_flockwork_P, "Simulates a flockwork P-model given an initial state as an edge list. Returns new edge list.",
            py::arg("E"),
            py::arg("N"),
            py::arg("P"),
            py::arg("seed"),
            py::arg("num_timesteps")
            );

    m.def("simulate_flockwork_P_group_lifetimes", &simulate_flockwork_P_group_life_time, "Simulates a flockwork P-model with initial state of all nodes unconnected. Returns a list of group life times.",
            py::arg("N"),
            py::arg("P"),
            py::arg("seed"),
            py::arg("num_timesteps")
            );

    m.def("measure_group_sizes_and_durations_for_edge_lists", &measure_group_sizes_and_durations, "Get a temporal network as a list of edge lists, list of times and number of nodes N and return a list of contact durations, a list of group size histograms and a list of durations lists, one for each group size.",
            py::arg("edge_lists"),
            py::arg("ignore_size_histograms") = false,
            py::arg("verbose") = false
         );

    m.def("measure_group_sizes_and_durations_for_edge_changes", &measure_group_sizes_and_durations_for_edge_changes, "Get a temporal network as a list of edge lists, list of times and number of nodes N and return a list of contact durations, a list of group size histograms and a list of durations lists, one for each group size.",
            py::arg("edge_changes"),
            py::arg("ignore_size_histogram_differences") = false,
            py::arg("verbose") = false
         );

    m.def("sample_from_edge_lists", &sample_from_edge_lists, "Get a temporal network as a list of edge lists, given another instance of `edge_lists`, but resampled every dt. Alternatively, provide a number of time steps to divide (tmax-t0) into. if `sample_aggregates` is `True`, this does not sample the network state after each dt, but rather gives a graph of all edges being present in the last time step.",
            py::arg("edge_lists"),
            py::arg("dt") = 0.0,
            py::arg("N_time_steps") = 0,
            py::arg("sample_aggregates") = false,
            py::arg("verbose") = false
         );

    m.def("sample_from_edge_changes", &sample_from_edge_changes, "Get a temporal network as a list of edge lists, given an instance of `edge_changes`, but resampled every dt. Alternatively, provide a number of time steps to divide (tmax-t0) into. if `sample_aggregates` is `True`, this does not sample the network state after each dt, but rather gives a graph of all edges being present in the last time step.",
            py::arg("edge_changes"),
            py::arg("dt") = 0.0,
            py::arg("N_time_steps") = 0,
            py::arg("sample_aggregates") = false,
            py::arg("verbose") = false
         );

    m.def("bin_from_edge_lists", &bin_from_edge_lists, "Get a temporal network as a list of edge lists, given another instance of `edge_lists`, but binned for every dt. Alternatively, provide a number of time steps to divide (tmax-t0) into. This does not sample the network state after each dt, but rather gives a graph of all edges being present in the last time step.",
            py::arg("edge_lists"),
            py::arg("dt") = 0.0,
            py::arg("N_time_steps") = 0,
            py::arg("verbose") = false
         );

    m.def("bin_from_edge_changes", &bin_from_edge_changes, "Get a temporal network as a list of edge lists, given an instance of `edge_changes`, but binned every dt. Alternatively, provide a number of time steps to divide (tmax-t0) into. This does not sample the network state after each dt, but rather gives a graph of all edges being present in the last time step.",
            py::arg("edge_changes"),
            py::arg("dt") = 0.0,
            py::arg("N_time_steps") = 0,
            py::arg("verbose") = false
         );

    m.def("binned_social_trajectory_from_edge_changes", &binned_social_trajectory_from_edge_changes, "Get a social trajectory of a node, given an instance of `edge_changes`, binned every dt (list of group indices this node was part of in [t,t+dt) ).",
            py::arg("edge_changes"),
            py::arg("node"),
            py::arg("dt") = 0.0,
            py::arg("N_time_steps") = 0,
            py::arg("verbose") = false
         );

    m.def("social_trajectory_from_edge_changes", &social_trajectory_from_edge_changes, "Get a social trajectory of a node, given an instance of `edge_changes`.",
            py::arg("edge_changes"),
            py::arg("node"),
            py::arg("verbose") = false
         );

    m.def("social_trajectory_from_edge_lists", &social_trajectory_from_edge_lists, "Get a social trajectory of a node, given an instance of `edge_lists`.",
            py::arg("edge_lists"),
            py::arg("node"),
            py::arg("verbose") = false
         );

    m.def("binned_social_trajectory_from_edge_lists", &binned_social_trajectory_from_edge_lists, "Get a social trajectory of a node, given an instance of `edge_lists`, binned every dt (list of group indices this node was part of in [t,t+dt) ).",
            py::arg("edge_lists"),
            py::arg("node"),
            py::arg("dt") = 0.0,
            py::arg("N_time_steps") = 0,
            py::arg("verbose") = false
         );

    m.def("ZSBB_model", &ZSBB_model, "Simulate model after Zhao, Stehle, Bianconi, and Barrat.",
            py::arg("E"),
            py::arg("N"),
            py::arg("lambda"),
            py::arg("b0"),
            py::arg("b1"),
            py::arg("t_run_total"),
            py::arg("t_equilibration") = 0,
            py::arg("seed") = 0,
            py::arg("record_sizes_and_durations") = false,
            py::arg("return_after_equilibration_only") = false,
            py::arg("verbose") = false
         );

    m.def("dynamic_RGG", &dynamic_RGG, "Simulate dynamic random geometric graph model.",
            py::arg("N"),
            py::arg("t_run_total"),
            py::arg("step_distance") = 0.0,
            py::arg("mean_link_duration") = 0.0,
            py::arg("critical_density") = 0.65,
            py::arg("periodic_boundary_conditions_for_link_building") = false,
            py::arg("record_sizes_and_durations") = false,
            py::arg("seed") = 0,
            py::arg("verbose") = false
         );

    m.def("gillespie_SIS_on_edge_lists",&gillespie_on_edge_lists<Dyn_SIS>,"Perform a Gillespie SIS simulation on edge lists. Needs an instance of tacoma.edge_lists and an instance of tacoma.Dyn_SIS.",
            py::arg("edge_lists"),
            py::arg("Dyn_SIS"),
            py::arg("verbose") = false
            );

    m.def("gillespie_SIS_on_edge_changes",&gillespie_on_edge_changes<Dyn_SIS>,"Perform a Gillespie SIS simulation on edge changes. Needs an instance of tacoma.edge_changes and an instance of tacoma.Dyn_SIS.",
            py::arg("edge_lists"),
            py::arg("Dyn_SIS"),
            py::arg("verbose") = false
            );

    py::class_<edge_changes>(m,"edge_changes")
        .def(py::init<>())
        .def(py::init<const edge_changes_with_histograms &>(),
                py::arg("edge_changes_with_histograms")
            )
        .def("copy_from", &edge_changes::copy_from)
        .def_readwrite("t", &edge_changes::t)
        .def_readwrite("edges_out", &edge_changes::edges_out)
        .def_readwrite("edges_in", &edge_changes::edges_in)
        .def_readwrite("N", &edge_changes::N)
        .def_readwrite("t0", &edge_changes::t0)
        .def_readwrite("tmax", &edge_changes::tmax)
        .def_readwrite("edges_initial", &edge_changes::edges_initial);

    py::class_<edge_lists>(m,"edge_lists")
        .def(py::init<>())
        .def(py::init<const edge_lists_with_histograms &>(),
                py::arg("edge_lists_with_histograms")
            )
        .def("copy_from", &edge_lists::copy_from)
        .def_readwrite("t", &edge_lists::t)
        .def_readwrite("edges", &edge_lists::edges)
        .def_readwrite("N", &edge_lists::N)
        .def_readwrite("tmax", &edge_lists::tmax);

    py::class_<edge_lists_with_histograms>(m,"edge_lists_with_histograms")
        .def(py::init<>())
        .def_readwrite("t", &edge_lists_with_histograms::t)
        .def_readwrite("edges", &edge_lists_with_histograms::edges)
        .def_readwrite("size_histograms", &edge_lists_with_histograms::size_histograms)
        .def_readwrite("group_durations", &edge_lists_with_histograms::group_durations)
        .def_readwrite("N", &edge_lists_with_histograms::N)
        .def_readwrite("tmax", &edge_lists_with_histograms::tmax);

    py::class_<edge_changes_with_histograms>(m,"edge_changes_with_histograms")
        .def(py::init<>())
        .def_readwrite("t", &edge_changes_with_histograms::t)
        .def_readwrite("edges_in", &edge_changes_with_histograms::edges_in)
        .def_readwrite("edges_out", &edge_changes_with_histograms::edges_out)
        .def_readwrite("initial_size_histogram", &edge_changes_with_histograms::initial_size_histogram)
        .def_readwrite("group_changes", &edge_changes_with_histograms::group_changes)
        .def_readwrite("final_size_histogram", &edge_changes_with_histograms::final_size_histogram)
        .def_readwrite("contact_durations", &edge_changes_with_histograms::contact_durations)
        .def_readwrite("inter_contact_durations", &edge_changes_with_histograms::inter_contact_durations)
        .def_readwrite("group_durations", &edge_changes_with_histograms::group_durations)
        .def_readwrite("N", &edge_changes_with_histograms::N)
        .def_readwrite("t0", &edge_changes_with_histograms::t0)
        .def_readwrite("tmax", &edge_changes_with_histograms::tmax)
        .def_readwrite("edges_initial", &edge_changes_with_histograms::edges_initial);

    py::class_<group_sizes_and_durations>(m,"group_sizes_and_durations")
        .def(py::init<>())
        .def_readwrite("contact_durations", &group_sizes_and_durations::contact_durations)
        .def_readwrite("size_histograms", &group_sizes_and_durations::size_histograms)
        .def_readwrite("size_histogram_differences", &group_sizes_and_durations::size_histogram_differences)
        .def_readwrite("group_durations", &group_sizes_and_durations::group_durations)
        .def_readwrite("aggregated_size_histogram", &group_sizes_and_durations::aggregated_size_histogram)
        .def_readwrite("aggregated_network", &group_sizes_and_durations::aggregated_network);

    py::class_<edge_weight>(m,"edge_weight")
        .def(py::init<>())
        .def_readwrite("value", &edge_weight::value);

    py::class_<social_trajectory_entry>(m,"social_trajectory_entry")
        .def(py::init<>())
        .def_readwrite("hash", &social_trajectory_entry::hash)
        .def_readwrite("size", &social_trajectory_entry::size)
        .def_readwrite("time_pairs", &social_trajectory_entry::time_pairs);

    py::class_<dtu_week>(m,"dtu_week")
        .def(py::init<>())
        .def_readwrite("N", &dtu_week::N)
        .def_readwrite("tmax", &dtu_week::tmax)
        .def_readwrite("gamma", &dtu_week::gamma)
        .def_readwrite("P", &dtu_week::P);

    py::class_<Dyn_SIS>(m,"Dyn_SIS")
        .def(py::init<size_t,double,double,double,size_t,size_t,size_t,bool>(),
                py::arg("N"),
                py::arg("t_simulation"),
                py::arg("infection_rate"),
                py::arg("recovery_rate"),
                py::arg("number_of_initially_infected") = 1, 
                py::arg("number_of_initially_vaccinated") = 0, 
                py::arg("seed") = 0,
                py::arg("verbose") = false
            )
        .def_readwrite("time", &Dyn_SIS::time)
        .def_readwrite("R0", &Dyn_SIS::R0)
        .def_readwrite("SI", &Dyn_SIS::SI)
        .def_readwrite("I", &Dyn_SIS::I);

    return m.ptr();

}