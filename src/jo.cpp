#include "jo.hpp"

string
name_vector_repr (name_vector_t &name_vector)
{
    if (name_vector.size () == 0) {
        string repr = "[";
        repr += "]";
        return repr;
    }
    else {
        string repr = "[";
        for (auto name: name_vector) {
            repr += name;
            repr += " ";
        }
        if (! repr.empty ()) repr.pop_back ();
        repr += "]";
        return repr;
    }
}
