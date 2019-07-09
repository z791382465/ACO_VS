# ACO_VS

## Overview

Ant colony optimization solves the travelling salesman problem.  
References Ant System: Optimization by a Colony of Cooperating Agents (Marco Dorigo, 1996).

## Usage

    >ACO_VS.exe <max_time> <ant_number> <q0> <beta> <rho> <data_set_file_name>
    
51.bat

    >ACO_VS.exe 10000 25 0.9 2.0 0.1 eil51.tsp
    
## Demo

    >ACO_VS.exe 10000 25 0.9 2.0 0.1 eil51.tsp
    Optimal path :
    14 44 32 38 9 29 33 20 28 19 34 35 2 27 30 25 7 21 0 31 10 1 15 49 8 48 37 4 36 16 3 17 46 11 45 50 26 5 47 22 6 42 23 13 24 12 40 39 18 41 43
    Optimal distance : 429.53
    34.453 sec.
    done.
    
Will generate an optimal path log file named `tour.txt` .
