#include "env.hpp"

string
jojo_repr (env_t &env, shared_ptr <jojo_t> jojo)
{
    assert (jojo->jo_vector.size () != 0);
    string repr = "";
    for (auto &jo: jojo->jo_vector) {
        repr += jo->repr (env);
        repr += " ";
    }
    repr.pop_back ();
    return repr;
}

void
jojo_print (env_t &env, shared_ptr <jojo_t> jojo)
{
    for (auto &jo: jojo->jo_vector) {
        cout << jo->repr (env)
             << " ";
    }
}

void
jojo_print_with_index (env_t &env,
                       shared_ptr <jojo_t> jojo,
                       size_t index)
{
    for (auto it = jojo->jo_vector.begin ();
         it != jojo->jo_vector.end ();
         it++) {
        size_t it_index = it - jojo->jo_vector.begin ();
        jo_t *jo = *it;
        if (index == it_index) {
            cout << "->> " << jo->repr (env) << " ";
        }
        else {
            cout << jo->repr (env) << " ";
        }
    }
}
