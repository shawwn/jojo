#include "obj.hpp"

obj_t::~obj_t ()
{
    // all classes that will be derived from
    // should have a virtual or protected destructor,
    // otherwise deleting an instance via a pointer
    // to a base class results in undefined behavior.
}

string
obj_t::repr (env_t &env)
{
    return "#<" + this->tag + ">";
}

bool
obj_t::equal (env_t &env, shared_ptr <obj_t> obj)
{
    if (this->tag != obj->tag)
        return false;
    else {
        cout << "- fatal error : obj_t::equal" << "\n"
             << "  equal is not implemented for  : "
             << obj->tag << "\n";
        exit (1);
    }
}

void
obj_t::apply (env_t &env, size_t arity)
{
    cout << "- fatal error : applying non applicable object" << "\n";
    exit (1);
}
