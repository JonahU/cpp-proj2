import cppstruct

r = cppstruct.make_rocket_v1()
print(type(r))
print(r.name)
print(r.price)
# print(r.number_of_engines) # was previously immutable
print(r.max_speed)

r.name = "Rocket v2"
r.price = 444333000
# r.number_of_engines = 10 # number_of_engines of was immutable, couldn't do this before
r.max_speed = 200

cppstruct.launch_rocket_v1(r)

rockets = cppstruct.vector_to_python()
print(type(rockets))

rocket_list = list(rockets) # vector -> py list
print(type(rocket_list))
print(rocket_list)

rockets = cppstruct.vector_star_to_python() # ptr to vec
print(type(rockets))

rocket_list = list(rockets) # vector* -> py list (this seems potentially dangerous...)
print(type(rocket_list))
print(rocket_list)

rocket_map = cppstruct.map_to_python()

print(type(rocket_map))
print(len(rocket_map))
print("apollo10" in rocket_map)
print("apollo11" in rocket_map)
print(dir(rocket_map["apollo11"]))
print(rocket_map["apollo11"].name)
