import aggregate

r = aggregate.make_rocket_v1()
print(f"Python received {r.name}. STATS: price = ${r.price}, # engines = {r.number_of_engines}, max speed = {r.max_speed}mph")

print("Python upgrading rocket to v2...")
r.name = "Rocket v2"
r.price = 444333000
r.number_of_engines = 10
r.max_speed = 200.0

aggregate.launch_rocket_v1(r, "florida", "June 11 2020")
