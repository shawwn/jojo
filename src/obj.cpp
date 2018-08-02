#include "obj.hpp"

bool
tag_name_p (name_t name)
{
    auto size = name.size ();
    if (size < 3) return false;
    if (name [size - 1] != 't') return false;
    if (name [size - 2] != '-') return false;
    return true;
}

box_t *
boxing (env_t &env, name_t name);

void
define (env_t &env,
        name_t name,
        shared_ptr <obj_t> obj);

tag_t
tagging (env_t &env, name_t name)
{
    assert (tag_name_p (name));
    auto it = env.tag_map.find (name);
    if (it != env.tag_map.end ()) {
        tag_t tag = it->second;
        return tag;
    }
    else {
        auto tag = env.tag_name_box_vector.size ();
        env.tag_map [name] = tag;
        box_t *box = boxing (env, name);
        env.tag_name_box_vector.push_back (make_pair (name, box));
        auto type = make_shared <type_o>
            (env, tag, obj_map_t ());
        define (env, name, type);
        return tag;
    }
}

name_t
name_of_tag (env_t &env, tag_t tag)
{
    if (tag >= env.tag_name_box_vector.size ()) {
        return "#<unknown-tag-" + to_string (tag) + ">";
    }
    else {
        return env.tag_name_box_vector [tag] .first;
    }
}

box_t *
box_of_tag (env_t &env, tag_t tag)
{
    if (tag >= env.tag_name_box_vector.size ()) {
        cout << "- fatal error : box_of_tag" << "\n"
             << "  unknown tag : " << tag << "\n";
        exit (1);
    }
    else {
        return env.tag_name_box_vector [tag] .second;
    }
}
