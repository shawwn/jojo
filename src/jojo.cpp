    #include <iostream>
    #include <vector>
    #include <map>
    #include <set>
    #include <stack>
    using namespace std;
    using name_t = string;
    struct obj_s;
    struct frame_s;

    struct env_s
    {
        map<name_t, obj_s> name_map;
        stack<obj_s> obj_stack;
        stack<frame_s> frame_stack;
        void step ();
        void eval ();
    };
    struct ins_s;

    using body_t = vector<ins_s>;
    struct frame_s
    {
       size_t index;
       body_t body;
       map<name_t, obj_s> local_map;
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
    int main ()
    {

    }
