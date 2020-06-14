import container

# std::vector<Rocket>
rockets = container.vector_example()
print(type(rockets))

rocket_list = list(rockets) # convert vector -> py list
print(type(rocket_list))
print(rocket_list)

# std::vector<Rocket>*
rockets = container.vector_star_example()
print(type(rockets))


# std::map<std::string, Rocket>
rocket_map = container.map_example()

print(type(rocket_map))
print(len(rocket_map))
print("apollo10" in rocket_map)
print("apollo11" in rocket_map)
print(dir(rocket_map["apollo11"]))
print(rocket_map["apollo11"].name)

# demonstrate python name refers to c++ obj
global_map = container.get_global_map()
container.print_global_map() # print c++ obj before any changes
global_map["apollo11"].name = "PYTHONv1"
global_map["apollo12"].name = "PYTHONv2" 
container.print_global_map() # print c++ obj after changes
