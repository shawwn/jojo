    #include <iostream>
    #include <vector>
    #include <map>
    #include <set>
    #include <stack>
    using namespace std;
    using name_t = string;
    struct obj_t;
    struct frame_t;

    struct env_t
    {
        map<name_t, obj_t> name_map;
        stack<obj_t> obj_ttack;
        stack<frame_t> frame_ttack;
        void step ();
        void eval ();
    };
    struct ins_t;

    using body_t = vector<ins_t>;
    struct frame_t
    {
       size_t index;
       body_t body;
       map<name_t, obj_t> local_map;
    };
    void env_t::step ()
    {

    }
    void env_t::eval ()
    {

    }
    struct obj_t
    {

    };
    struct clo_obj_t: public obj_t
    {
        map<name_t, obj_t> local_map;
        body_t body;
    };
    struct int_obj_t: public obj_t
    {
        int i;
    };
    struct str_obj_t: public obj_t
    {
        string s;
    };
    struct ins_t
    {

    };
    struct call_ins_t: public ins_t
    {
        name_t name;
    };
    struct end_ins_t: public ins_t
    {

    };
    struct get_ins_t: public ins_t
    {
        name_t name;
    };
    struct let_ins_t: public ins_t
    {
        name_t name;
    };
    struct clo_ins_t: public ins_t
    {
        body_t body;
    };
    int main ()
    {

    }
