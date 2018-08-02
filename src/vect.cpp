#include "vect.hpp"

vect_o::vect_o (env_t &env, obj_vector_t obj_vector)
{
    this->tag = tagging (env, "vect-t");
    this->obj_vector = obj_vector;
}

bool
vect_equal (env_t &env,
            obj_vector_t &lhs,
            obj_vector_t &rhs)
{
    if (lhs.size () != rhs.size ()) return false;
    auto size = lhs.size ();
    auto index = 0;
    while (index < size) {
        if (! obj_equal (env, lhs [index], rhs [index]))
            return false;
        index++;
    }
    return true;
}

bool
vect_o::equal (env_t &env, shared_ptr <obj_t> obj)
{
    if (this->tag != obj->tag) return false;
    auto that = static_pointer_cast <vect_o> (obj);
    return vect_equal (env, this->obj_vector, that->obj_vector);
}

string
vect_o::repr (env_t &env)
{
    string repr = "[";
    for (auto &obj: this->obj_vector) {
        repr += obj->repr (env);
        repr += " ";
    }
    if (! repr.empty ()) repr.pop_back ();
    repr += "]";
    return repr;
}

bool
vect_p (env_t &env, shared_ptr <obj_t> a)
{
    return a->tag == tagging (env, "vect-t");
}

shared_ptr <vect_o>
list_to_vect (env_t &env, shared_ptr <obj_t> l)
{
    auto obj_vector = obj_vector_t ();
    while (cons_p (env, l)) {
        obj_vector.push_back (car (env, l));
        l = cdr (env, l);
    }
    return make_shared <vect_o> (env, obj_vector);
}

sig_t jj_list_to_vect_sig = { "list-to-vect", "list" };
// -- (list-t t) -> (vect-t t)
void jj_list_to_vect (env_t &env, obj_map_t &obj_map)
{
    env.obj_stack.push (list_to_vect (env, obj_map ["list"]));
}

shared_ptr <obj_t>
vect_to_list (env_t &env, shared_ptr <vect_o> vect)
{
    auto obj_vector = vect->obj_vector;
    auto result = null_c (env);
    auto begin = obj_vector.rbegin ();
    auto end = obj_vector.rend ();
    for (auto it = begin; it != end; it++)
        result = cons_c (env, *it, result);
    return result;
}


sig_t jj_vect_to_list_sig = { "vect-to-list", "vect" };
// -- (vect-t t) -> (list-t t)
void jj_vect_to_list (env_t &env, obj_map_t &obj_map)
{
    auto obj = obj_map ["vect"];
    assert (vect_p (env, obj));
    auto vect = static_pointer_cast <vect_o> (obj);
    env.obj_stack.push (vect_to_list (env, vect));
}

void
import_vect (env_t &env)
{
    tagging (env, "vect-t");
    define_prim (env,
                 jj_list_to_vect_sig,
                 jj_list_to_vect);
    define_prim (env,
                 jj_vect_to_list_sig,
                 jj_vect_to_list);
}

void
test_vect ()
{
    auto env = env_t ();

    import_list (env);
    import_vect (env);

    define (env, "s1", make_shared <str_o> (env, "bye"));
    define (env, "s2", make_shared <str_o> (env, "world"));

    jo_vector_t jo_vector = {
        new ref_jo_t (boxing (env, "s1")),
        new ref_jo_t (boxing (env, "s2")),
        new ref_jo_t (boxing (env, "null-c")),
        new ref_jo_t (boxing (env, "cons-c")),
        new apply_jo_t (2),
        new ref_jo_t (boxing (env, "cons-c")),
        new apply_jo_t (2),
        new ref_jo_t (boxing (env, "list-to-vect")),
        new apply_jo_t (1),
        new ref_jo_t (boxing (env, "vect-to-list")),
        new apply_jo_t (1),
        new field_jo_t ("cdr"),
        new field_jo_t ("car"),
    };

    env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

    // env.double_report ();

    {
        env.run ();
        assert_stack_size (env, 1);
        assert_pop_eq (env, make_shared <str_o> (env, "world"));
        assert_stack_size (env, 0);
    }
}
