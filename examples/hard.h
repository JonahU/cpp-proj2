#include <map>
#include <string>
#include <vector>

struct Teacher {
    std::string name;
};

struct Student {
    std::string name;
    std::map<int, double> grades;
};

struct Class {
    Teacher teacher;
    std::vector<Student> students;
    std::string classname;
};

inline std::vector<std::string> all_classes;

inline Class* start_semester(Teacher t, std::vector<Student> s, std::string c) {
    all_classes.push_back(c);
    return new Class { t, s, c };
}

inline void end_semester(Class* c) {
    for(auto iter = all_classes.begin(); iter != all_classes.end(); ++iter) {
        if (*iter == c->classname) {
            all_classes.erase(iter);
            break;
        }
    }
    delete c;
}

inline void grade_homework(Student s, int homework_id, double grade) {
    s.grades.insert({homework_id, grade});
}

std::map<Student, double> rank_students_by_grade();

std::map<std::string, Teacher> get_list_of_classes_and_teachers();

std::vector<std::string> get_list_of_teachers();

std::vector<std::string>& get_list_of_classes() {
    return all_classes;
}
