from pyvrp import Model
from pyvrp.stop import NoImprovement

COORDS = [
    (456, 320),  # location 0 - depot 1
    (228, 0),  # location 1 - depot 2
    (912, 0),  # location 2
    (0, 80),  # location 3
    (114, 80),  # location 4
    (570, 160),  # location 5
    (798, 160),  # location 6
    (342, 240),  # location 7
    (684, 240),  # location 8
    (570, 400),  # location 9
    (912, 400),  # location 10
    (114, 480),  # location 11
    (228, 480),  # location 12
    (342, 560),  # location 13
    (684, 560),  # location 14
    (0, 640),  # location 15
    (798, 640),  # location 16
]

m = Model()


for k in range(3):
    depot_k = m.add_depot(x=COORDS[k][0], y=COORDS[k][1], name=f"{k}")

    m.add_vehicle_type(num_available=1, capacity=6, depot=depot_k, )

clients = [
    m.add_client(x=COORDS[idx][0], y=COORDS[idx][1], delivery=1)
    for idx in range(2, len(COORDS))
]

for frm_idx, frm in enumerate(m.locations):
    for to_idx, to in enumerate(m.locations):
        distance = abs(frm.x - to.x) + abs(frm.y - to.y)  # Manhattan
        m.add_edge(frm, to, distance=distance)

res = m.solve(
    stop=NoImprovement(20000), seed=42, display=True, collect_stats=True
)

print(res)
