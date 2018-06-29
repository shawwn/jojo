#include <iostream>

using namespace std;

using name_t = string;

struct env_s
{
    map<name_t, obj_s> name_map;
    name_t obj_stack;
    name_t frame_stack;
    void step();
    void eval();
};

struct obj_s
{

};

struct clo_obj_s: public obj_s
{

};

struct int_obj_s: public obj_s
{

};

struct str_obj_s: public obj_s
{

};

struct ins_s
{

};

struct call_ins_s: public ins_s
{

};

struct end_ins_s: public ins_s
{

};

struct get_ins_s: public ins_s
{

};

struct let_ins_s: public ins_s
{

};

struct clo_ins_s: public ins_s
{

};

int main()
{

}
