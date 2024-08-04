import matplotlib.pyplot as plt
from tabulate import tabulate
from vrplib import read_solution

import logging as lg

from pyvrp import Model, read
from pyvrp.plotting import (
    plot_coordinates,
    plot_instance,
    plot_result,
    plot_route_schedule,
)
from pyvrp.stop import MaxIterations, MaxRuntime

""" 
(i)     Change the name of the test instance.
(ii)    Change the Runtime.
"""

if __name__ == '__main__':

    instance_names_original = [
        "Original/X-n186-k15.vrp",
        "Original/X-n110-k13.vrp",
        "Original/X-n153-k22.vrp",
        "Original/X-n298-k31.vrp",
        "Original/X-n129-k18.vrp",
        "Original/X-n143-k7.vrp",
        "Original/X-n367-k17.vrp",
        "Original/X-n106-k14.vrp",
        "Original/X-n162-k11.vrp",
        "Original/X-n214-k11.vrp"
    ]

    instance_names_unit_demand = [
        "X-n186-k15-C13_unit-demand.vrp",
        "X-n110-k13-C9_unit-demand.vrp",
        "X-n153-k22-C7_unit-demand.vrp",
        "X-n298-k31-C10_unit-demand.vrp",
        "X-n129-k18-C8_unit-demand.vrp",
        "X-n143-k7-C21_unit-demand.vrp",
        "X-n367-k17-C22_unit-demand.vrp",
        "X-n106-k14-C8_unit-demand.vrp",
        "X-n162-k11-C15_unit-demand.vrp",
        "X-n214-k11-C20_unit-demand.vrp"
    ]

    runtimes = [385, 227, 316, 618, 266, 295, 761, 218, 335, 443]
    for instance_name, runtime in zip(instance_names_original, runtimes):

        INSTANCE = read(f"BM_dev/BM_instances/{instance_name}", round_func="round")

        model = Model.from_data(INSTANCE)
        lg.basicConfig(filename='LOGGING...Original-Test-Instances', filemode='a', format='%(asctime)s,%(msecs)d %(message)s', 
                        datefmt='%H:%M:%S', level=lg.DEBUG)
        seeds = [42, 12, 37, 1000, 402, 68, 153, 24, 87, 2]
        # 
        lg.info(f"\n\n\n\n--------------------------INSTANCE: {instance_name} using runtime {runtime}------------------------------")

        for seed in seeds:
            print(f"\nSeed: {seed}")
            lg.info(f"\nSeed: {seed}")

            result = model.solve(stop=MaxRuntime(runtime), seed=seed, display=False)

            print(f"\nResult: {result}")
            lg.info(f"\nResult: {result}")