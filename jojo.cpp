    #include <iostream>
    #include <fstream>
    #include <sstream>

    #include <algorithm>
    #include <functional>

    #include <cassert>
    #include <memory>

    #include <vector>
    #include <array>
    #include <map>
    #include <set>
    #include <stack>

    #include <cmath>

    #include <filesystem>
    using namespace std;

    namespace fs = filesystem;
    struct obj_t;
    struct jo_t;
    struct env_t;
    struct box_t;
    struct jojo_t;
    struct frame_t;
    using name_t = string;
    using tag_t = size_t;
    using tag_vector_t = vector <tag_t>;
    using name_vector_t = vector <name_t>;
    using name_stack_t = stack <name_t>;
    using bind_t = pair <name_t, shared_ptr <obj_t>>;
    using bind_vector_t = vector <bind_t>; // index from end
    using local_scope_t = vector <bind_vector_t>; // index from end
    using jo_vector_t = vector <jo_t *>;
    using obj_map_t = map <name_t, shared_ptr <obj_t>>;
    using obj_vector_t = vector <shared_ptr <obj_t>>;
    using box_map_t = map <name_t, box_t *>;
    using obj_stack_t = stack <shared_ptr <obj_t>>;
    using frame_stack_t = stack <shared_ptr <frame_t>>;
    using jojo_map_t = map <tag_t, shared_ptr <jojo_t>>;
    using string_vector_t = vector <string> ;
    using local_ref_t = pair <size_t, size_t>;
    using local_ref_map_t = map <name_t, local_ref_t>;
    using path_t = fs::path;
    using num_t = double;
    using tagged_box_t = pair <name_t, box_t *>;
    using tagged_box_vector_t = vector <tagged_box_t>;
    using tag_map_t = map <name_t, tag_t>;
    using symbol = string;
    struct obj_t
    {
        tag_t tag;
        obj_map_t obj_map;
        virtual ~obj_t ();
        virtual string repr (env_t &env);
        virtual bool eq (env_t &env, shared_ptr <obj_t> obj);
        virtual void apply (env_t &env, size_t arity);
        virtual void apply_to_arg_dict (env_t &env);
    };
    struct jo_t
    {
        virtual jo_t * copy ();
        virtual ~jo_t ();
        virtual void exe (env_t &env, local_scope_t &local_scope);
        virtual string repr (env_t &env);
    };
    struct env_t
    {
        box_map_t box_map;
        obj_stack_t obj_stack;
        frame_stack_t frame_stack;
        tagged_box_vector_t tagged_box_vector;
        tag_map_t tag_map;
        path_t current_dir;
        path_t module_path;
        env_t ();
        void step ();
        void run ();
        void box_map_report ();
        void frame_stack_report ();
        void obj_stack_report ();
        void report ();
        void run_with_base (size_t base);
        void double_report ();
        void step_and_report ();
    };
    struct box_t
    {
        shared_ptr <obj_t> obj;
        bool empty_p;
        box_t ();
    };
    struct jojo_t
    {
        jo_vector_t jo_vector;
        jojo_t (jo_vector_t jo_vector);
        ~jojo_t ();
    };
    struct frame_t
    {
        size_t index;
        shared_ptr <jojo_t> jojo;
        local_scope_t local_scope;
        frame_t (shared_ptr <jojo_t> jojo, local_scope_t local_scope);
    };
      template <typename Out>
      void
      string_split (const string &s, char delim, Out result)
      {
          stringstream ss (s);
          string item;
          while (getline (ss, item, delim)) {
              *(result++) = item;
          }
      }

      vector <string>
      string_split (const string &s, char delim)
      {
          vector <string> elems;
          string_split (s, delim, back_inserter (elems));
          return elems;
      }
      string
      string_vector_join (string_vector_t string_vector, char c)
      {
          string str = "";
          for (auto s: string_vector) {
              str += s;
              str += c;
          }
          if (! str.empty ()) str.pop_back ();
          return str;
      }
    template <typename T>
    vector <T>
    vector_rest (vector <T> v)
    {
        auto size = v.size ();
        assert (size >= 1);
        vector <T> result = {};
        for (auto it = v.begin () + 1;
             it != v.end ();
             it++) {
            auto obj = *it;
            result.push_back (obj);
        }
        return result;
    }
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
      bool
      bind_eq (
          env_t &env,
          bind_t &lhs,
          bind_t &rhs)
      {
          if (lhs.first != rhs.first) return false;
          return lhs.second->eq (env, rhs.second);
      }
      string
      bind_vector_repr (
          env_t &env,
          bind_vector_t bind_vector)
      {
          string repr = "";
          for (auto it = bind_vector.rbegin ();
               it != bind_vector.rend ();
               it++) {
              repr += "(";
              repr += to_string (distance (bind_vector.rbegin (), it));
              repr += " ";
              repr += it->first;
              repr += " = ";
              auto obj = it->second;
              if (obj == nullptr)
                  repr += "_";
              else
                  repr += obj->repr (env);
              repr += ") ";
          }
          return repr;
      }
      size_t
      number_of_obj_in_bind_vector (bind_vector_t &bind_vector)
      {
          size_t sum = 0;
          auto begin = bind_vector.begin ();
          auto end = bind_vector.end ();
          for (auto it = begin; it != end; it++)
              if (it->second)
                  sum++;
          return sum;
      }
      void
      bind_vector_insert_obj (
          env_t &env,
          bind_vector_t &bind_vector,
          shared_ptr <obj_t> obj)
      {
          auto begin = bind_vector.rbegin ();
          auto end = bind_vector.rend ();
          for (auto it = begin; it != end; it++) {
              if (it->second == nullptr) {
                  it->second = obj;
                  return;
              }
          }
          cout << "- fatal error : bind_vector_insert_obj" << "\n";
          cout << "  the bind_vector is filled" << "\n";
          exit (1);
      }
      bind_vector_t
      bind_vector_merge_obj_vector (
          env_t &env,
          bind_vector_t &old_bind_vector,
          obj_vector_t &obj_vector)
      {
          auto bind_vector = old_bind_vector;
          for (auto obj: obj_vector)
              bind_vector_insert_obj (env, bind_vector, obj);
          return bind_vector;
      }
      void
      bind_vector_insert_obj_with_name (
          env_t &env,
          bind_vector_t &bind_vector,
          shared_ptr <obj_t> obj,
          name_t name)
      {
          auto begin = bind_vector.rbegin ();
          auto end = bind_vector.rend ();
          for (auto it = begin; it != end; it++) {
              if (it->first == name) {
                  it->second = obj;
                  return;
              }
          }
          cout << "- fatal error : bind_vector_insert_obj_with_name" << "\n";
          cout << "  name not found in the bind_vector" << "\n";
          cout << "  name : " << name << "\n";
          cout << "  bind_vector : "
               << bind_vector_repr (env, bind_vector) << "\n";
          exit (1);
      }
      bind_vector_t
      bind_vector_merge_obj_map (
          env_t &env,
          bind_vector_t &old_bind_vector,
          obj_map_t &obj_map)
      {
          auto bind_vector = old_bind_vector;
          for (auto &kv: obj_map) {
              auto name = kv.first;
              auto obj = kv.second;
              bind_vector_insert_obj_with_name (
                  env,
                  bind_vector,
                  obj,
                  name);
          }
          return bind_vector;
      }
      bind_vector_t
      bind_vector_from_name_vector (name_vector_t &name_vector)
      {
          auto bind_vector = bind_vector_t ();
          auto begin = name_vector.rbegin ();
          auto end = name_vector.rend ();
          for (auto it = begin; it != end; it++)
              bind_vector.push_back (make_pair (*it, nullptr));
          return bind_vector;
      }
      bool
      bind_vector_eq (
          env_t &env,
          bind_vector_t &lhs,
          bind_vector_t &rhs)
      {
          if (lhs.size () != rhs.size ()) return false;
          auto size = lhs.size ();
          size_t index = 0;
          while (index < size) {
              if (! bind_eq (env, lhs [index], rhs [index]))
                  return false;
              index++;
          }
          return true;
      }
      obj_vector_t
      pick_up_obj_vector (env_t &env, size_t counter)
      {
          auto obj_vector = obj_vector_t ();
          while (counter > 0) {
              counter--;
              auto obj = env.obj_stack.top ();
              obj_vector.push_back (obj);
              env.obj_stack.pop ();
          }
          reverse (obj_vector.begin (),
                   obj_vector.end ());
          return obj_vector;
      }
      bool
      local_scope_eq (
          env_t &env,
          local_scope_t &lhs,
          local_scope_t &rhs)
      {
          if (lhs.size () != rhs.size ()) return false;
          auto size = lhs.size ();
          size_t index = 0;
          while (index < size) {
              if (! bind_vector_eq (env, lhs [index], rhs [index]))
                  return false;
              index++;
          }
          return true;
      }
      local_scope_t
      local_scope_extend (
          local_scope_t old_local_scope,
          bind_vector_t bind_vector)
      {
          auto local_scope = old_local_scope;
          local_scope.push_back (bind_vector);
          return local_scope;
      }
      string
      local_scope_repr (env_t &env, local_scope_t local_scope)
      {
          string repr = "";
          repr += "  - [";
          repr += to_string (local_scope.size ());
          repr += "] ";
          repr += "local_scope - ";
          repr += "\n";
          for (auto it = local_scope.rbegin ();
               it != local_scope.rend ();
               it++) {
              repr += "    ";
              repr += to_string (distance (local_scope.rbegin (), it));
              repr += " ";
              repr += bind_vector_repr (env, *it);
              repr += "\n";
          }
          return repr;
      }
      bool
      obj_map_eq (env_t &env, obj_map_t &lhs, obj_map_t &rhs)
      {
          if (lhs.size () != rhs.size ()) return false;
          for (auto &kv: lhs) {
              auto name = kv.first;
              auto it = rhs.find (name);
              if (it == rhs.end ()) return false;
              if (! kv.second->eq (env, it->second)) return false;
          }
          return true;
      }
      string
      obj_map_repr (env_t &env, obj_map_t &obj_map)
      {
          string repr = "";
          for (auto &kv: obj_map) {
              auto name = kv.first;
              repr += name;
              repr += " = ";
              auto obj = kv.second;
              repr += obj->repr (env);
              repr += " ";
          }
          if (! repr.empty ()) repr.pop_back ();
          return repr;
      }
      name_vector_t
      name_vector_obj_map_lack (
          name_vector_t &old_name_vector,
          obj_map_t &obj_map)
      {
          auto name_vector = name_vector_t ();
          for (auto name: old_name_vector) {
              auto it = obj_map.find (name);
              // not found == lack
              if (it == obj_map.end ())
                  name_vector.push_back (name);
          }
          return name_vector;
      }
      name_vector_t
      name_vector_obj_map_arity_lack (
          name_vector_t &old_name_vector,
          obj_map_t &obj_map,
          size_t arity)
      {
          auto name_vector = name_vector_obj_map_lack
              (old_name_vector, obj_map);
          auto lack = name_vector.size ();
          auto counter = lack - arity;
          while (counter > 0) {
              counter--;
              name_vector.pop_back ();
          }
          return name_vector;
      }
      obj_map_t
      pick_up_obj_map_and_merge (
          env_t &env,
          name_vector_t &lack_name_vector,
          obj_map_t &old_obj_map)
      {
          auto obj_map = old_obj_map;
          auto begin = lack_name_vector.rbegin ();
          auto end = lack_name_vector.rend ();
          for (auto it = begin; it != end; it++) {
              name_t name = *it;
              auto obj = env.obj_stack.top ();
              env.obj_stack.pop ();
              obj_map [name] = obj;
          }
          return obj_map;
      }
      string
      name_vector_and_obj_map_repr (
          env_t &env,
          name_vector_t &name_vector,
          obj_map_t &obj_map)
      {
          string repr = "";
          for (auto &name: name_vector) {
              auto it = obj_map.find (name);
              if (it == obj_map.end ()) {
                  repr += name;
                  repr += " = _ ";
              }
          }
          for (auto &kv: obj_map) {
              auto name = kv.first;
              repr += name;
              repr += " = ";
              auto obj = kv.second;
              repr += obj->repr (env);
              repr += " ";
          }
          if (! repr.empty ()) repr.pop_back ();
          return repr;
      }
      name_t
      name_t2c (name_t type_name)
      {
          auto name = type_name;
          auto size = name.size ();
          assert (size > 2);
          assert (name [size - 1] == 't');
          assert (name [size - 2] == '-');
          name.pop_back ();
          name += 'c';
          return name;
      }
      name_t
      name_t2p (name_t type_name)
      {
          auto name = type_name;
          auto size = name.size ();
          assert (size > 2);
          assert (name [size - 1] == 't');
          assert (name [size - 2] == '-');
          name.pop_back ();
          name += 'p';
          return name;
      }
      name_t
      name_c2t (name_t data_name)
      {
          auto name = data_name;
          auto size = name.size ();
          assert (size > 2);
          assert (name [size - 1] == 'c');
          assert (name [size - 2] == '-');
          name.pop_back ();
          name += 't';
          return name;
      }
      name_t
      name_p2t (name_t pred_name)
      {
          auto name = pred_name;
          auto size = name.size ();
          assert (size > 2);
          assert (name [size - 1] == 'p');
          assert (name [size - 2] == '-');
          name.pop_back ();
          name += 't';
          return name;
      }
      shared_ptr <frame_t>
      new_frame_from_jojo (shared_ptr <jojo_t> jojo)
      {
          return make_shared <frame_t>
              (jojo, local_scope_t ());
      }
      shared_ptr <frame_t>
      new_frame_from_jo_vector (jo_vector_t jo_vector)
      {
          auto jojo = make_shared <jojo_t> (jo_vector);
          return make_shared <frame_t>
              (jojo, local_scope_t ());
      }
      local_ref_map_t
      local_ref_map_extend (
          env_t &env,
          local_ref_map_t &old_local_ref_map,
          name_vector_t &name_vector)
      {
          auto local_ref_map = local_ref_map_t ();
          for (auto &kv: old_local_ref_map) {
              auto name = kv.first;
              auto old_local_ref = kv.second;
              auto local_ref = local_ref_t ();
              local_ref.first = old_local_ref.first + 1;
              local_ref.second = old_local_ref.second;
              local_ref_map.insert (make_pair (name, local_ref));
          }
          size_t index = 0;
          auto size = name_vector.size ();
          while (index < size) {
              auto name = name_vector [index];
              auto local_ref = local_ref_t ();
              local_ref.first = 0;
              local_ref.second = index;
              local_ref_map.insert (make_pair (name, local_ref));
              index++;
          }
          return local_ref_map;
      }
      void
      assert_pop_eq (env_t &env, shared_ptr <obj_t> obj)
      {
          auto that = env.obj_stack.top ();
          assert (obj->eq (env, that));
          env.obj_stack.pop ();
      }
      void
      assert_tos_eq (env_t &env, shared_ptr <obj_t> obj)
      {
          auto that = env.obj_stack.top ();
          assert (obj->eq (env, that));
      }
      void
      assert_stack_size (env_t &env, size_t size)
      {
          assert (env.obj_stack.size () == size);
      }
      num_t s2n (string s)
      {
          return stod (s);
      }
    obj_t::~obj_t ()
    {
        // all classes that will be derived from
        // should have a virtual or protected destructor,
        // otherwise deleting an instance via a pointer
        // to a base class results in undefined behavior.
    }
    name_t
    name_of_tag (env_t &env, tag_t tag);

    string
    obj_t::repr (env_t &env)
    {
        return "#<" + name_of_tag (env, this->tag) + ">";
    }
    bool
    obj_t::eq (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag)
            return false;
        else {
            cout << "- fatal error : obj_t::eq" << "\n";
            cout << "  eq is not implemented for  : ";
            cout << obj->tag << "\n";
            exit (1);
        }
    }
    void
    obj_t::apply (env_t &env, size_t arity)
    {
        cout << "- fatal error : obj_t::apply" << "\n";
        cout << "  applying non applicable object" << "\n";
        cout << "  tag : " << name_of_tag (env, this->tag) << "\n";
        cout << "  obj : " << this->repr (env) << "\n";
        exit (1);
    }
    void
    obj_t::apply_to_arg_dict (env_t &env)
    {
        cout << "- fatal error : obj_t::apply_to_arg_dict" << "\n";
        cout << "  applying non applicable object" << "\n";
        cout << "  tag : " << name_of_tag (env, this->tag) << "\n";
        cout << "  obj : " << this->repr (env) << "\n";
        exit (1);
    }
    box_t *
    boxing (env_t &env, name_t name);

    void
    define (
        env_t &env,
        name_t name,
        shared_ptr <obj_t> obj)
    {
        auto box = boxing (env, name);
        box->obj = obj;
        box->empty_p = false;
    }
    bool
    obj_eq (
        env_t &env,
        shared_ptr <obj_t> &lhs,
        shared_ptr <obj_t> &rhs)
    {
        return lhs->eq (env, rhs);
    }
    shared_ptr <obj_t>
    find_obj_from_name (env_t &env, name_t name)
    {
        auto string_vector = string_split (name, '.');
        assert (string_vector.size () > 0);
        auto first_name = string_vector [0];
        auto it = env.box_map.find (first_name);
        if (it != env.box_map.end ()) {
            auto box = it->second;
            auto obj = box->obj;
            for (auto sub_name: vector_rest (string_vector)) {
                auto it = obj->obj_map.find (sub_name);
                if (it != obj->obj_map.end ()) {
                    obj = it->second;
                }
                else {
                    return nullptr;
                }
            }
            return obj;
        }
        else {
            return nullptr;
        }
    }
    jo_t *
    jo_t::copy ()
    {
        cout << "- fatal error : jo_t::copy unknown jo" << "\n";
        exit (1);
    }
    jo_t::~jo_t ()
    {
        // all classes that will be derived from
        // should have a virtual or protected destructor,
        // otherwise deleting an instance via a pointer
        // to a base class results in undefined behavior.
    }
    void
    jo_t::exe (env_t &env, local_scope_t &local_scope)
    {
        cout << "- fatal error : unknown jo" << "\n";
        exit (1);
    }
    string
    jo_t::repr (env_t &env)
    {
        return "#<unknown-jo>";
    }
      box_t::box_t ()
      {
          this->empty_p = true;
      }
      box_t *
      boxing (env_t &env, name_t name)
      {
          auto it = env.box_map.find (name);
          if (it != env.box_map.end ()) {
              auto box = it->second;
              return box;
          }
          else {
              auto box = new box_t ();
              env.box_map [name] = box;
              return box;
          }
      }
      name_t
      name_of_box (env_t &env, box_t *box)
      {
          for (auto &kv: env.box_map) {
              auto name = kv.first;
              if (kv.second == box) {
                  return name;
              }
          }
          return "#non-name";
      }
      jojo_t::
      jojo_t (jo_vector_t jo_vector)
      {
          this->jo_vector = jo_vector;
      }
      jojo_t::
      ~jojo_t ()
      {
          for (jo_t *jo_ptr: this->jo_vector)
              delete jo_ptr;
      }
      shared_ptr <jojo_t>
      jojo_append (
          shared_ptr <jojo_t> ante,
          shared_ptr <jojo_t> succ)
      {
          auto jo_vector = jo_vector_t ();
          for (auto x: ante->jo_vector) jo_vector.push_back (x->copy ());
          for (auto x: succ->jo_vector) jo_vector.push_back (x->copy ());
          return make_shared <jojo_t> (jo_vector);
      }
      string
      jojo_repr (
          env_t &env,
          shared_ptr <jojo_t> jojo)
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
      frame_t::
      frame_t (
          shared_ptr <jojo_t> jojo,
          local_scope_t local_scope)
      {
          this->index = 0;
          this->jojo = jojo;
          this->local_scope = local_scope;
      }
      void
      jojo_print (env_t &env, shared_ptr <jojo_t> jojo)
      {
          for (auto &jo: jojo->jo_vector) {
              cout << jo->repr (env) << " ";
          }
      }
      void
      jojo_print_with_index (
          env_t &env,
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
      void
      frame_report (env_t &env, shared_ptr <frame_t> frame)
      {
          cout << "  - [" << frame->index+1
               << "/" << frame->jojo->jo_vector.size ()
               << "] ";
          jojo_print_with_index (env, frame->jojo, frame->index);
          cout << "\n";
          cout << local_scope_repr (env, frame->local_scope);
      }
      bool
      tag_name_p (name_t name)
      {
          auto size = name.size ();
          if (size < 3) return false;
          if (name [size - 1] != 't') return false;
          if (name [size - 2] != '-') return false;
          return true;
      }
      tag_t
      tagging (env_t &env, name_t name)
      {
          assert (tag_name_p (name));
          auto it = env.tag_map.find (name);
          if (it != env.tag_map.end ()) {
              auto tag = it->second;
              return tag;
          }
          else {
              auto tag = env.tagged_box_vector.size ();
              env.tag_map [name] = tag;
              auto box = boxing (env, name);
              env.tagged_box_vector.push_back (make_pair (name, box));
              return tag;
          }
      }
      box_t *
      box_of_tag (env_t &env, tag_t tag)
      {
          if (tag >= env.tagged_box_vector.size ()) {
              cout << "- fatal error : box_of_tag" << "\n"
                   << "  unknown tag : " << tag << "\n";
              exit (1);
          }
          else {
              return env.tagged_box_vector [tag] .second;
          }
      }
      name_t
      name_of_tag (env_t &env, tag_t tag)
      {
          if (tag >= env.tagged_box_vector.size ()) {
              return "#<unknown-tag:" + to_string (tag) + ">";
          }
          else {
              return env.tagged_box_vector [tag] .first;
          }
      }
      tagged_box_t null_tagged_box = make_pair ("", nullptr);
      tagged_box_vector_t
      make_tagged_box_vector ()
      {
          auto tagged_box_vector = tagged_box_vector_t (64, null_tagged_box);
          return tagged_box_vector;
      }
      void
      def_type (env_t &env, name_t name);

      void
      preserve_tag (env_t &env, tag_t tag, name_t name)
      {
          env.tag_map [name] = tag;
          auto box = boxing (env, name);
          env.tagged_box_vector [tag] = make_pair (name, box);
          def_type (env, name);
      }
      tag_t closure_tag      = 0;
      tag_t type_tag         = 1;
      tag_t true_tag         = 2;
      tag_t false_tag        = 3;
      tag_t data_pred_tag    = 4;
      tag_t data_cons_tag    = 5;
      tag_t prim_tag         = 6;
      tag_t num_tag          = 7;
      tag_t str_tag          = 8;
      tag_t null_tag         = 9;
      tag_t cons_tag         = 10;
      tag_t vect_tag         = 12;
      tag_t dict_tag         = 13;
      tag_t module_tag       = 14;
      tag_t keyword_tag      = 15;
      tag_t macro_tag        = 16;
      tag_t top_keyword_tag  = 17;
      tag_t sym_tag          = 18;
      tag_t nothing_tag      = 19;
      tag_t just_tag         = 20;
      void
      init_tagged_box_vector (env_t &env)
      {
          preserve_tag (env, closure_tag      , "closure-t");
          preserve_tag (env, type_tag         , "type-t");
          preserve_tag (env, true_tag         , "true-t");
          preserve_tag (env, false_tag        , "false-t");
          preserve_tag (env, data_pred_tag    , "data-pred-t");
          preserve_tag (env, data_cons_tag    , "data-cons-t");
          preserve_tag (env, prim_tag         , "prim-t");
          preserve_tag (env, num_tag          , "num-t");
          preserve_tag (env, str_tag          , "str-t");
          preserve_tag (env, null_tag         , "null-t");
          preserve_tag (env, cons_tag         , "cons-t");
          preserve_tag (env, vect_tag         , "vect-t");
          preserve_tag (env, dict_tag         , "dict-t");
          preserve_tag (env, module_tag       , "module-t");
          preserve_tag (env, keyword_tag      , "keyword-t");
          preserve_tag (env, macro_tag        , "macro-t");
          preserve_tag (env, top_keyword_tag  , "top-keyword-t");
          preserve_tag (env, sym_tag          , "sym-t");
          preserve_tag (env, nothing_tag      , "nothing-t");
          preserve_tag (env, just_tag         , "just-t");
      }
    env_t::env_t ()
    {
       this->current_dir = fs::current_path ();
       this->tagged_box_vector = make_tagged_box_vector ();
       auto &env = *this;
       init_tagged_box_vector (env);
    }
    void
    env_t::step ()
    {
        auto frame = this->frame_stack.top ();
        size_t size = frame->jojo->jo_vector.size ();
        // it is assumed that jojo in frame are not empty
        assert (size != 0);
        size_t index = frame->index;
        frame->index++;
        // handle proper tail call
        if (index+1 == size) this->frame_stack.pop ();
        // since the last frame might be drop,
        //   we pass last local_scope as an extra argument.
        frame->jojo->jo_vector[index]->exe (*this, frame->local_scope);
    }
    void
    env_t::run ()
    {
        while (!this->frame_stack.empty ()) {
            this->step ();
        }
    }
    void
    env_t::box_map_report ()
    {
        auto &env = *this;
        cout << "- [" << env.box_map.size () << "] "
             << "box_map - " << "\n";
        for (auto &kv: env.box_map) {
            cout << "  " << kv.first << " = ";
            auto box = kv.second;
            if (box->empty_p)
                cout << "_";
            else
                cout << box->obj->repr (env);
            cout << "\n";
        }
    }
    void
    env_t::frame_stack_report ()
    {
        auto &env = *this;
        cout << "- [" << env.frame_stack.size () << "] "
             << "frame_stack - " << "\n";
        frame_stack_t frame_stack = env.frame_stack;
        while (! frame_stack.empty ()) {
           auto frame = frame_stack.top ();
           frame_report (env, frame);
           frame_stack.pop ();
        }
    }
    void
    env_t::obj_stack_report ()
    {
        auto &env = *this;
        cout << "- [" << env.obj_stack.size () << "] "
             << "obj_stack - " << "\n";
        auto obj_stack = env.obj_stack;
        while (! obj_stack.empty ()) {
            auto obj = obj_stack.top ();
            cout << "  ";
            cout << obj->repr (env);
            cout << "\n";
            obj_stack.pop ();
        }
    }
    void
    env_t::report ()
    {
        this->box_map_report ();
        this->frame_stack_report ();
        this->obj_stack_report ();
    }
    void
    env_t::run_with_base (size_t base)
    {
        while (this->frame_stack.size () > base) {
            this->step ();
        }
    }
    void
    env_t::double_report ()
    {
        this->report ();
        this->run ();
        this->report ();
    }
    void
    env_t::step_and_report ()
    {
        this->step ();
        this->report ();
    }
    struct closure_o: obj_t
    {
        name_vector_t name_vector;
        shared_ptr <jojo_t> jojo;
        bind_vector_t bind_vector;
        local_scope_t local_scope;
        closure_o (env_t &env,
                   name_vector_t name_vector,
                   shared_ptr <jojo_t> jojo,
                   bind_vector_t bind_vector,
                   local_scope_t local_scope);
        bool eq (env_t &env, shared_ptr <obj_t> obj);
        void apply (env_t &env, size_t arity);
        void apply_to_arg_dict (env_t &env);
        string repr (env_t &env);
    };
    closure_o::
    closure_o (
        env_t &env,
        name_vector_t name_vector,
        shared_ptr <jojo_t> jojo,
        bind_vector_t bind_vector,
        local_scope_t local_scope)
    {
        this->tag = closure_tag;
        this->name_vector = name_vector;
        this->jojo = jojo;
        this->bind_vector = bind_vector;
        this->local_scope = local_scope;
    }
    shared_ptr <closure_o>
    make_closure (
        env_t &env,
        name_vector_t name_vector,
        shared_ptr <jojo_t> jojo,
        bind_vector_t bind_vector,
        local_scope_t local_scope)
    {
        return make_shared <closure_o> (
            env,
            name_vector,
            jojo,
            bind_vector,
            local_scope);
    }
    void
    closure_o::apply (env_t &env, size_t arity)
    {
        auto size = this->name_vector.size ();
        auto have = number_of_obj_in_bind_vector (this->bind_vector);
        auto lack = size - have;
        if (arity > lack) {
            cout << "- fatal error : closure_o::apply" << "\n";
            cout << "  over-arity apply" << "\n";
            cout << "  arity > lack" << "\n";
            cout << "  arity : " << arity << "\n";
            cout << "  lack : " << lack << "\n";
            exit (1);
        }
        auto obj_vector = pick_up_obj_vector (env, arity);
        auto bind_vector = bind_vector_merge_obj_vector (
            env, this->bind_vector, obj_vector);
        if (lack == arity) {
            auto local_scope = local_scope_extend (
                this->local_scope, bind_vector);
            auto frame = make_shared <frame_t> (
                this->jojo, local_scope);
            env.frame_stack.push (frame);
        }
        else {
            assert ((arity < lack));
            auto closure = make_closure (
                env,
                this->name_vector,
                this->jojo,
                bind_vector,
                this->local_scope);
            env.obj_stack.push (closure);
        }
    }
    bool
    closure_o::eq (env_t &env, shared_ptr <obj_t> obj)
    {
        // raw pointers must be eq first
        if (this != obj.get ()) return false;
        auto that = static_pointer_cast <closure_o> (obj);
        // then scopes
        if (local_scope_eq
            (env,
             this->local_scope,
             that->local_scope)) return false;
        // then bindings
        if (bind_vector_eq
            (env,
             this->bind_vector,
             that->bind_vector)) return false;
        else return true;
    }
    bool
    closure_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == closure_tag;
    }
    string
    closure_o::repr (env_t &env)
    {
        string repr = "(closure ";
        repr += name_vector_repr (this->name_vector);
        repr += " ";
        repr += jojo_repr (env, this->jojo);
        repr += "\n";
        auto local_scope = this->local_scope;
        local_scope.push_back (this->bind_vector);
        repr += local_scope_repr (env, local_scope);
        if (! repr.empty ()) repr.pop_back ();
        if (! repr.empty ()) repr.pop_back ();
        repr += ")";
        return repr;
    }
    struct type_o: obj_t
    {
        tag_t tag_of_type;
        tag_vector_t super_tag_vector;
        type_o (
            env_t &env,
            tag_t tag_of_type,
            tag_vector_t super_tag_vector,
            obj_map_t obj_map);
        bool eq (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    type_o::type_o (
        env_t &env,
        tag_t tag_of_type,
        tag_vector_t super_tag_vector,
        obj_map_t obj_map)
    {
        this->tag = type_tag;
        this->tag_of_type = tag_of_type;
        this->super_tag_vector = super_tag_vector;
        this->obj_map = obj_map;
    }
    shared_ptr <type_o>
    make_type (
        env_t &env,
        tag_t tag_of_type,
        tag_vector_t super_tag_vector,
        obj_map_t obj_map)
    {
        return make_shared <type_o> (
            env,
            tag_of_type,
            super_tag_vector,
            obj_map);
    }
    shared_ptr <type_o>
    as_type (shared_ptr <obj_t> obj)
    {
        return static_pointer_cast <type_o> (obj);
    }
    string
    type_o::repr (env_t &env)
    {
        return name_of_tag (env, this->tag_of_type);
    }
    bool
    type_o::eq (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = as_type (obj);
        if (this->tag_of_type != that->tag_of_type) return false;
        return true;
    }
    shared_ptr <type_o>
    find_type_from_prefix (env_t &env, name_t prefix)
    {
        auto string_vector = string_split (prefix, '.');
        assert (string_vector.size () > 0);
        auto top = string_vector [0];
        auto it = env.box_map.find (top + "-t");
        if (it != env.box_map.end ()) {
            auto box = it->second;
            if (box->empty_p) return nullptr;
            auto obj = box->obj;
            if (obj->tag != type_tag) return nullptr;
            auto type = as_type (obj);
            auto begin = string_vector.begin () + 1;
            auto end = string_vector.end ();
            for (auto it = begin; it != end; it++) {
                auto field = *it;
                field += "-t";
                auto obj = type->obj_map [field];
                if (obj->tag != type_tag) return nullptr;
                type = as_type (obj);
            }
            return type;
        }
        return nullptr;
    }
    void
    assign (
        env_t &env,
        name_t prefix,
        name_t name,
        shared_ptr <obj_t> obj)
    {
        if (prefix == "") {
            define (env, name, obj);
            return;
        }
        auto type = find_type_from_prefix (env, prefix);
        if (type) {
            type->obj_map [name] = obj;
        }
        else {
            cout << "- fatal error : assign fail" << "\n";
            cout << "  unknown prefix : " << prefix << "\n";
            exit (1);
        }
    }
    void
    assign_type (
        env_t &env,
        name_t prefix,
        name_t type_name,
        tag_t tag_of_type,
        tag_vector_t super_tag_vector)
    {
        auto type = make_type (
            env,
            tag_of_type,
            super_tag_vector,
            obj_map_t ());
        auto box = box_of_tag (env, tag_of_type);
        box->obj = type;
        box->empty_p = false;
        assign (env, prefix, type_name, type);
    }
    void
    define_type (env_t &env, name_t name)
    {
        auto type_name = name;
        auto tag_of_type = tagging (env, name);
        assign_type (env, "", type_name, tag_of_type, {});
    }
    shared_ptr <type_o>
    type_of (env_t &env, shared_ptr <obj_t> obj)
    {
        auto box = box_of_tag (env, obj->tag);
        assert (! box->empty_p);
        return as_type (box->obj);
    }
    struct data_o: obj_t, enable_shared_from_this <obj_t>
    {
        data_o (
            env_t &env,
            tag_t tag,
            obj_map_t obj_map);
        bool eq (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    data_o::
    data_o (
        env_t &env,
        tag_t tag,
        obj_map_t obj_map)
    {
        this->tag = tag;
        this->obj_map = obj_map;
    }
    bool
    data_o::eq (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = static_pointer_cast <data_o> (obj);
        return obj_map_eq (env, this->obj_map, that->obj_map);

    }
    string
    sexp_repr (env_t &env, shared_ptr <obj_t> a);

    string
    data_o::repr (env_t &env)
    {
        if (this->tag == null_tag or
            this->tag == cons_tag)
        {
            return sexp_repr (env, shared_from_this ());
        }
        else if (this->obj_map.size () == 0) {
            string repr = "";
            repr += name_of_tag (env, this->tag);
            repr.pop_back ();
            repr.pop_back ();
            repr += "-c";
            return repr;
        }
        else {
            string repr = "(";
            repr += name_of_tag (env, this->tag);
            repr.pop_back ();
            repr.pop_back ();
            repr += "-c ";
            repr += obj_map_repr (env, this->obj_map);
            repr += ")";
            return repr;
        }
    }
    void
    assign_data (
        env_t &env,
        name_t prefix,
        name_t data_name,
        tag_t tag_of_type)
    {
        auto data = make_shared <data_o>
            (env, tag_of_type, obj_map_t ());
        assign (env, prefix, data_name, data);
    }
    shared_ptr <obj_t>
    true_c (env_t &env)
    {
       return make_shared <data_o>
           (env, true_tag, obj_map_t ());
    }
    bool
    true_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == true_tag;
    }
    shared_ptr <obj_t>
    false_c (env_t &env)
    {
       return make_shared <data_o>
           (env, false_tag, obj_map_t ());
    }
    bool
    false_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == false_tag;
    }
    shared_ptr <obj_t>
    make_bool (env_t &env, bool b)
    {
        if (b) {
            return true_c (env);
        }
        else {
            return false_c (env);
        }
    }
    bool
    bool_p (env_t &env, shared_ptr <obj_t> a)
    {
        return true_p (env, a)
            or false_p (env, a);
    }
    struct data_pred_o: obj_t
    {
        tag_t tag_of_type;
        data_pred_o (
            env_t &env,
            tag_t tag_of_type);
        void apply (env_t &env, size_t arity);
        bool eq (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    data_pred_o::
    data_pred_o (
        env_t &env,
        tag_t tag_of_type)
    {
        this->tag = data_pred_tag;
        this->tag_of_type = tag_of_type;
    }
    void
    data_pred_o::apply (env_t &env, size_t arity)
    {
        if (arity == 1) {
            auto obj = env.obj_stack.top ();
            env.obj_stack.pop ();
            env.obj_stack.push (
                make_bool (
                    env,
                    obj->tag == this->tag_of_type));
        }
        else {
            cout << "- fatal error : data_pred_o::apply" << "\n";
            cout << "  arity of this kind of apply must be 1" << "\n";
            cout << "  arity : " << arity << "\n";
            exit (1);
        }
    }
    bool
    data_pred_o::eq (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = static_pointer_cast <data_pred_o> (obj);
        if (this->tag_of_type != that->tag_of_type) return false;
        return true;
    }
    string
    data_pred_o::repr (env_t &env)
    {
        string repr = "";
        repr += name_of_tag (env, this->tag_of_type);
        repr.pop_back ();
        repr.pop_back ();
        repr += "-p";
        return repr;
    }
    void
    assign_data_pred (
        env_t &env,
        name_t prefix,
        name_t pred_name,
        tag_t tag_of_type)
    {
        auto data_pred = make_shared <data_pred_o>
            (env, tag_of_type);
        assign (env, prefix, pred_name, data_pred);
    }
    void
    define_data_pred (
        env_t &env,
        name_t pred_name,
        tag_t tag_of_type)
    {
        assign_data_pred (env, "", pred_name, tag_of_type);
    }
    struct data_cons_o: obj_t
    {
        tag_t tag_of_type;
        name_vector_t name_vector;
        data_cons_o (
            env_t &env,
            tag_t tag_of_type,
            name_vector_t name_vector,
            obj_map_t obj_map);
        void apply (env_t &env, size_t arity);
        bool eq (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    data_cons_o::
    data_cons_o (
        env_t &env,
        tag_t tag_of_type,
        name_vector_t name_vector,
        obj_map_t obj_map)
    {
        this->tag = data_cons_tag;
        this->tag_of_type = tag_of_type;
        this->name_vector = name_vector;
        this->obj_map = obj_map;
    }
    void
    data_cons_o::apply (env_t &env, size_t arity)
    {
        auto size = this->name_vector.size ();
        auto have = this->obj_map.size ();
        auto lack = size - have;
        if (lack == arity) {
            auto lack_name_vector = name_vector_obj_map_lack
                (this->name_vector, this->obj_map);
            auto obj_map = pick_up_obj_map_and_merge
                (env, lack_name_vector, this->obj_map);
            auto data = make_shared <data_o>
                (env, this->tag_of_type, obj_map);
            env.obj_stack.push (data);
        }
        else if (arity < lack) {
            auto lack_name_vector = name_vector_obj_map_arity_lack
                (this->name_vector, this->obj_map, arity);
            auto obj_map = pick_up_obj_map_and_merge
                (env, lack_name_vector, this->obj_map);
            auto data_cons = make_shared <data_cons_o>
                (env, this->tag_of_type, this->name_vector, obj_map);
            env.obj_stack.push (data_cons);
        }
        else {
            cout << "- fatal error : data_cons_o::apply" << "\n";
            cout << "  over-arity apply" << "\n";
            cout << "  arity > lack" << "\n";
            cout << "  arity : " << arity << "\n";
            cout << "  lack : " << lack << "\n";
            exit (1);
        }
    }
    bool
    data_cons_o::eq (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = static_pointer_cast <data_cons_o> (obj);
        if (this->tag_of_type != that->tag_of_type) return false;
        return obj_map_eq (env, this->obj_map, that->obj_map);
    }
    bool
    data_cons_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == data_cons_tag;
    }
    string
    data_cons_o::repr (env_t &env)
    {
        if (this->name_vector.size () == 0) {
            string repr = "";
            repr += name_of_tag (env, this->tag_of_type);
            repr.pop_back ();
            repr.pop_back ();
            repr += "-c";
            return repr;
        }
        else {
            string repr = "(";
            repr += name_of_tag (env, this->tag_of_type);
            repr.pop_back ();
            repr.pop_back ();
            repr += "-c ";
            repr += name_vector_and_obj_map_repr
                (env, this->name_vector, this->obj_map);
            repr += ")";
            return repr;
        }
    }
    void
    assign_data_cons (
        env_t &env,
        name_t prefix,
        name_t data_name,
        tag_t tag_of_type,
        name_vector_t name_vector)
    {
        auto data_cons = make_shared <data_cons_o>
            (env, tag_of_type, name_vector, obj_map_t ());
        assign (env, prefix, data_name, data_cons);
    }
    using prim_fn = function
        <void (env_t &, obj_map_t &)>;
    struct prim_o: obj_t
    {
        name_vector_t name_vector;
        prim_fn fn;
        prim_o (
            env_t &env,
            name_vector_t name_vector,
            prim_fn fn,
            obj_map_t obj_map);
        bool eq (env_t &env, shared_ptr <obj_t> obj);
        void apply (env_t &env, size_t arity);
        string repr (env_t &env);
    };
    prim_o::prim_o (
        env_t &env,
        name_vector_t name_vector,
        prim_fn fn,
        obj_map_t obj_map)
    {
        this->tag = prim_tag;
        this->name_vector = name_vector;
        this->fn = fn;
        this->obj_map = obj_map;
    }
    bool
    prim_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == prim_tag;
    }
    string
    prim_o::repr (env_t &env)
    {
        if (this->name_vector.size () == 0) {
            string repr = "(prim)";
            return repr;
        }
        else {
            string repr = "(prim ";
            repr += name_vector_and_obj_map_repr
                (env, this->name_vector, this->obj_map);
            repr += ")";
            return repr;
        }
    }
    bool prim_o::eq (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = static_pointer_cast <prim_o> (obj);
        if (this != obj.get ()) return false;
        return obj_map_eq (env, this->obj_map, that->obj_map);
    }
    void prim_o::apply (env_t &env, size_t arity)
    {
        auto size = this->name_vector.size ();
        auto have = this->obj_map.size ();
        auto lack = size - have;
        if (lack == arity) {
            auto lack_name_vector = name_vector_obj_map_lack
                (this->name_vector, this->obj_map);
            auto obj_map = pick_up_obj_map_and_merge
                (env, lack_name_vector, this->obj_map);
            this->fn (env, obj_map);
        }
        else if (arity < lack) {
            auto lack_name_vector = name_vector_obj_map_arity_lack
                (this->name_vector, this->obj_map, arity);
            auto obj_map = pick_up_obj_map_and_merge
                (env, lack_name_vector, this->obj_map);
            auto prim = make_shared <prim_o>
                (env, this->name_vector, this->fn, obj_map);
            env.obj_stack.push (prim);
        }
        else {
            cout << "- fatal error : prim_o::apply" << "\n";
            cout << "  over-arity apply" << "\n";
            cout << "  arity > lack" << "\n";
            cout << "  arity : " << arity << "\n";
            cout << "  lack : " << lack << "\n";
            exit (1);
        }
    }
    using sig_t = name_vector_t;
    name_t
    name_of_sig (sig_t &sig)
    {
        return sig [0];
    }
    name_vector_t
    name_vector_of_sig (sig_t &sig)
    {
        auto name_vector = name_vector_t ();
        auto begin = sig.begin () + 1;
        auto end = sig.end ();
        for (auto it = begin; it != end; it++) {
            name_vector.push_back (*it);
        }
        return name_vector;
    }
    void
    define_prim (env_t &env, sig_t sig, prim_fn fn)
    {
        auto name = name_of_sig (sig);
        auto name_vector = name_vector_of_sig (sig);
        auto prim = make_shared <prim_o> (
            env, name_vector, fn, obj_map_t ());
        define (env, name, prim);
    }
    struct num_o: obj_t
    {
        num_t num;
        num_o (env_t &env, num_t num);
        bool eq (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    num_o::num_o (env_t &env, num_t num)
    {
        this->tag = num_tag;
        this->num = num;
    }
    shared_ptr <num_o>
    make_num (env_t &env, num_t num)
    {
        return make_shared <num_o> (env, num);
    }
    string
    num_o::repr (env_t &env)
    {
        if (this->num == floor (this->num)) {
            return to_string (static_cast <long long int> (this->num));
        }
        else {
            return to_string (this->num);
        }
    }
    shared_ptr <num_o>
    as_num (shared_ptr <obj_t> obj)
    {
        assert (obj->tag == num_tag);
        return static_pointer_cast <num_o> (obj);
    }
    bool
    num_o::eq (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = as_num (obj);
        return (this->num == that->num);
    }
    bool
    num_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == num_tag;
    }
    struct str_o: obj_t
    {
        string str;
        str_o (env_t &env, string str);
        bool eq (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    str_o::str_o (env_t &env, string str)
    {
        this->tag = str_tag;
        this->str = str;
    }
    shared_ptr <str_o>
    make_str (env_t &env, string str)
    {
        return make_shared <str_o> (env, str);
    }
    string
    str_o::repr (env_t &env)
    {
        return "\"" + this->str + "\"";
    }
    shared_ptr <str_o>
    as_str (shared_ptr <obj_t> obj)
    {
        assert (obj->tag == str_tag);
        return static_pointer_cast <str_o> (obj);
    }
    bool
    str_o::eq (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = as_str (obj);
        return (this->str == that->str);
    }
    bool
    str_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == str_tag;
    }
    shared_ptr <num_o>
    str_length (
        env_t &env,
        shared_ptr <str_o> str)
    {
        auto size = str->str.size ();
        return make_num (env, static_cast <num_t> (size));
    }
    shared_ptr <str_o>
    str_append (
        env_t &env,
        shared_ptr <str_o> ante,
        shared_ptr <str_o> succ)
    {
        return make_str (env, ante->str + succ->str);
    }
    shared_ptr <str_o>
    str_slice (
        env_t &env,
        shared_ptr <str_o> str,
        shared_ptr <num_o> begin,
        shared_ptr <num_o> end)
    {
        auto size = str->str.size ();
        assert (begin->num >= 0);
        assert (end->num < size);
        auto length = end->num - begin->num;
        return make_str (env, str->str.substr (begin->num, length));
    }
    shared_ptr <str_o>
    str_ref (
        env_t &env,
        shared_ptr <str_o> str,
        shared_ptr <num_o> index)
    {
        auto size = str->str.size ();
        assert (index->num >= 0);
        assert (index->num < size);
        auto c = str->str [index->num];
        auto s = string ();
        s += c;
        return make_str (env, s);
    }
    shared_ptr <str_o>
    str_head (
        env_t &env,
        shared_ptr <str_o> str)
    {
        auto size = str->str.size ();
        assert (size >= 1);
        auto c = str->str [0];
        auto s = string ();
        s += c;
        return make_str (env, s);
    }
    shared_ptr <str_o>
    str_rest (
        env_t &env,
        shared_ptr <str_o> str)
    {
        auto size = str->str.size ();
        return make_str (env, str->str.substr (1, size -1));
    }
    struct sym_o: obj_t
    {
        symbol sym;
        sym_o (env_t &env, symbol sym);
        bool eq (env_t &env, shared_ptr <obj_t> obj);
        symbol repr (env_t &env);
    };
    sym_o::sym_o (env_t &env, symbol sym)
    {
        this->tag = sym_tag;
        this->sym = sym;
    }
    shared_ptr <sym_o>
    make_sym (env_t &env, symbol sym)
    {
        return make_shared <sym_o> (env, sym);
    }
    symbol
    sym_o::repr (env_t &env)
    {
        return "'" + this->sym;
    }
    shared_ptr <sym_o>
    as_sym (shared_ptr <obj_t> obj)
    {
        assert (obj->tag == sym_tag);
        return static_pointer_cast <sym_o> (obj);
    }
    bool
    sym_o::eq (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = as_sym (obj);
        return (this->sym == that->sym);
    }
    bool
    sym_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == sym_tag;
    }
    shared_ptr <num_o>
    sym_length (
        env_t &env,
        shared_ptr <sym_o> sym)
    {
        auto size = sym->sym.size ();
        return make_num (env, static_cast <num_t> (size));
    }
    shared_ptr <sym_o>
    sym_append (
        env_t &env,
        shared_ptr <sym_o> ante,
        shared_ptr <sym_o> succ)
    {
        return make_sym (env, ante->sym + succ->sym);
    }
    shared_ptr <sym_o>
    sym_slice (
        env_t &env,
        shared_ptr <sym_o> sym,
        shared_ptr <num_o> begin,
        shared_ptr <num_o> end)
    {
        auto size = sym->sym.size ();
        assert (begin->num >= 0);
        assert (end->num < size);
        auto length = end->num - begin->num;
        return make_sym (env, sym->sym.substr (begin->num, length));
    }
    shared_ptr <sym_o>
    sym_ref (
        env_t &env,
        shared_ptr <sym_o> sym,
        shared_ptr <num_o> index)
    {
        auto size = sym->sym.size ();
        assert (index->num >= 0);
        assert (index->num < size);
        auto c = sym->sym [index->num];
        auto s = symbol ();
        s += c;
        return make_sym (env, s);
    }
    shared_ptr <sym_o>
    sym_head (
        env_t &env,
        shared_ptr <sym_o> sym)
    {
        auto size = sym->sym.size ();
        assert (size >= 1);
        auto c = sym->sym [0];
        auto s = symbol ();
        s += c;
        return make_sym (env, s);
    }
    shared_ptr <sym_o>
    sym_rest (
        env_t &env,
        shared_ptr <sym_o> sym)
    {
        auto size = sym->sym.size ();
        return make_sym (env, sym->sym.substr (1, size -1));
    }
    shared_ptr <obj_t>
    null_c (env_t &env)
    {
       return make_shared <data_o>
           (env, null_tag, obj_map_t ());
    }
    bool
    null_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == null_tag;
    }
    shared_ptr <obj_t>
    cons_c (
        env_t &env,
        shared_ptr <obj_t> car,
        shared_ptr <obj_t> cdr)
    {
        auto obj_map = obj_map_t ();
        obj_map ["car"] = car;
        obj_map ["cdr"] = cdr;
        return make_shared <data_o>
            (env, cons_tag, obj_map);
    }
    bool
    cons_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == cons_tag;
    }
    shared_ptr <obj_t>
    car (env_t &env, shared_ptr <obj_t> cons)
    {
        assert (cons_p (env, cons));
        return cons->obj_map ["car"];
    }
    shared_ptr <obj_t>
    cdr (env_t &env, shared_ptr <obj_t> cons)
    {
        assert (cons_p (env, cons));
        return cons->obj_map ["cdr"];
    }
    bool
    list_p (env_t &env, shared_ptr <obj_t> a)
    {
        return null_p (env, a)
            or cons_p (env, a);
    }
    size_t
    list_size (env_t &env, shared_ptr <obj_t> l)
    {
        assert (list_p (env, l));
        auto size = 0;
        while (! null_p (env, l)) {
            size++;
            l = cdr (env, l);
        }
        return size;
    }
    shared_ptr <num_o>
    list_length (env_t &env, shared_ptr <obj_t> l)
    {
        auto size = list_size (env, l);
        auto length = static_cast <num_t> (size);
        return make_num (env, length);
    }
    shared_ptr <obj_t>
    list_reverse (env_t &env, shared_ptr <obj_t> l)
    {
        assert (list_p (env, l));
        auto result = null_c (env);
        while (! null_p (env, l)) {
            auto obj = car (env, l);
            result = cons_c (env, obj, result);
            l = cdr (env, l);
        }
        return result;
    }
    shared_ptr <obj_t>
    list_append (
        env_t &env,
        shared_ptr <obj_t> ante,
        shared_ptr <obj_t> succ)
    {
        auto l = list_reverse (env, ante);
        auto result = succ;
        while (! null_p (env, l)) {
            auto obj = car (env, l);
            result = cons_c (env, obj, result);
            l = cdr (env, l);
        }
        return result;
    }
    shared_ptr <obj_t>
    unit_list (env_t &env, shared_ptr <obj_t> obj)
    {
        return cons_c (env, obj, null_c (env));
    }
    struct vect_o: obj_t
    {
        obj_vector_t obj_vector;
        vect_o (env_t &env, obj_vector_t obj_vector);
        bool eq (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    vect_o::vect_o (env_t &env, obj_vector_t obj_vector)
    {
        this->tag = vect_tag;
        this->obj_vector = obj_vector;
    }
    shared_ptr <vect_o>
    make_vect (env_t &env, obj_vector_t obj_vector)
    {
        return make_shared <vect_o> (env, obj_vector);
    }
    shared_ptr <vect_o>
    as_vect (shared_ptr <obj_t> obj)
    {
        assert (obj->tag == vect_tag);
        return static_pointer_cast <vect_o> (obj);
    }
    bool
    vect_eq (
        env_t &env,
        obj_vector_t &lhs,
        obj_vector_t &rhs)
    {
        if (lhs.size () != rhs.size ()) return false;
        auto size = lhs.size ();
        size_t index = 0;
        while (index < size) {
            if (! obj_eq (env, lhs [index], rhs [index]))
                return false;
            index++;
        }
        return true;
    }
    bool
    vect_o::eq (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = as_vect (obj);
        return vect_eq (env, this->obj_vector, that->obj_vector);
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
        return a->tag == vect_tag;
    }
    shared_ptr <vect_o>
    list_to_vect (env_t &env, shared_ptr <obj_t> l)
    {
        auto obj_vector = obj_vector_t ();
        while (cons_p (env, l)) {
            obj_vector.push_back (car (env, l));
            l = cdr (env, l);
        }
        return make_vect (env, obj_vector);
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
    shared_ptr <num_o>
    vect_length (env_t &env, shared_ptr <vect_o> vect)
    {
        return make_num (env, vect->obj_vector.size ());
    }
    shared_ptr <vect_o>
    vect_append (
        env_t &env,
        shared_ptr <vect_o> ante,
        shared_ptr <vect_o> succ)
    {
        auto obj_vector = obj_vector_t ();
        for (auto obj: ante->obj_vector)
            obj_vector.push_back (obj);
        for (auto obj: succ->obj_vector)
            obj_vector.push_back (obj);
        return make_vect (env, obj_vector);
    }
    shared_ptr <vect_o>
    vect_slice (
        env_t &env,
        shared_ptr <vect_o> vect,
        shared_ptr <num_o> begin,
        shared_ptr <num_o> end)
    {
        auto size = vect->obj_vector.size ();
        assert (begin->num >= 0);
        assert (end->num < size);
        auto obj_vector = obj_vector_t ();
        for (auto it = vect->obj_vector.begin () + begin->num;
             it != vect->obj_vector.begin () + end->num;
             it++) {
            auto obj = *it;
            obj_vector.push_back (obj);
        }
        return make_vect (env, obj_vector);
    }
    shared_ptr <obj_t>
    vect_ref (
        env_t &env,
        shared_ptr <vect_o> vect,
        shared_ptr <num_o> index)
    {
        auto size = vect->obj_vector.size ();
        assert (index->num >= 0);
        assert (index->num < size);
        return vect->obj_vector [index->num];
    }
    shared_ptr <obj_t>
    vect_head (
        env_t &env,
        shared_ptr <vect_o> vect)
    {
        auto size = vect->obj_vector.size ();
        assert (size >= 1);
        return vect->obj_vector [0];
    }
    shared_ptr <vect_o>
    vect_rest (
        env_t &env,
        shared_ptr <vect_o> vect)
    {
        auto size = vect->obj_vector.size ();
        assert (size >= 1);
        auto obj_vector = obj_vector_t ();
        for (auto it = vect->obj_vector.begin () + 1;
             it != vect->obj_vector.end ();
             it++) {
            auto obj = *it;
            obj_vector.push_back (obj);
        }
        return make_vect (env, obj_vector);
    }
    shared_ptr <vect_o>
    vect_reverse (
        env_t &env,
        shared_ptr <vect_o> vect)
    {
        auto obj_vector = vect->obj_vector;
        reverse (obj_vector.begin (),
                 obj_vector.end ());
        return make_vect (env, obj_vector);
    }
    shared_ptr <vect_o>
    unit_vect (
        env_t &env,
        shared_ptr <obj_t> obj)
    {
        auto obj_vector = obj_vector_t ();
        obj_vector.push_back (obj);
        return make_vect (env, obj_vector);
    }
    shared_ptr <obj_t>
    nothing_c (env_t &env)
    {
       return make_shared <data_o>
           (env, nothing_tag, obj_map_t ());
    }
    bool
    nothing_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == nothing_tag;
    }
    shared_ptr <obj_t>
    just_c (
        env_t &env,
        shared_ptr <obj_t> value)
    {
        auto obj_map = obj_map_t ();
        obj_map ["value"] = value;
        return make_shared <data_o>
            (env, just_tag, obj_map);
    }
    bool
    just_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == just_tag;
    }
    shared_ptr <obj_t>
    value_of_just (env_t &env, shared_ptr <obj_t> just)
    {
        assert (just_p (env, just));
        return just->obj_map ["value"];
    }
    bool
    maybe_p (env_t &env, shared_ptr <obj_t> a)
    {
        return nothing_p (env, a)
            or just_p (env, a);
    }
    struct dict_o: obj_t
    {
        dict_o (env_t &env, obj_map_t obj_map);
        bool eq (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    dict_o::dict_o (env_t &env, obj_map_t obj_map)
    {
        this->tag = dict_tag;
        this->obj_map = obj_map;
    }
    shared_ptr <dict_o>
    make_dict (env_t &env, obj_map_t obj_map)
    {
        return make_shared <dict_o> (env, obj_map);
    }
    shared_ptr <dict_o>
    as_dict (shared_ptr <obj_t> obj)
    {
        assert (obj->tag == dict_tag);
        return static_pointer_cast <dict_o> (obj);
    }
    bool
    dict_o::eq (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = as_dict (obj);
        return obj_map_eq (env, this->obj_map, that->obj_map);
    }
    string
    dict_o::repr (env_t &env)
    {
        string repr = "{";
        repr += obj_map_repr (env, this->obj_map);
        repr += "}";
        return repr;
    }
    bool
    dict_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == dict_tag;
    }
    shared_ptr <dict_o>
    list_to_dict (env_t &env, shared_ptr <obj_t> l)
    {
        auto obj_map = obj_map_t ();
        while (! null_p (env, l)) {
            auto pair = car (env, l);
            auto sym = as_sym (car (env, pair));
            auto obj = car (env, cdr (env, pair));
            obj_map [sym->sym] = obj;
            l = cdr (env, l);
        }
        return make_dict (env, obj_map);
    }
    shared_ptr <obj_t>
    dict_to_list (env_t &env, shared_ptr <dict_o> dict)
    {
        auto result = null_c (env);
        for (auto &kv: dict->obj_map) {
            auto sym = make_sym (env, kv.first);
            auto obj = kv.second;
            auto pair = cons_c (env, sym, unit_list (env, obj));
            result = cons_c (env, pair, result);
        }
        return result;
    }
    shared_ptr <obj_t>
    dict_to_flat_list (env_t &env, shared_ptr <dict_o> dict)
    {
        auto result = null_c (env);
        for (auto &kv: dict->obj_map) {
            auto sym = make_sym (env, kv.first);
            auto key = cons_c (
                env,
                make_sym (env, "quote"),
                unit_list (env, sym));
            auto obj = kv.second;
            result = cons_c (env, obj, result);
            result = cons_c (env, key, result);
        }
        return result;
    }
    shared_ptr <num_o>
    dict_length (env_t &env, shared_ptr <dict_o> dict)
    {
        return make_num (env, dict->obj_map.size ());
    }
    shared_ptr <obj_t>
    dict_key_list (env_t &env, shared_ptr <dict_o> dict)
    {
        auto result = null_c (env);
        for (auto &kv: dict->obj_map) {
            auto sym = make_sym (env, kv.first);
            result = cons_c (env, sym, result);
        }
        return result;
    }
    shared_ptr <obj_t>
    dict_value_list (env_t &env, shared_ptr <dict_o> dict)
    {
        auto result = null_c (env);
        for (auto &kv: dict->obj_map) {
            auto obj = kv.second;
            result = cons_c (env, obj, result);
        }
        return result;
    }
    shared_ptr <dict_o>
    dict_insert (
        env_t &env,
        shared_ptr <dict_o> dict,
        shared_ptr <sym_o> sym,
        shared_ptr <obj_t> value)
    {
        auto obj_map = dict->obj_map;
        auto key = sym->sym;
        obj_map [key] = value;
        return make_dict (env, obj_map);
    }
    shared_ptr <dict_o>
    dict_merge (
        env_t &env,
        shared_ptr <dict_o> ante,
        shared_ptr <dict_o> succ)
    {
        auto obj_map = ante->obj_map;
        for (auto &kv: succ->obj_map) {
            auto key = kv.first;
            auto value = kv.second;
            obj_map [key] = value;
        }
        return make_dict (env, obj_map);
    }
    shared_ptr <obj_t>
    dict_find (
        env_t &env,
        shared_ptr <dict_o> dict,
        shared_ptr <sym_o> sym)
    {
        auto obj_map = dict->obj_map;
        auto key = sym->sym;
        auto it = obj_map.find (key);
        if (it != obj_map.end ()) {
            auto value = it->second;
            return just_c (env, value);
        }
        else {
            return nothing_c (env);
        }
    }
    bool
    space_char_p (char c)
    {
        return c == ' '
            or c == '\n'
            or c == '\t';
    }
    bool
    delimiter_char_p (char c)
    {
        return c == '('
            or c == ')'
            or c == '['
            or c == ']'
            or c == '{'
            or c == '}'
            or c == ','
            or c == '`'
            or c == '\'';
    }
    bool
    semicolon_char_p (char c)
    {
        return (c == ';');
    }
    bool
    newline_char_p (char c)
    {
        return (c == '\n');
    }
    string
    string_from_char (char c)
    {
        string str = "";
        str.push_back (c);
        return str;
    }
    bool
    doublequote_char_p (char c)
    {
        return c == '"';
    }
    size_t
    find_word_length (string code, size_t begin)
    {
        size_t length = code.length ();
        size_t index = begin;
        while (true) {
            if (index == length)
                return index - begin;
            char c = code [index];
            if (space_char_p (c) or
                doublequote_char_p (c) or
                semicolon_char_p (c) or
                delimiter_char_p (c))
                return index - begin;
            index++;
        }
    }
    size_t
    find_string_length (string code, size_t begin)
    {
        size_t length = code.length ();
        size_t index = begin + 1;
        while (true) {
            if (index == length) {
                cout << "- fatal error : find_string_length" << "\n";
                cout << "  doublequote mismatch" << "\n";
                exit (1);
            }
            char c = code [index];
            if (doublequote_char_p (c))
                return index - begin + 1;
            index++;
        }
    }
    size_t
    find_comment_length (string code, size_t begin)
    {
        size_t length = code.length ();
        size_t index = begin;
        while (true) {
            if (index == length) {
                cout << "- fatal error : find_string_length" << "\n";
                cout << "  end-of-line mismatch" << "\n";
                exit (1);
            }
            char c = code [index];
            if (newline_char_p (c))
                return index - begin + 1;
            index++;
        }
    }
    string_vector_t
    scan_word_vector (string code)
    {
        auto string_vector = string_vector_t ();
        size_t i = 0;
        while (i < code.length ()) {
            char c = code [i];
            if (space_char_p (c)) i++;
            else if (delimiter_char_p (c)) {
                string_vector.push_back (string_from_char (c));
                i++;
            }
            else if (semicolon_char_p (c)) {
                auto length = find_comment_length (code, i);
                i += length;
            }
            else if (doublequote_char_p (c)) {
                auto length = find_string_length (code, i);
                string str = code.substr (i, length);
                string_vector.push_back (str);
                i += length;
            }
            else {
                auto length = find_word_length (code, i);
                string word = code.substr (i, length);
                string_vector.push_back (word);
                i += length;
            }
        }
        return string_vector;
    }
    void
    test_scan ()
    {
        auto code = "(cons-c <car> <cdr>)";
        auto string_vector = scan_word_vector (code);
        assert (string_vector.size () == 5);
        assert (string_vector [0] == "(");
        assert (string_vector [1] == "cons-c");
        assert (string_vector [2] == "<car>");
        assert (string_vector [3] == "<cdr>");
        assert (string_vector [4] == ")");

        {
            auto code = "\"123\"";
            auto string_vector = scan_word_vector (code);
            assert (string_vector.size () == 1);
            assert (string_vector [0] == "\"123\"");
        }

    }
    shared_ptr <obj_t>
    word_vector_to_word_list
    (env_t &env, string_vector_t &word_vector)
    {
        auto begin = word_vector.rbegin ();
        auto end = word_vector.rend ();
        auto collect = null_c (env);
        for (auto it = begin; it != end; it++) {
            auto word = *it;
            if (word != ",") {
                auto obj = make_str (env, word);
                collect = cons_c (env, obj, collect);
            }
        }
        return collect;
    }
    shared_ptr <obj_t>
    scan_word_list (env_t &env, shared_ptr <str_o> code)
    {
        auto word_vector = scan_word_vector (code->str);
        return word_vector_to_word_list
            (env, word_vector);
    }
    bool
    bar_word_p (string word)
    {
        return word == "("
            or word == "["
            or word == "{";
    }
    bool
    ket_word_p (string word)
    {
        return word == ")"
            or word == "]"
            or word == "}";
    }
    bool
    quote_word_p (string word)
    {
        return word == "'"
            or word == "`";
    }
    bool
    unquote_word_p (string word)
    {
        return word == "~"
            or word == "~@";
    }
    string
    bar_word_to_ket_word (string bar)
    {
        assert (bar_word_p (bar));
        if (bar == "(") return ")";
        if (bar == "[") return "]";
        if (bar == "{") return "}";
        cout << "bar_word_to_ket_word fail\n";
        exit (1);
    }
    shared_ptr <obj_t>
    word_list_head_with_bar_ket_counter (
        env_t &env,
        shared_ptr <obj_t> word_list,
        string bar,
        string ket,
        size_t counter)
    {
        if (counter == 0)
            return null_c (env);
        auto head = as_str (car (env, word_list));
        auto word = head->str;
        if (word == bar)
            return cons_c (
                env, head, word_list_head_with_bar_ket_counter (
                    env, cdr (env, word_list),
                    bar, ket, counter + 1));
        if (word == ket)
            return cons_c (
                env, head, word_list_head_with_bar_ket_counter (
                    env,
                    cdr (env, word_list),
                    bar, ket, counter - 1));
        else
            return cons_c (
                env, head, word_list_head_with_bar_ket_counter (
                    env,
                    cdr (env, word_list),
                    bar, ket, counter));
    }
    shared_ptr <obj_t>
    word_list_head (env_t &env, shared_ptr <obj_t> word_list)
    {
        assert (cons_p (env, word_list));
        auto head = as_str (car (env, word_list));
        auto word = head->str;
        if (bar_word_p (word)) {
            auto bar = word;
            auto ket = bar_word_to_ket_word (word);
            return cons_c (
                env, head, word_list_head_with_bar_ket_counter (
                    env,
                    cdr (env, word_list),
                    bar, ket, 1));
        }
        else if (quote_word_p (word))
            return cons_c (
                env, head, word_list_head (env, cdr (env, word_list)));
        else if (unquote_word_p (word))
            return cons_c (
                env, head, word_list_head (env, cdr (env, word_list)));
        else
            return unit_list (env, head);
    }
    shared_ptr <obj_t>
    word_list_rest_with_bar_ket_counter (
        env_t &env,
        shared_ptr <obj_t> word_list,
        string bar,
        string ket,
        size_t counter)
    {
        if (counter == 0)
            return word_list;
        auto head = as_str (car (env, word_list));
        auto word = head->str;
        if (word == bar)
            return word_list_rest_with_bar_ket_counter (
                env,
                cdr (env, word_list),
                bar, ket, counter + 1);
        if (word == ket)
            return word_list_rest_with_bar_ket_counter (
                env,
                cdr (env, word_list),
                bar, ket, counter - 1);
        else
            return word_list_rest_with_bar_ket_counter (
                env,
                cdr (env, word_list),
                bar, ket, counter);
    }
    shared_ptr <obj_t>
    word_list_rest (env_t &env, shared_ptr <obj_t> word_list)
    {
        assert (cons_p (env, word_list));
        auto head = as_str (car (env, word_list));
        auto word = head->str;
        if (bar_word_p (word)) {
            auto bar = word;
            auto ket = bar_word_to_ket_word (word);
            return word_list_rest_with_bar_ket_counter
                (env,
                 cdr (env, word_list),
                 bar, ket, 1);
        }
        else if (quote_word_p (word))
            return word_list_rest (env, cdr (env, word_list));
        else if (unquote_word_p (word))
            return word_list_rest (env, cdr (env, word_list));
        else
            return cdr (env, word_list);
    }
    shared_ptr <obj_t>
    word_list_drop_ket (
        env_t &env,
        shared_ptr <obj_t> word_list,
        string ket)
    {
        auto head = car (env, word_list);
        auto rest = cdr (env, word_list);
        if (null_p (env, rest))
            return null_c (env);
        auto cdr_rest = cdr (env, rest);
        auto car_rest = as_str (car (env, rest));
        auto word = car_rest->str;
        if (null_p (env, cdr_rest)) {
            assert (word == ket);
            return unit_list (env, head);
        }
        else {
            return cons_c (
                env, head,
                word_list_drop_ket (env, rest, ket));
        }
    }
    bool
    string_string_p (string str)
    {
        auto size = str.size ();
        if (size < 2) return false;
        if (str [0] != '"') return false;
        if (str [size-1] != '"') return false;
        return true;
    }
    string
    string_string_to_string (string str)
    {
        auto size = str.size () - 2;
        return str.substr (1, size);
    }
    bool
    num_string_p (string str)
    {
        auto size = str.size ();
        if (size < 1) return false;
        if (str [0] == '-')
            return num_string_p (str.substr (1, size - 1));
        auto string_vector = string_split (str, '.');
        if (string_vector.size () == 0) return false;
        if (string_vector.size () >= 3) return false;
        auto pos = str.find_first_not_of ("0123456789.");
        if (pos != string::npos) {
            return false;
        }
        else {
            return true;
        }
    }
    shared_ptr <vect_o>
    sexp_list_to_vect (env_t &env, shared_ptr <obj_t> sexp_list)
    {
        return list_to_vect (env, sexp_list);
    }
    shared_ptr <obj_t>
    sexp_list_prefix_assign (
        env_t &env,
        shared_ptr <obj_t> sexp_list);

    shared_ptr <obj_t>
    sexp_list_prefix_assign_with_last_sexp (
        env_t &env,
        shared_ptr <obj_t> sexp_list,
        shared_ptr <obj_t> last_sexp)
    {
        if (null_p (env, sexp_list)) {
            return unit_list (env, last_sexp);
        }
        else {
            auto head = car (env, sexp_list);
            if (sym_p (env, head) and as_sym (head) ->sym == "=") {
                auto next = car (env, cdr (env, sexp_list));
                auto rest = cdr (env, cdr (env, sexp_list));
                auto new_last_sexp = cons_c (
                    env, head, cons_c (
                        env, last_sexp,
                        unit_list (env, next)));
                return cons_c (
                    env, new_last_sexp,
                    sexp_list_prefix_assign (
                        env, rest));
            }
            else {
                auto rest = cdr (env, sexp_list);
                return cons_c (
                    env, last_sexp,
                    sexp_list_prefix_assign_with_last_sexp (
                        env, rest, head));
            }
        }
    }
    shared_ptr <obj_t>
    sexp_list_prefix_assign (
        env_t &env,
        shared_ptr <obj_t> sexp_list)
    {
        if (null_p (env, sexp_list))
            return sexp_list;
        else {
            return sexp_list_prefix_assign_with_last_sexp (
                env,
                cdr (env, sexp_list),
                car (env, sexp_list));
        }
    }
    shared_ptr <obj_t>
    sexp_list_assign_to_pair (
        env_t &env,
        shared_ptr <obj_t> sexp_list)
    {
        if (null_p (env, sexp_list))
            return sexp_list;
        else
            return cons_c (
                env,
                cdr (env, car (env, sexp_list)),
                sexp_list_assign_to_pair (
                    env, cdr (env, sexp_list)));
    }
    shared_ptr <dict_o>
    sexp_list_to_dict (env_t &env, shared_ptr <obj_t> sexp_list)
    {
        return list_to_dict (
            env, sexp_list_assign_to_pair (
                env, sexp_list_prefix_assign (env, sexp_list)));
    }
    shared_ptr <obj_t>
    parse_sexp_list (env_t &env, shared_ptr <obj_t> word_list);

    shared_ptr <obj_t>
    parse_sexp (env_t &env, shared_ptr <obj_t> word_list)
    {
        auto head = as_str (car (env, word_list));
        auto word = head->str;
        auto rest = cdr (env, word_list);
        if (word == "(")
            return parse_sexp_list (
                env,
                word_list_drop_ket (env, rest, ")"));
        else if (word == "[")
            return sexp_list_to_vect (
                env, parse_sexp_list (
                    env, word_list_drop_ket (env, rest, "]")));
        else if (word == "{")
            return sexp_list_to_dict (
                env, parse_sexp_list (
                    env, word_list_drop_ket (env, rest, "}")));
        else if (word == "'")
            return cons_c (env, make_sym (env, "quote"),
                           unit_list (env, parse_sexp (env, rest)));
        else if (word == "`")
            return cons_c (env, make_sym (env, "quasiquote"),
                           unit_list (env, parse_sexp (env, rest)));
        else if (word == "~")
            return cons_c (env, make_sym (env, "unquote"),
                           unit_list (env, parse_sexp (env, rest)));
        else if (word == "~@")
            return cons_c (env, make_sym (env, "unquote-splicing"),
                           unit_list (env, parse_sexp (env, rest)));
        else if (num_string_p (word))
            return make_num (env, s2n (word));
        else if (string_string_p (word))
            return make_str (env, string_string_to_string (word));
        else
            return make_sym (env, word);
    }
    shared_ptr <obj_t>
    parse_sexp_list (env_t &env, shared_ptr <obj_t> word_list)
    {
        if (null_p (env, word_list))
            return word_list;
        else
            return cons_c (
                env,
                parse_sexp (env, word_list_head (env, word_list)),
                parse_sexp_list (env, word_list_rest (env, word_list)));
    }
    string
    sexp_list_repr (env_t &env, shared_ptr <obj_t> a);

    string
    sexp_repr (env_t &env, shared_ptr <obj_t> a)
    {
        if (null_p (env, a)) {
            return "()";
        }
        else if (cons_p (env, a)) {
            return "(" + sexp_list_repr (env, a) + ")";
        }
        else if (vect_p (env, a)) {
            auto v = as_vect (a);
            auto l = vect_to_list (env, v);
            return "[" + sexp_list_repr (env, l) + "]";
        }
        else if (dict_p (env, a)) {
            auto d = as_dict (a);
            auto l = dict_to_list (env, d);
            return "{" + sexp_list_repr (env, l) + "}";
        }
        else if (str_p (env, a)) {
            auto str = as_str (a);
            return '"' + str->str + '"';
        }
        else if (sym_p (env, a)) {
            auto sym = as_sym (a);
            return sym->sym;
        }
        else {
            return a->repr (env);
        }
    }
    string
    sexp_list_repr (env_t &env, shared_ptr <obj_t> sexp_list)
    {
        if (null_p (env, sexp_list))
            return "";
        else if (null_p (env, cdr (env, sexp_list)))
            return sexp_repr (env, car (env, sexp_list));
        else {
            return
                sexp_repr (env, car (env, sexp_list)) + " " +
                sexp_list_repr (env, cdr (env, sexp_list));
        }
    }
    string
    system_env_find (string name)
    {
        auto env_var = name.c_str ();
        const char* env_p = getenv (env_var);
        if (env_p) {
            return string (env_p);
        }
        else {
            return string ();
        }
    }
    bool
    dollar_string_p (string s)
    {
        auto size = s.size ();
        if (size < 2) return false;
        return s [0] == '$';
    }
    name_t
    dollar_string_to_name (string dollar_string)
    {
        assert (dollar_string_p (dollar_string));
        auto size = dollar_string.size ();
        return dollar_string.substr (1, size -1);
    }
    path_t
    expend_path (path_t p)
    {
        auto string_vector = string_split (p.string (), '/');
        auto result_vector = string_vector_t ();
        for (auto str: string_vector) {
            if (dollar_string_p (str)) {
                auto name = dollar_string_to_name (str);
                auto result = system_env_find (name);
                if (result == "") {
                    cout << "- fatal error : expend_path" << "\n";
                    cout << "  a var is unbound" << "\n";
                    cout << "  var : " << str << "\n";
                    exit (1);
                }
                auto pos = result.find (":");
                if (pos != string::npos) {
                    cout << "- fatal error : expend_path" << "\n";
                    cout << "  a var must be bound to one string" << "\n";
                    cout << "  var : " << str << "\n";
                    cout << "  string list : " << result << "\n";
                    exit (1);
                }
                result_vector.push_back (result);
            }
            else {
                result_vector.push_back (str);
            }
        }
        return path_t (string_vector_join (result_vector, '/'));
    }
    struct module_o: obj_t
    {
        env_t module_env;
        module_o (env_t &env, env_t module_env);
        bool eq (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    module_o::module_o (env_t &env, env_t module_env)
    {
        this->tag = module_tag;
        this->module_env = module_env;
        for (auto &kv: module_env.box_map) {
            auto name = kv.first;
            auto box = kv.second;
            if (! box->empty_p) {
                this->obj_map [name] = box->obj;
            }
        }
    }
    bool
    eq_env_p (env_t &lhs, env_t &rhs)
    {
        return false;
    }
    bool
    module_o::eq (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = static_pointer_cast <module_o> (obj);
        return eq_env_p (this->module_env, that->module_env);
    }
    string
    module_o::repr (env_t &env)
    {
        return "(module)";
    }
    shared_ptr <jojo_t>
    symbol_compile (
        env_t &env,
        local_ref_map_t &local_ref_map,
        symbol sym);

    shared_ptr <jojo_t>
    sexp_compile (
        env_t &env,
        local_ref_map_t &local_ref_map,
        shared_ptr <obj_t> sexp);

    shared_ptr <jojo_t>
    sexp_list_compile (
        env_t &env,
        local_ref_map_t &local_ref_map,
        shared_ptr <obj_t> sexp_list);
        struct lit_jo_t: jo_t
        {
            shared_ptr <obj_t> obj;
            lit_jo_t (shared_ptr <obj_t> obj);
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        lit_jo_t::
        lit_jo_t (shared_ptr <obj_t> obj)
        {
            this->obj = obj;
        }
        jo_t *
        lit_jo_t::copy ()
        {
            return new lit_jo_t (this->obj);
        }
        void
        lit_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            env.obj_stack.push (this->obj);
        }
        string
        lit_jo_t::repr (env_t &env)
        {
            return this->obj->repr (env);
        }
      shared_ptr <jojo_t>
      lit_compile (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> sexp)
      {
          auto jo_vector = jo_vector_t ();
          jo_vector.push_back (new lit_jo_t (sexp));;
          auto jojo = make_shared <jojo_t> (jo_vector);
          return jojo;
      }
        struct field_jo_t: jo_t
        {
            name_t name;
            jo_t * copy ();
            field_jo_t (name_t name);
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        field_jo_t::field_jo_t (name_t name)
        {
            this->name = name;
        }
        jo_t *
        field_jo_t::copy ()
        {
            return new field_jo_t (this->name);
        }
        void
        field_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            auto obj = env.obj_stack.top ();
            env.obj_stack.pop ();
            auto it = obj->obj_map.find (this->name);
            if (it != obj->obj_map.end ()) {
                env.obj_stack.push (it->second);
            }
            else {
                auto type = type_of (env, obj);
                auto it = type->obj_map.find (this->name);
                if (it != type->obj_map.end ()) {
                    if (it->second->tag == closure_tag) {
                        auto method = static_pointer_cast <closure_o>
                            (it->second);
                        assert (method->name_vector.size () == 1);
                        env.obj_stack.push (obj);
                        method->apply (env, 1);
                    }
                    else {
                        env.obj_stack.push (it->second);
                    }
                }
                else {
                    cout << "- fatal error : field_jo_t::exe" << "\n";
                    cout << "  unknown field : " << this->name << "\n";
                    cout << "  fail to find it in both object and type" << "\n";
                    exit (1);
                }
            }
        }
        string
        field_jo_t::repr (env_t &env)
        {
            return "." + this->name;
        }
      bool
      field_symbol_p (string str)
      {
          if (string_string_p (str)) return false;
          auto pos = str.find (".");
          return (pos != string::npos);
      }
      shared_ptr <jojo_t>
      field_symbol_compile (
          env_t &env,
          local_ref_map_t &local_ref_map,
          string str)
      {
          auto string_vector = string_split (str, '.');
          auto jojo = symbol_compile
              (env, local_ref_map, string_vector [0]);
          auto begin = string_vector.begin () + 1;
          auto end = string_vector.end ();
          auto jo_vector = jo_vector_t ();
          for (auto it = begin; it != end; it++) {
              jo_vector.push_back (new field_jo_t (*it));
          }
          return jojo_append (jojo, make_shared <jojo_t> (jo_vector));
      }
      bool
      dot_symbol_p (string str)
      {
          auto size = str.size ();
          if (size < 1) return false;
          return (str [0] == '.');
      }
      shared_ptr <jojo_t>
      dot_symbol_compile (
          env_t &env,
          local_ref_map_t &local_ref_map,
          string str)
      {
          auto string_vector = string_split (str, '.');
          auto begin = string_vector.begin () + 1;
          auto end = string_vector.end ();
          auto jo_vector = jo_vector_t ();
          for (auto it = begin; it != end; it++) {
              jo_vector.push_back (new field_jo_t (*it));
          }
          return make_shared <jojo_t> (jo_vector);
      }
        struct ref_jo_t: jo_t
        {
            box_t *box;
            ref_jo_t (box_t *);
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        ref_jo_t::ref_jo_t (box_t *box)
        {
            this->box = box;
        }
        jo_t *
        ref_jo_t::copy ()
        {
            return new ref_jo_t (this->box);
        }
        void
        ref_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            if (this->box->empty_p) {
                cout << "- fatal error : ref_jo_t::exe fail" << "\n";
                cout << "  undefined name : "
                     << name_of_box (env, box) << "\n";
                exit (1);
            }
            else {
                env.obj_stack.push (this->box->obj);
            }
        }
        string
        ref_jo_t::repr (env_t &env)
        {
            return name_of_box (env, this->box);
        }
        struct local_ref_jo_t: jo_t
        {
            size_t level;
            size_t index;
            local_ref_jo_t (size_t level, size_t index);
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        local_ref_jo_t::
        local_ref_jo_t (size_t level, size_t index)
        {
            this->level = level;
            this->index = index;
        }
        jo_t *
        local_ref_jo_t::copy ()
        {
            return new local_ref_jo_t (this->level, this->index);
        }
        void
        local_ref_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            // this is the only place where
            //   the local_scope in the arg of exe is uesd.
            auto bind_vector =
                local_scope [local_scope.size () - this->level - 1];
            auto bind =
                bind_vector [bind_vector.size () - this->index - 1];
            auto obj = bind.second;
            env.obj_stack.push (obj);
        }
        string
        local_ref_jo_t::repr (env_t &env)
        {
            return "local." +
                to_string (this->level) + "." +
                to_string (this->index);
        }
      shared_ptr <jojo_t>
      ref_compile (
          env_t &env,
          local_ref_map_t &local_ref_map,
          name_t name)
      {
          auto jo_vector = jo_vector_t ();
          auto it = local_ref_map.find (name);
          if (it != local_ref_map.end ()) {
              auto local_ref = it->second;
              auto local_ref_jo = new local_ref_jo_t
                  (local_ref.first,
                   local_ref.second);
              jo_vector.push_back (local_ref_jo);
          }
          else
              jo_vector.push_back (new ref_jo_t (boxing (env, name)));
          return make_shared <jojo_t> (jo_vector);
      }
    shared_ptr <jojo_t>
    symbol_compile (
        env_t &env,
        local_ref_map_t &local_ref_map,
        symbol sym)
    {
        if (dot_symbol_p (sym))
            return dot_symbol_compile (env, local_ref_map, sym);
        else if (field_symbol_p (sym))
            return field_symbol_compile (env, local_ref_map, sym);
        else
            return ref_compile (env, local_ref_map, sym);
    }
        struct collect_vect_jo_t: jo_t
        {
            size_t counter;
            collect_vect_jo_t (size_t counter);
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        collect_vect_jo_t::
        collect_vect_jo_t (size_t counter)
        {
            this->counter = counter;
        }
        jo_t *
        collect_vect_jo_t::copy ()
        {
            return new collect_vect_jo_t (this->counter);
        }
        void
        collect_vect_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            size_t index = 0;
            auto obj_vector = obj_vector_t ();
            while (index < this->counter) {
                auto obj = env.obj_stack.top ();
                env.obj_stack.pop ();
                obj_vector.push_back (obj);
                index++;
            }
            reverse (obj_vector.begin (),
                     obj_vector.end ());
            auto vect = make_vect (env, obj_vector);
            env.obj_stack.push (vect);
        }
        string
        collect_vect_jo_t::repr (env_t &env)
        {
            return "(collect-vect " + to_string (this->counter) + ")";
        }
      shared_ptr <jojo_t>
      vect_compile (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <vect_o> vect)
      {
          auto sexp_list = vect_to_list (env, vect);
          auto jojo = sexp_list_compile
              (env, local_ref_map, sexp_list);
          auto counter = list_size (env, sexp_list);
          jo_vector_t jo_vector = {
              new collect_vect_jo_t (counter),
          };
          auto ending_jojo = make_shared <jojo_t> (jo_vector);
          return jojo_append (jojo, ending_jojo);
      }
        struct collect_dict_jo_t: jo_t
        {
            size_t counter;
            collect_dict_jo_t (size_t counter);
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        collect_dict_jo_t::
        collect_dict_jo_t (size_t counter)
        {
            this->counter = counter;
        }
        jo_t *
        collect_dict_jo_t::copy ()
        {
            return new collect_dict_jo_t (this->counter);
        }
        void
        collect_dict_jo_t::exe (
            env_t &env,
            local_scope_t &local_scope)
        {
            size_t index = 0;
            auto obj_map = obj_map_t ();
            while (index < this->counter) {
                auto obj = env.obj_stack.top ();
                env.obj_stack.pop ();
                auto str = env.obj_stack.top ();
                env.obj_stack.pop ();
                auto key = as_sym (str);
                obj_map [key->sym] = obj;
                index++;
            }
            auto dict = make_dict (env, obj_map);
            env.obj_stack.push (dict);
        }
        string
        collect_dict_jo_t::repr (env_t &env)
        {
            return "(collect-dict " + to_string (this->counter) + ")";
        }
      shared_ptr <jojo_t>
      dict_compile (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <dict_o> dict)
      {
          auto sexp_list = dict_to_flat_list (env, dict);
          auto jojo = sexp_list_compile
              (env, local_ref_map, sexp_list);
          auto counter = list_size (env, sexp_list);
          counter = counter / 2;
          jo_vector_t jo_vector = {
              new collect_dict_jo_t (counter),
          };
          auto ending_jojo = make_shared <jojo_t> (jo_vector);
          return jojo_append (jojo, ending_jojo);
      }
        using keyword_fn = function
            <shared_ptr <jojo_t> (
                 env_t &,
                 local_ref_map_t &,
                 shared_ptr <obj_t>)>;
        struct keyword_o: obj_t
        {
            keyword_fn fn;
            keyword_o (env_t &env, keyword_fn fn);
            bool eq (env_t &env, shared_ptr <obj_t> obj);
        };
        keyword_o::
        keyword_o (env_t &env, keyword_fn fn)
        {
            this->tag = keyword_tag;
            this->fn = fn;
        }
        bool
        keyword_o::eq (env_t &env, shared_ptr <obj_t> obj)
        {
            if (this->tag != obj->tag) return false;
            return this != obj.get ();
        }
        bool
        keyword_p (env_t &env, shared_ptr <obj_t> a)
        {
            return a->tag == keyword_tag;
        }
        void
        define_keyword (env_t &env, name_t name, keyword_fn fn)
        {
            define (env, name, make_shared <keyword_o> (env, fn));
        }
        bool
        keyword_sexp_p (env_t &env, shared_ptr <obj_t> sexp)
        {
            if (! cons_p (env, sexp)) return false;
            if (! sym_p (env, (car (env, sexp)))) return false;
            auto head = as_sym (car (env, sexp));
            auto name = head->sym;
            auto it = env.box_map.find (name);
            if (it != env.box_map.end ()) {
                auto box = it->second;
                if (box->empty_p) return false;
                if (keyword_p (env, box->obj)) return true;
                else return false;
            }
            else {
                return false;
            }
        }
        keyword_fn
        keyword_fn_from_name (env_t &env, name_t name)
        {
            auto it = env.box_map.find (name);
            if (it != env.box_map.end ()) {
                auto box = it->second;
                if (box->empty_p) {
                    cout << "- fatal error: keyword_fn_from_name fail\n";
                    exit (1);
                }
                if (keyword_p (env, box->obj)) {
                    auto keyword = static_pointer_cast <keyword_o>
                        (box->obj);
                    return keyword->fn;
                }
                else {
                    cout << "- fatal error: keyword_fn_from_name fail\n";
                    exit (1);
                };
            }
            else {
                cout << "- fatal error: keyword_fn_from_name fail\n";
                exit (1);
            }
        }
      shared_ptr <jojo_t>
      keyword_compile (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> sexp)
      {
          auto head = as_sym (car (env, sexp));
          auto body = cdr (env, sexp);
          auto name = head->sym;
          auto fn = keyword_fn_from_name (env, name);
          return fn (env, local_ref_map, body);
      }
        struct macro_o: obj_t
        {
            shared_ptr <obj_t> obj;
            macro_o (env_t &env, shared_ptr <obj_t> obj);
            bool eq (env_t &env, shared_ptr <obj_t> obj);
        };
        macro_o::
        macro_o (env_t &env, shared_ptr <obj_t> obj)
        {
            this->tag = macro_tag;
            this->obj = obj;
        }
        shared_ptr <macro_o>
        make_macro (env_t &env, shared_ptr <obj_t> obj)
        {
            return make_shared <macro_o> (env, obj);
        }
        bool
        macro_p (env_t &env, shared_ptr <obj_t> a)
        {
            return a->tag == macro_tag;
        }
        shared_ptr <macro_o>
        as_macro (shared_ptr <obj_t> obj)
        {
            assert (obj->tag == macro_tag);
            return static_pointer_cast <macro_o> (obj);
        }
        bool
        macro_o::eq (env_t &env, shared_ptr <obj_t> obj)
        {
            if (this->tag != obj->tag) return false;
            auto that = as_macro (obj);
            return obj_eq (env, this->obj, that->obj);
        }
        shared_ptr <obj_t>
        sexp_eval (env_t &env, shared_ptr <obj_t> sexp);
        bool
        macro_sexp_p (env_t &env, shared_ptr <obj_t> sexp)
        {
            if (! cons_p (env, sexp)) return false;;
            auto head = car (env, sexp);
            if (! sym_p (env, head)) return false;
            auto sym = as_sym (head);
            auto name = sym->sym;
            auto found = find_obj_from_name (env, name);
            if (! found) return false;
            auto obj = sexp_eval (env, head);
            return macro_p (env, obj);
        }
        shared_ptr <obj_t>
        macro_eval (env_t &env, shared_ptr <obj_t> sexp)
        {
            assert (cons_p (env, sexp));
            auto head = car (env, sexp);
            auto rest = cdr (env, sexp);
            auto obj = sexp_eval (env, head);
            auto macro = as_macro (obj);
            env.obj_stack.push (rest);
            auto base = env.frame_stack.size ();
            macro->obj->apply (env, 1);
            env.run_with_base (base);
            auto new_sexp = env.obj_stack.top ();
            env.obj_stack.pop ();
            return new_sexp;
        }
        void
        define_prim_macro (env_t &env, name_t name, prim_fn fn)
        {
            auto name_vector = name_vector_t ();
            name_vector.push_back ("body");
            auto prim = make_shared <prim_o> (
                env, name_vector, fn, obj_map_t ());
            auto macro = make_shared <macro_o> (env, prim);
            define (env, name, macro);
        }
      shared_ptr <jojo_t>
      macro_compile (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> sexp)
      {
          return sexp_compile (
              env,
              local_ref_map,
              macro_eval (env, sexp));
      }
        struct apply_jo_t: jo_t
        {
            size_t arity;
            apply_jo_t (size_t arity);
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        apply_jo_t::
        apply_jo_t (size_t arity)
        {
            this->arity = arity;
        }
        jo_t *
        apply_jo_t::copy ()
        {
            return new apply_jo_t (this->arity);
        }
        void
        apply_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            auto obj = env.obj_stack.top ();
            env.obj_stack.pop ();
            obj->apply (env, this->arity);
        }
        string
        apply_jo_t::repr (env_t &env)
        {
            return "(apply " +
                to_string (this->arity) + ")";
        }
      size_t
      arity_of_body (env_t &env, shared_ptr <obj_t> body)
      {
          assert (list_p (env, body));
          auto arity = 0;
          while (! null_p (env, body)) {
              auto head = car (env, body);
              if (! sym_p (env, head)) {
                  arity++;
              }
              else {
                  auto sym = as_sym (head) ->sym;
                  if (dot_symbol_p (sym)) {
                      // arity = arity;
                  }
                  else if (sym == "drop") {
                      arity--;
                  }
                  else if (sym == "dup") {
                      arity++;
                  }
                  else if (sym == "over") {
                      arity++;
                  }
                  else if (sym == "tuck") {
                      arity++;
                  }
                  else if (sym == "swap") {
                      // arity = arity;
                  }
                  else {
                      arity++;
                  }
              }
              body = cdr (env, body);
          }
          return arity;
      }
      shared_ptr <jojo_t>
      call_compile (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> sexp)
      {
          auto head = car (env, sexp);
          auto body = cdr (env, sexp);
          auto jo_vector = jo_vector_t ();
          auto arity = arity_of_body (env, body);
          jo_vector.push_back (new apply_jo_t (arity));
          auto jojo = make_shared <jojo_t> (jo_vector);
          auto head_jojo = sexp_compile (env, local_ref_map, head);
          auto body_jojo = sexp_list_compile (env, local_ref_map, body);
          jojo = jojo_append (head_jojo, jojo);
          jojo = jojo_append (body_jojo, jojo);
          return jojo;
      }
        struct apply_to_arg_dict_jo_t: jo_t
        {
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        jo_t *
        apply_to_arg_dict_jo_t::copy ()
        {
            return new apply_to_arg_dict_jo_t ();
        }
        void
        apply_to_arg_dict_jo_t::exe (
            env_t &env,
            local_scope_t &local_scope)
        {
            auto obj = env.obj_stack.top ();
            env.obj_stack.pop ();
            obj->apply_to_arg_dict (env);
        }
        string
        apply_to_arg_dict_jo_t::repr (env_t &env)
        {
            return "(apply-to-arg-dict)";
        }
        void
        closure_o::apply_to_arg_dict (env_t &env)
        {
            auto obj = env.obj_stack.top ();
            env.obj_stack.pop ();
            auto arg_dict = as_dict (obj);
            auto bind_vector = bind_vector_merge_obj_map (
                env, this->bind_vector, arg_dict->obj_map);
            auto size = this->name_vector.size ();
            auto have = number_of_obj_in_bind_vector (bind_vector);
            if (size == have) {
                auto local_scope = local_scope_extend (
                    this->local_scope, bind_vector);
                auto frame = make_shared <frame_t> (
                    this->jojo, local_scope);
                env.frame_stack.push (frame);
            }
            else {
                auto closure = make_closure (
                    env,
                    this->name_vector,
                    this->jojo,
                    bind_vector,
                    this->local_scope);
                env.obj_stack.push (closure);
            }
        }
      bool
      call_with_arg_dict_sexp_p (
          env_t &env,
          shared_ptr <obj_t> sexp)
      {
          if (! cons_p (env, sexp)) return false;
          auto l = cdr (env, sexp);
          while (! null_p (env, l)) {
              auto head = car (env, l);
              if (sym_p (env, head) and as_sym (head) ->sym == "=") {
                  return true;
              }
              if (cons_p (env, head)) {
                  auto head_head = car (env, head);
                  if (sym_p (env, head_head) and
                      as_sym (head_head) ->sym == "=")
                  {
                      return true;
                  }
              }
              l = cdr (env, l);
          }
          return false;
      }
      shared_ptr <jojo_t>
      call_with_arg_dict_compile (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> sexp)
      {
          auto head = car (env, sexp);
          auto body = cdr (env, sexp);
          auto jo_vector = jo_vector_t ();
          jo_vector.push_back (new apply_to_arg_dict_jo_t ());
          auto jojo = make_shared <jojo_t> (jo_vector);
          auto head_jojo = sexp_compile (env, local_ref_map, head);
          auto dict = sexp_list_to_dict (env, body);
          auto body_jojo = dict_compile (env, local_ref_map, dict);
          jojo = jojo_append (head_jojo, jojo);
          jojo = jojo_append (body_jojo, jojo);
          return jojo;
      }
    shared_ptr <jojo_t>
    sexp_compile (
        env_t &env,
        local_ref_map_t &local_ref_map,
        shared_ptr <obj_t> sexp)
    {
        if (str_p (env, sexp) or
            num_p (env, sexp))
        {
            return lit_compile (env, local_ref_map, sexp);
        }
        else if (sym_p (env, sexp)) {
            auto sym = as_sym (sexp);
            return symbol_compile (env, local_ref_map, sym->sym);
        }
        else if (vect_p (env, sexp)) {
            return vect_compile (env, local_ref_map, as_vect (sexp));
        }
        else if (dict_p (env, sexp)) {
            return dict_compile (env, local_ref_map, as_dict (sexp));
        }
        else if (keyword_sexp_p (env, sexp)) {
            return keyword_compile (env, local_ref_map, sexp);
        }
        else if (macro_sexp_p (env, sexp)) {
            return macro_compile (env, local_ref_map, sexp);
        }
        else if (call_with_arg_dict_sexp_p (env, sexp)) {
            return call_with_arg_dict_compile (env, local_ref_map, sexp);
        }
        else {
            assert (cons_p (env, sexp));
            return call_compile (env, local_ref_map, sexp);
        }
    }
    shared_ptr <jojo_t>
    sexp_list_compile (
        env_t &env,
        local_ref_map_t &local_ref_map,
        shared_ptr <obj_t> sexp_list)
    {
        auto jojo = make_shared <jojo_t> (jo_vector_t ());
        if (null_p (env, sexp_list))
            return jojo;
        else {
            assert (cons_p (env, sexp_list));
            auto head_jojo = sexp_compile
                (env, local_ref_map, car (env, sexp_list));
            auto body_jojo = sexp_list_compile
                (env, local_ref_map, cdr (env, sexp_list));
            return jojo_append (head_jojo, body_jojo);
        }
    }
      using top_keyword_fn = function
          <void (env_t &, shared_ptr <obj_t>)>;
      struct top_keyword_o: obj_t
      {
          top_keyword_fn fn;
          top_keyword_o (env_t &env, top_keyword_fn fn);
          bool eq (env_t &env, shared_ptr <obj_t> obj);
      };
      top_keyword_o::
      top_keyword_o (env_t &env, top_keyword_fn fn)
      {
          this->tag = top_keyword_tag;
          this->fn = fn;
      }
      bool
      top_keyword_o::eq (env_t &env, shared_ptr <obj_t> obj)
      {
          if (this->tag != obj->tag) return false;
          return this != obj.get ();
      }
      bool
      top_keyword_p (env_t &env, shared_ptr <obj_t> a)
      {
          return a->tag == top_keyword_tag;
      }
      void
      define_top_keyword (env_t &env, name_t name, top_keyword_fn fn)
      {
          define (env, name, make_shared <top_keyword_o> (env, fn));
      }
      bool
      top_keyword_sexp_p (env_t &env, shared_ptr <obj_t> sexp)
      {
          if (! cons_p (env, sexp)) return false;
          if (! sym_p (env, (car (env, sexp)))) return false;
          auto head = as_sym (car (env, sexp));
          auto name = head->sym;
          auto it = env.box_map.find (name);
          if (it != env.box_map.end ()) {
              auto box = it->second;
              if (box->empty_p) return false;
              if (top_keyword_p (env, box->obj)) return true;
              else return false;
          }
          else {
              return false;
          }
      }
      top_keyword_fn
      top_keyword_fn_from_name (env_t &env, name_t name)
      {
          auto it = env.box_map.find (name);
          if (it != env.box_map.end ()) {
              auto box = it->second;
              if (box->empty_p) {
                  cout << "- fatal error: top_keyword_fn_from_name fail\n";
                  exit (1);
              }
              if (top_keyword_p (env, box->obj)) {
                  auto top_keyword = static_pointer_cast <top_keyword_o>
                      (box->obj);
                  return top_keyword->fn;
              }
              else {
                  cout << "- fatal error: top_keyword_fn_from_name fail\n";
                  exit (1);
              };
          }
          else {
              cout << "- fatal error: top_keyword_fn_from_name fail\n";
              exit (1);
          }
      }
    void
    jojo_run (
        env_t &env,
        local_scope_t &local_scope,
        shared_ptr <jojo_t> jojo)
    {
        auto base = env.frame_stack.size ();
        env.frame_stack.push (make_shared <frame_t> (jojo, local_scope));
        env.run_with_base (base);
    }
    shared_ptr <obj_t>
    jojo_eval (
        env_t &env,
        local_scope_t &local_scope,
        shared_ptr <jojo_t> jojo)
    {
        jojo_run (env, local_scope, jojo);
        auto result = env.obj_stack.top ();
        env.obj_stack.pop ();
        return result;
    }
    void
    jojo_run_in_new_frame (env_t &env, shared_ptr <jojo_t> jojo)
    {
        auto base = env.frame_stack.size ();
        env.frame_stack.push (new_frame_from_jojo (jojo));
        env.run_with_base (base);
    }
    shared_ptr <obj_t>
    jojo_eval_in_new_frame (env_t &env, shared_ptr <jojo_t> jojo)
    {
        jojo_run_in_new_frame (env, jojo);
        auto result = env.obj_stack.top ();
        env.obj_stack.pop ();
        return result;
    }
    void
    sexp_run (env_t &env, shared_ptr <obj_t> sexp)
    {
        if (top_keyword_sexp_p (env, sexp)) {
            cout << "- fatal error : sexp_run" << "\n";
            cout << "  can not handle top_keyword_sexp" << "\n";
            cout << "  sexp : " << sexp_repr (env, sexp) << "\n";
            exit (1);
        }
        else {
            auto local_ref_map = local_ref_map_t ();
            auto jojo = sexp_compile (env, local_ref_map, sexp);
            jojo_run_in_new_frame (env, jojo);
        }
    }
    void
    sexp_list_run (env_t &env, shared_ptr <obj_t> sexp_list)
    {
        if (null_p (env, sexp_list))
            return;
        else {
            sexp_run (env, car (env, sexp_list));
            sexp_list_run (env, cdr (env, sexp_list));
        }
    }
    shared_ptr <obj_t>
    sexp_eval (env_t &env, shared_ptr <obj_t> sexp)
    {
        auto size_before = env.obj_stack.size ();
        sexp_run (env, sexp);
        auto size_after = env.obj_stack.size ();
        if (size_after - size_before == 1) {
            auto obj = env.obj_stack.top ();
            env.obj_stack.pop ();
            return obj;
        }
        else {
            cout << "- fatal error : sexp_eval mismatch" << "\n";
            cout << "  sexp must eval to one value" << "\n";
            cout << "  sexp : " << sexp_repr (env, sexp) << "\n";
            cout << "  stack size before : " << size_before << "\n";
            cout << "  stack size after : " << size_after << "\n";
            exit (1);
        }
    }
    void
    top_sexp_run (env_t &env, shared_ptr <obj_t> sexp)
    {
        if (top_keyword_sexp_p (env, sexp)) {
            auto head = as_sym (car (env, sexp));
            auto body = cdr (env, sexp);
            auto name = head->sym;
            auto fn = top_keyword_fn_from_name (env, name);
            fn (env, body);
        }
        else {
            auto local_ref_map = local_ref_map_t ();
            auto jojo = sexp_compile (env, local_ref_map, sexp);
            jojo_run_in_new_frame (env, jojo);
            if (! env.obj_stack.empty ())
                env.obj_stack.pop ();
        }
    }
    void
    top_sexp_list_run (env_t &env, shared_ptr <obj_t> sexp_list)
    {
        if (null_p (env, sexp_list))
            return;
        else {
            top_sexp_run (env, car (env, sexp_list));
            top_sexp_list_run (env, cdr (env, sexp_list));
        }
    }
    void
    code_run (env_t &env, shared_ptr <str_o> code)
    {
        auto word_list = scan_word_list (env, code);
        auto sexp_list = parse_sexp_list (env, word_list);
        top_sexp_list_run (env, sexp_list);
    }
    shared_ptr <str_o>
    code_from_module_path (env_t &env, path_t module_path)
    {
        auto input_file = ifstream (module_path);
        auto buffer = stringstream ();
        buffer << input_file.rdbuf ();
        auto code = make_str (env, buffer.str ());
        return code;
    }
    path_t
    respect_current_dir (env_t &env, path_t p)
    {
        p = expend_path (p);
        auto normal_path = path_t ();
        if (p.is_absolute ()) {
            normal_path = p;
        }
        else {
            assert (p.is_relative ());
            normal_path = env.current_dir / p;
        }
        // normal_path = fs::canonical (normal_path);
        return normal_path;
    }
    path_t
    respect_module_path (env_t &env, path_t p)
    {
        p = expend_path (p);
        auto normal_path = path_t ();
        if (p.is_absolute ()) {
            normal_path = p;
        }
        else {
            assert (p.is_relative ());
            normal_path = env.module_path.parent_path () / p;
        }
        // normal_path = fs::canonical (normal_path);
        return normal_path;
    }
    void
    expose_core (env_t &env);

    env_t
    env_from_module_path (path_t module_path)
    {
        auto env = env_t ();
        module_path = respect_current_dir (env, module_path);
        if (! fs::exists (module_path)) {
            cout << "- fatal error : env_from_module_path" << "\n";
            cout << "  file does not exists : " << module_path << "\n";
            exit (1);
        }
        if (! fs::is_regular_file (module_path)) {
            cout << "- fatal error : env_from_module_path" << "\n";
            cout << "  not regular file : " << module_path << "\n";
            exit (1);
        }
        env.module_path = module_path;
        expose_core (env);
        auto code = code_from_module_path (env, env.module_path);
        code_run (env, code);
        return env;
    }
    name_t
    prefix_of_string (string str)
    {
        auto string_vector = string_split (str, '.');
        assert (string_vector.size () > 0);
        if (string_vector.size () == 1)
            return "";
        else {
            string_vector.pop_back ();
            return string_vector_join (string_vector, '.');
        }
    }
    name_t
    name_of_string (string str)
    {
        auto string_vector = string_split (str, '.');
        assert (string_vector.size () > 0);
        return string_vector [string_vector.size () - 1];
    }
      bool
      assign_data_p (env_t &env, shared_ptr <obj_t> body)
      {
          if (! cons_p (env, body))
              return false;
          if (! sym_p (env, car (env, body)))
              return false;
          if (! cons_p (env, cdr (env, body)))
              return false;
          if (! cons_p (env, car (env, cdr (env, body))))
              return false;
          if (! sym_p (env, car (env, car (env, cdr (env, body)))))
              return false;
          auto sym = as_sym (car (env, car (env, cdr (env, body))));
          if (sym->sym != "data")
              return false;
          if (null_p (env, cdr (env, car (env, cdr (env, body)))))
              return true;
          if (! sym_p (env, car (env, cdr (env, car (env, cdr (env, body))))))
              return true;
          if (as_sym (car (env, cdr (env, car (env, cdr (env, body)))))
              ->sym == ":>")
              return false;
          if (as_sym (car (env, cdr (env, car (env, cdr (env, body)))))
              ->sym == "<:")
              return false;
          else
              return true;
      }
      void
      tk_assign_data (env_t &env, shared_ptr <obj_t> body)
      {
          auto head = as_sym (car (env, body));
          auto prefix = prefix_of_string (head->sym);
          auto type_name = name_of_string (head->sym);
          auto data_name = name_t2c (type_name);
          auto pred_name = name_t2p (type_name);
          auto tag_of_type = tagging (env, head->sym);
          auto rest = cdr (env, body);
          auto data_body = cdr (env, (car (env, rest)));
          if (null_p (env, data_body)) {
              assign_type
                  (env, prefix, type_name, tag_of_type, {});
              assign_data_pred
                  (env, prefix, pred_name, tag_of_type);
              assign_data
                  (env, prefix, data_name, tag_of_type);
          }
          else {
              auto name_vect = list_to_vect (env, data_body);
              auto name_vector = name_vector_t ();
              for (auto obj: name_vect->obj_vector) {
                  auto sym = as_sym (obj);
                  name_vector.push_back (sym->sym);
              }
              assign_type
                  (env, prefix, type_name, tag_of_type, {});
              assign_data_pred
                  (env, prefix, pred_name, tag_of_type);
              assign_data_cons
                  (env, prefix, data_name, tag_of_type, name_vector);
          }
      }
      bool
      assign_data_inherit_p (env_t &env, shared_ptr <obj_t> body)
      {
          if (! cons_p (env, body))
              return false;
          if (! sym_p (env, car (env, body)))
              return false;
          if (! cons_p (env, cdr (env, body)))
              return false;
          if (! cons_p (env, car (env, cdr (env, body))))
              return false;
          if (! sym_p (env, car (env, car (env, cdr (env, body)))))
              return false;
          auto sym = as_sym (car (env, car (env, cdr (env, body))));
          if (sym->sym != "data")
              return false;
          if (null_p (env, cdr (env, car (env, cdr (env, body)))))
              return false;
          if (! sym_p (env, car (env, cdr (env, car (env, cdr (env, body))))))
              return false;
          if (as_sym (car (env, cdr (env, car (env, cdr (env, body)))))
              ->sym == ":>")
              return false;
          if (as_sym (car (env, cdr (env, car (env, cdr (env, body)))))
              ->sym == "<:")
              return true;
          else
              return false;
      }
      tag_vector_t
      tagging_name_vect (
          env_t &env,
          shared_ptr <vect_o> vect)
      {
          auto tag_vector = tag_vector_t ();
          for (auto obj: vect->obj_vector) {
              auto sym = as_sym (obj);
              auto name = sym->sym;
              auto tag = tagging (env, name);
              tag_vector.push_back (tag);
          }
          return tag_vector;
      }
      void
      tk_assign_data_inherit (env_t &env, shared_ptr <obj_t> body)
      {
          auto head = as_sym (car (env, body));
          auto prefix = prefix_of_string (head->sym);
          auto type_name = name_of_string (head->sym);
          auto data_name = name_t2c (type_name);
          auto pred_name = name_t2p (type_name);
          auto tag_of_type = tagging (env, head->sym);
          auto rest = cdr (env, body);
          auto data_body = cdr (env, (car (env, rest)));
          auto super_name_vect = as_vect (car (env, cdr (env, data_body)));
          auto super_tag_vector = tagging_name_vect (env, super_name_vect);
          data_body = cdr (env, (cdr (env, data_body)));
          if (null_p (env, data_body)) {
              assign_type
                  (env, prefix, type_name, tag_of_type, super_tag_vector);
              assign_data_pred
                  (env, prefix, pred_name, tag_of_type);
              assign_data
                  (env, prefix, data_name, tag_of_type);
          }
          else {
              auto name_vect = list_to_vect (env, data_body);
              auto name_vector = name_vector_t ();
              for (auto obj: name_vect->obj_vector) {
                  auto sym = as_sym (obj);
                  name_vector.push_back (sym->sym);
              }
              assign_type
                  (env, prefix, type_name, tag_of_type, super_tag_vector);
              assign_data_pred
                  (env, prefix, pred_name, tag_of_type);
              assign_data_cons
                  (env, prefix, data_name, tag_of_type, name_vector);
          }
      }
      bool
      assign_data_union_p (env_t &env, shared_ptr <obj_t> body)
      {
          if (! cons_p (env, body))
              return false;
          if (! sym_p (env, car (env, body)))
              return false;
          if (! cons_p (env, cdr (env, body)))
              return false;
          if (! cons_p (env, car (env, cdr (env, body))))
              return false;
          if (! sym_p (env, car (env, car (env, cdr (env, body)))))
              return false;
          auto sym = as_sym (car (env, car (env, cdr (env, body))));
          if (sym->sym != "data")
              return false;
          if (null_p (env, cdr (env, car (env, cdr (env, body)))))
              return false;
          if (! sym_p (env, car (env, cdr (env, car (env, cdr (env, body))))))
              return false;
          if (as_sym (car (env, cdr (env, car (env, cdr (env, body)))))
              ->sym == ":>")
              return true;
          if (as_sym (car (env, cdr (env, car (env, cdr (env, body)))))
              ->sym == "<:")
              return false;
          else
              return false;
      }

      bool
      assign_lambda_sugar_p (env_t &env, shared_ptr <obj_t> body)
      {
          if (! cons_p (env, body))
              return false;
          if (! cons_p (env, car (env, body)))
              return false;
          return true;
      }
      shared_ptr <obj_t>
      assign_lambda_desugar (env_t &env, shared_ptr <obj_t> body)
      {
          auto head = car (env, body);
          auto name = car (env, head);
          auto lambda_body = cdr (env, body);
          lambda_body = cons_c
              (env,
               list_to_vect (env, cdr (env, head)),
               lambda_body);
          lambda_body = cons_c
              (env,
               make_sym (env, "lambda"),
               lambda_body);
          lambda_body = unit_list (env, lambda_body);
          return cons_c (env, name, lambda_body);
      }
      shared_ptr <obj_t>
      sexp_substitute_recur (
          env_t &env,
          shared_ptr <obj_t> sub,
          shared_ptr <obj_t> sexp)
      {
          if (sym_p (env, sexp)) {
              auto sym = as_sym (sexp);
              if (sym->sym == "recur")
                  return sub;
              else
                  return sexp;
          }
          if (cons_p (env, sexp))
              return cons_c
                  (env,
                   sexp_substitute_recur (env, sub, car (env, sexp)),
                   sexp_substitute_recur (env, sub, cdr (env, sexp)));
          if (vect_p (env, sexp)) {
              auto vect_sexp = as_vect (sexp);
              auto list_sexp = vect_to_list (env, vect_sexp);
              auto new_list_sexp = sexp_substitute_recur (env, sub, list_sexp);
              return list_to_vect (env, new_list_sexp);
          }
          else
              return sexp;
      }
      shared_ptr <obj_t>
      sexp_patch_this (env_t &env, shared_ptr <obj_t> sexp)
      {
          auto this_str = make_sym (env, "this");
          obj_vector_t obj_vector = { this_str };
          auto vect = make_vect (env, obj_vector);
          auto lambda_body = unit_list (env, sexp);
          lambda_body = cons_c (env, vect, lambda_body);
          lambda_body = cons_c (
              env,
              make_sym (env, "lambda"),
              lambda_body);
          return lambda_body;
      }
      void
      tk_assign_value (env_t &env, shared_ptr <obj_t> body)
      {
          auto head = as_sym (car (env, body));
          auto rest = cdr (env, body);
          assert (null_p (env, cdr (env, rest)));
          auto sexp = car (env, rest);
          auto name = name_of_string (head->sym);
          auto prefix = prefix_of_string (head->sym);
          if (prefix != "")
              sexp = sexp_patch_this (env, sexp);
          sexp = sexp_substitute_recur (env, head, sexp);
          auto obj = sexp_eval (env, sexp);
          assign (env, prefix, name, obj);
      }
    void
    tk_assign (env_t &env, shared_ptr <obj_t> body)
    {
        if (assign_data_p (env, body))
            tk_assign_data (env, body);
        else if (assign_data_inherit_p (env, body))
            tk_assign_data_inherit (env, body);
        // else if (assign_data_union_p (env, body))
        //     tk_assign_data_union (env, body);
        else if (assign_lambda_sugar_p (env, body))
            tk_assign_value (env, assign_lambda_desugar (env, body));
        else
            tk_assign_value (env, body);
    }
    bool
    stack_word_p (string word)
    {
        return word == "drop"
            or word == "dup"
            or word == "over"
            or word == "tuck"
            or word == "swap";
    }
        struct lambda_jo_t: jo_t
        {
            name_vector_t name_vector;
            shared_ptr <jojo_t> jojo;
            lambda_jo_t (
                name_vector_t name_vector,
                shared_ptr <jojo_t> jojo);
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        lambda_jo_t::
        lambda_jo_t (
            name_vector_t name_vector,
            shared_ptr <jojo_t> jojo)
        {
            this->name_vector = name_vector;
            this->jojo = jojo;
        }
        jo_t *
        lambda_jo_t::copy ()
        {
            return new lambda_jo_t (this->name_vector, this->jojo);
        }
        void
        lambda_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            auto closure = make_closure (
                env,
                this->name_vector,
                this->jojo,
                bind_vector_from_name_vector (this->name_vector),
                local_scope);
            env.obj_stack.push (closure);
        }
        string
        lambda_jo_t::repr (env_t &env)
        {
            return "(lambda " +
                name_vector_repr (this->name_vector) +
                " " +
                jojo_repr (env, this->jojo) +
                ")";
        }
      shared_ptr <obj_t>
      body_patch_drop (env_t &env, shared_ptr <obj_t> sexp_list)
      {
          assert (cons_p (env, sexp_list));
          auto head = car (env, sexp_list);
          auto rest = cdr (env, sexp_list);
          if (null_p (env, rest))
              return sexp_list;
          else {
              auto drop = unit_list (env, make_sym (env, "drop"));
              sexp_list = body_patch_drop (env, rest);
              sexp_list = cons_c (env, drop, sexp_list);
              sexp_list = cons_c (env, head, sexp_list);
              return sexp_list;
          }
      }
      name_vector_t
      obj_vector_to_name_vector (env_t &env, obj_vector_t &obj_vect)
      {
          auto name_vector = name_vector_t ();
          for (auto &obj: obj_vect) {
              auto sym = as_sym (obj);
              name_vector.push_back (sym->sym);
          }
          return name_vector;
      }
      shared_ptr <jojo_t>
      k_lambda (
          env_t &env,
          local_ref_map_t &old_local_ref_map,
          shared_ptr <obj_t> body)
      {
          auto name_vect = as_vect (car (env, body));
          auto rest = cdr (env, body);
          auto name_vector = obj_vector_to_name_vector
              (env, name_vect->obj_vector);
          auto local_ref_map = local_ref_map_extend
              (env, old_local_ref_map, name_vector);
          rest = body_patch_drop (env, rest);
          auto rest_jojo = sexp_list_compile
              (env, local_ref_map, rest);
          jo_vector_t jo_vector = {
              new lambda_jo_t (name_vector, rest_jojo),
          };
          return make_shared <jojo_t> (jo_vector);
      }
        struct macro_maker_jo_t: jo_t
        {
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        jo_t *
        macro_maker_jo_t::copy ()
        {
            return new macro_maker_jo_t ();
        }
        void
        macro_maker_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            auto obj = env.obj_stack.top ();
            env.obj_stack.pop ();
            if (closure_p (env, obj)) {
                auto macro = make_macro (env, obj);
                env.obj_stack.push (macro);
            }
            else {
                cout << "- fatal error : macro_maker_jo_t::exe" << "\n";
                cout << "  can only make macro from closure" << "\n";
                exit (1);
            }
        }
        string
        macro_maker_jo_t::repr (env_t &env)
        {
            return "(macro_maker)";
        }
      shared_ptr <jojo_t>
      k_macro (
          env_t &env,
          local_ref_map_t &old_local_ref_map,
          shared_ptr <obj_t> body)
      {
          auto name_vect = as_vect (car (env, body));
          auto rest = cdr (env, body);
          auto name_vector = obj_vector_to_name_vector
              (env, name_vect->obj_vector);
          auto local_ref_map = local_ref_map_extend
              (env, old_local_ref_map, name_vector);
          rest = body_patch_drop (env, rest);
          auto rest_jojo = sexp_list_compile
              (env, local_ref_map, rest);
          jo_vector_t jo_vector = {
              new lambda_jo_t (name_vector, rest_jojo),
              new macro_maker_jo_t (),
          };
          return make_shared <jojo_t> (jo_vector);
      }
        struct case_jo_t: jo_t
        {
            jojo_map_t jojo_map;
            shared_ptr <jojo_t> default_jojo;
            case_jo_t (jojo_map_t jojo_map);
            case_jo_t (
                jojo_map_t jojo_map,
                shared_ptr <jojo_t> default_jojo);
            bool has_default_jojo_p ();
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        case_jo_t::
        case_jo_t (jojo_map_t jojo_map)
        {
            this->jojo_map = jojo_map;
            this->default_jojo = nullptr;
        }

        case_jo_t::
        case_jo_t (jojo_map_t jojo_map,
                   shared_ptr <jojo_t> default_jojo)
        {
            this->jojo_map = jojo_map;
            this->default_jojo = default_jojo;
        }
        bool
        case_jo_t::has_default_jojo_p ()
        {
            return this->default_jojo != nullptr;
        }
        jo_t *
        case_jo_t::copy ()
        {
            return new case_jo_t
                (this->jojo_map,
                 this->default_jojo);
        }
        void
        case_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            auto obj = env.obj_stack.top ();
            env.obj_stack.pop ();
            auto it = this->jojo_map.find (obj->tag);
            if (it != this->jojo_map.end ()) {
                auto jojo = it->second;
                auto frame = make_shared <frame_t> (jojo, local_scope);
                env.frame_stack.push (frame);
            }
            else if (this->has_default_jojo_p ()) {
                auto jojo = this->default_jojo;
                auto frame = make_shared <frame_t> (jojo, local_scope);
                env.frame_stack.push (frame);
            }
            else {
                cout << "- fatal error : case_jo_t::exe mismatch" << "\n";
                cout << "  tag : " << obj->tag << "\n";
                exit (1);
            }
        }
        string
        case_jo_t::repr (env_t &env)
        {
            return "(case)";
        }
      shared_ptr <jojo_t>
      case_compile (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> body)
      {
          auto jojo_map = jojo_map_t ();
          shared_ptr <jojo_t> default_jojo = nullptr;
          while (! null_p (env, body)) {
              auto one = car (env, body);
              auto head = as_sym (car (env, one));
              auto rest = cdr (env, one);
              auto name = head->sym;
              if (name == "_") {
                  auto jojo = sexp_list_compile (env, local_ref_map, rest);
                  body = cdr (env, body);
                  default_jojo = jojo;
              }
              else {
                  auto tag = tagging (env, name);
                  auto jojo = sexp_list_compile (env, local_ref_map, rest);
                  jojo_map [tag] = jojo;
                  body = cdr (env, body);
              }
          }
          jo_vector_t jo_vector = {
              new case_jo_t (jojo_map, default_jojo),
          };
          return make_shared <jojo_t> (jo_vector);
      }
      shared_ptr <jojo_t>
      k_case (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> body)
      {
          auto head = car (env, body);
          auto rest = cdr (env, body);
          auto head_jojo = sexp_compile (env, local_ref_map, head);
          auto rest_jojo = case_compile (env, local_ref_map, rest);
          return jojo_append (head_jojo, rest_jojo);
      }
      shared_ptr <jojo_t>
      sexp_qoute_compile (
          env_t &env,
          shared_ptr <obj_t> sexp)
      {
          jo_vector_t jo_vector = {
              new lit_jo_t (sexp),
          };
          auto jojo = make_shared <jojo_t> (jo_vector);
          return jojo;
      }
      shared_ptr <jojo_t>
      k_quote (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> body)
      {
          assert (cons_p (env, body));
          assert (null_p (env, cdr (env, body)));
          auto sexp = car (env, body);
          return sexp_qoute_compile (env, sexp);
      }
        struct collect_list_jo_t: jo_t
        {
            size_t counter;
            collect_list_jo_t (size_t counter);
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        collect_list_jo_t::
        collect_list_jo_t (size_t counter)
        {
            this->counter = counter;
        }
        jo_t *
        collect_list_jo_t::copy ()
        {
            return new collect_list_jo_t (this->counter);
        }
        void
        collect_list_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            size_t index = 0;
            auto collection = null_c (env);
            while (index < this->counter) {
                auto obj = env.obj_stack.top ();
                env.obj_stack.pop ();
                collection = cons_c (env, obj, collection);
                index++;
            }
            env.obj_stack.push (collection);
        }
        string
        collect_list_jo_t::repr (env_t &env)
        {
            return "(collect-list " + to_string (this->counter) + ")";
        }
      shared_ptr <jojo_t>
      k_list (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> body)
      {
          auto sexp_list = body;
          auto jojo = sexp_list_compile
              (env, local_ref_map, sexp_list);
          auto counter = list_size (env, sexp_list);
          jo_vector_t jo_vector = {
              new collect_list_jo_t (counter),
          };
          auto ending_jojo = make_shared <jojo_t> (jo_vector);
          return jojo_append (jojo, ending_jojo);
      }
      shared_ptr <jojo_t>
      k_note (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> body)
      {
          body = cons_c (env, make_sym (env, "note"),
                         body);
          jo_vector_t jo_vector = {
              new lit_jo_t (body),
          };
          auto jojo = make_shared <jojo_t> (jo_vector);
          return jojo;
      }
        struct assert_jo_t: jo_t
        {
            shared_ptr <obj_t> body;
            shared_ptr <jojo_t> jojo;
            assert_jo_t (
                shared_ptr <obj_t> body,
                shared_ptr <jojo_t> jojo);
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        assert_jo_t::
        assert_jo_t (
            shared_ptr <obj_t> body,
            shared_ptr <jojo_t> jojo)
        {
            this->body = body;
            this->jojo = jojo;
        }
        jo_t *
        assert_jo_t::copy ()
        {
            return new assert_jo_t
                (this->body,
                 this->jojo);
        }
        void
        assert_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            auto base = env.frame_stack.size ();
            env.frame_stack.push (
                make_shared <frame_t> (
                    this->jojo,
                    local_scope));
            env.run_with_base (base);
            auto result = env.obj_stack.top ();
            if (true_p (env, result)) {
                return;
            }
            else {
                env.frame_stack_report ();
                env.obj_stack_report ();
                cout << "- assert fail : " << "\n";
                cout << "  " << sexp_list_repr (env, this->body) << "\n";
                exit (1);
            }
        }
        string
        assert_jo_t::repr (env_t &env)
        {
            return "(assert)";
        }
      shared_ptr <jojo_t>
      k_assert (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> body)
      {
          auto jojo = sexp_list_compile (env, local_ref_map, body);
          jo_vector_t jo_vector = {
              new assert_jo_t (body, jojo),
          };
          return make_shared <jojo_t> (jo_vector);
      }
        struct if_jo_t: jo_t
        {
            shared_ptr <jojo_t> pred_jojo;
            shared_ptr <jojo_t> then_jojo;
            shared_ptr <jojo_t> else_jojo;
            if_jo_t (
                shared_ptr <jojo_t> pred_jojo,
                shared_ptr <jojo_t> then_jojo,
                shared_ptr <jojo_t> else_jojo);
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        if_jo_t::
        if_jo_t (
            shared_ptr <jojo_t> pred_jojo,
            shared_ptr <jojo_t> then_jojo,
            shared_ptr <jojo_t> else_jojo)
        {
            this->pred_jojo = pred_jojo;
            this->then_jojo = then_jojo;
            this->else_jojo = else_jojo;
        }
        jo_t *
        if_jo_t::copy ()
        {
            return new if_jo_t
                (this->pred_jojo,
                 this->then_jojo,
                 this->else_jojo);
        }
        void
        if_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            auto result = jojo_eval (env, local_scope, pred_jojo);
            if (true_p (env, result)) {
                env.frame_stack.push (
                    make_shared <frame_t> (
                        this->then_jojo,
                        local_scope));
            }
            else if (false_p (env, result)) {
                env.frame_stack.push (
                    make_shared <frame_t> (
                        this->else_jojo,
                        local_scope));
            }
            else {
                cout << "- fatal error : if_jo_t::exe" << "\n";
                cout << "  pred_jojo run to non bool value" << "\n";
                exit (1);
            }
        }
        string
        if_jo_t::repr (env_t &env)
        {
            return "(if)";
        }
      shared_ptr <jojo_t>
      k_if (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> body)
      {
          auto size = list_size (env, body);
          assert (size == 3);
          auto pred_sexp = car (env, body);
          auto then_sexp = car (env, cdr (env, body));
          auto else_sexp = car (env, cdr (env, cdr (env, body)));
          auto pred_jojo = sexp_compile (env, local_ref_map, pred_sexp);
          auto then_jojo = sexp_compile (env, local_ref_map, then_sexp);
          auto else_jojo = sexp_compile (env, local_ref_map, else_sexp);
          jo_vector_t jo_vector = {
              new if_jo_t (pred_jojo, then_jojo, else_jojo),
          };
          return make_shared <jojo_t> (jo_vector);
      }
        struct when_jo_t: jo_t
        {
            shared_ptr <jojo_t> pred_jojo;
            shared_ptr <jojo_t> then_jojo;
            when_jo_t (
                shared_ptr <jojo_t> pred_jojo,
                shared_ptr <jojo_t> then_jojo);
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        when_jo_t::
        when_jo_t (
            shared_ptr <jojo_t> pred_jojo,
            shared_ptr <jojo_t> then_jojo)
        {
            this->pred_jojo = pred_jojo;
            this->then_jojo = then_jojo;
        }
        jo_t *
        when_jo_t::copy ()
        {
            return new when_jo_t
                (this->pred_jojo,
                 this->then_jojo);
        }
        void
        when_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            auto result = jojo_eval (env, local_scope, pred_jojo);
            if (true_p (env, result)) {
                env.frame_stack.push (
                    make_shared <frame_t> (
                        this->then_jojo,
                        local_scope));
            }
            else if (false_p (env, result)) {
                env.obj_stack.push (result);
            }
            else {
                cout << "- fatal error : when_jo_t::exe" << "\n";
                cout << "  pred_jojo run to non bool value" << "\n";
                exit (1);
            }
        }
        string
        when_jo_t::repr (env_t &env)
        {
            return "(when)";
        }
      shared_ptr <jojo_t>
      k_when (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> body)
      {
          auto size = list_size (env, body);
          assert (size == 2);
          auto pred_sexp = car (env, body);
          auto then_sexp = car (env, cdr (env, body));
          auto pred_jojo = sexp_compile (env, local_ref_map, pred_sexp);
          auto then_jojo = sexp_compile (env, local_ref_map, then_sexp);
          jo_vector_t jo_vector = {
              new when_jo_t (pred_jojo, then_jojo),
          };
          return make_shared <jojo_t> (jo_vector);
      }
      shared_ptr <jojo_t>
      k_begin (
          env_t &env,
          local_ref_map_t &local_ref_map,
          shared_ptr <obj_t> body)
      {
          body = body_patch_drop (env, body);
          return sexp_list_compile (env, local_ref_map, body);
      }
      void
      m_let (env_t &env, obj_map_t &obj_map)
      {
          auto body = obj_map ["body"];
          auto head = car (env, body);
          auto rest = cdr (env, body);
          auto binding_vect = as_vect (head);
          binding_vect = vect_reverse (env, binding_vect);
          rest = cons_c (env, make_sym (env, "begin"), rest);
          for (auto binding: binding_vect->obj_vector) {
              auto name = car (env, binding);
              auto obj = car (env, cdr (env, binding));
              rest = unit_list (env, rest);
              rest = cons_c (env, unit_vect (env, name), rest);
              rest = cons_c (env, make_sym (env, "lambda"), rest);
              rest = cons_c (env, rest, unit_list (env, obj));
          }
          env.obj_stack.push (rest);
      }
      shared_ptr <obj_t>
      sexp_list_quote_and_unquote (
          env_t &env,
          shared_ptr <obj_t> sexp_list);

      shared_ptr <obj_t>
      sexp_quote_and_unquote (
          env_t &env,
          shared_ptr <obj_t> sexp)
      {
          if (str_p (env, sexp) or num_p (env, sexp)) {
              return sexp;
          }
          else if (sym_p (env, sexp)) {
              return cons_c (
                  env,
                  make_sym (env, "quote"),
                  unit_list (env, sexp));
          }
          else if (null_p (env, sexp)) {
              return cons_c (
                  env,
                  make_sym (env, "quote"),
                  unit_list (env, sexp));
          }
          else if (vect_p (env, sexp)) {
              auto l = vect_to_list (env, as_vect (sexp));
              return cons_c (
                  env,
                  make_sym (env, "list-to-vect"),
                  unit_list (env, sexp_list_quote_and_unquote (env, l)));
          }
          else if (dict_p (env, sexp)) {
              auto l = dict_to_list (env, as_dict (sexp));
              return cons_c (
                  env,
                  make_sym (env, "list-to-dict"),
                  unit_list (env, sexp_list_quote_and_unquote (env, l)));
          }
          else {
              assert (cons_p (env, sexp));
              auto head = car (env, sexp);
              if (sym_p (env, head) and
                  as_sym (head) ->sym == "unquote")
              {
                  auto rest = cdr (env, sexp);
                  assert (cons_p (env, rest));
                  assert (null_p (env, cdr (env, rest)));
                  return car (env, rest);
              }
              else {
                  return sexp_list_quote_and_unquote (
                      env,
                      sexp);
              }
          }
      }
      shared_ptr <obj_t>
      sexp_list_quote_and_unquote (
          env_t &env,
          shared_ptr <obj_t> sexp_list)
      {
          if (null_p (env, sexp_list)) {
              return unit_list (env, make_sym (env, "*"));
          }
          else {
              assert (cons_p (env, sexp_list));
              auto sexp = car (env, sexp_list);
              if (cons_p (env, sexp) and
                  sym_p (env, car (env, sexp)) and
                  as_sym (car (env, sexp)) ->sym == "unquote-splicing")
              {
                  auto rest = cdr (env, sexp);
                  assert (cons_p (env, rest));
                  assert (null_p (env, cdr (env, rest)));
                  sexp = car (env, rest);
              }
              else {
                  sexp = cons_c (
                      env,
                      make_sym (env, "*"),
                      unit_list (env, sexp_quote_and_unquote (env, sexp)));
              }
              auto result = sexp_list_quote_and_unquote (
                  env, cdr (env, sexp_list));
              result = unit_list (env, result);
              result = cons_c (env, sexp, result);
              result = cons_c (env, make_sym (env, "list-append"), result);
              return result;
          }
      }
      void
      m_quasiquote (
          env_t &env,
          obj_map_t &obj_map)
      {
          auto body = obj_map ["body"];
          assert (cons_p (env, body));
          assert (null_p (env, cdr (env, body)));
          auto sexp = car (env, body);
          auto new_sexp = sexp_quote_and_unquote (env, sexp);
          env.obj_stack.push (new_sexp);
      }
      shared_ptr <obj_t>
      sexp_list_and (env_t &env, shared_ptr <obj_t> sexp_list)
      {
          if (null_p (env, sexp_list)) {
              return make_sym (env, "true-c");
          }
          else if (null_p (env, cdr (env, sexp_list))) {
              return car (env, sexp_list);
          }
          else {
              auto head = car (env, sexp_list);
              auto rest = cdr (env, sexp_list);
              head = cons_c (
                  env,
                  make_sym (env, "not"),
                  unit_list (env, head));
              auto result = unit_list (env, sexp_list_and (env, rest));
              result = cons_c (
                  env,
                  make_sym (env, "false-c"),
                  result);
              result = cons_c (
                  env,
                  head,
                  result);
              result = cons_c (
                  env,
                  make_sym (env, "if"),
                  result);
              return result;
          }
      }
      void
      m_and (env_t &env, obj_map_t &obj_map)
      {
          auto body = obj_map ["body"];
          env.obj_stack.push (sexp_list_and (env, body));
      }
      shared_ptr <obj_t>
      sexp_list_or (env_t &env, shared_ptr <obj_t> sexp_list)
      {
          if (null_p (env, sexp_list)) {
              return make_sym (env, "false-c");
          }
          else if (null_p (env, cdr (env, sexp_list))) {
              return car (env, sexp_list);
          }
          else {
              auto head = car (env, sexp_list);
              auto rest = cdr (env, sexp_list);
              auto result = unit_list (env, sexp_list_or (env, rest));
              result = cons_c (
                  env,
                  make_sym (env, "true-c"),
                  result);
              result = cons_c (
                  env,
                  head,
                  result);
              result = cons_c (
                  env,
                  make_sym (env, "if"),
                  result);
              return result;
          }
      }
      void
      m_or (env_t &env, obj_map_t &obj_map)
      {
          auto body = obj_map ["body"];
          env.obj_stack.push (sexp_list_or (env, body));
      }
      shared_ptr <obj_t>
      vect_list_cond (env_t &env, shared_ptr <obj_t> vect_list)
      {
          assert (! null_p (env, vect_list));
          auto head = car (env, vect_list);
          auto rest = cdr (env, vect_list);
          auto l = vect_to_list (env, as_vect (head));
          auto question = car (env, l);
          auto answer = cons_c (
              env,
              make_sym (env, "begin"),
              cdr (env, l));
          if (null_p (env, rest)) {
              if (sym_p (env, question) and
                  as_sym (question) ->sym == "else")
              {
                  return answer;
              }
              else {
                  auto result = null_c (env);
                  result = cons_c (env, answer, result);
                  result = cons_c (env, question, result);
                  result = cons_c (env, make_sym (env, "when"), result);
                  return result;
              }
          }
          else {
              auto result = unit_list (env, vect_list_cond (env, rest));
              result = cons_c (env, answer, result);
              result = cons_c (env, question, result);
              result = cons_c (env, make_sym (env, "if"), result);
              return result;
          }
      }
      void
      m_cond (env_t &env, obj_map_t &obj_map)
      {
          auto body = obj_map ["body"];
          env.obj_stack.push (vect_list_cond (env, body));
      }

    void
    def_type (env_t &env, name_t name)
    {
        define_type (env, name);
        define_data_pred (env, name_t2p (name), tagging (env, name));
    }
    void
    expose_type (env_t &env)
    {
        define_prim (
            env, { "type-of", "obj" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                auto obj = obj_map ["obj"];
                env.obj_stack.push (type_of (env, obj));
            });
    }
    void
    expose_bool (env_t &env)
    {
        define (env, "true-c", true_c (env));
        define (env, "true", true_c (env));
        define (env, "false-c", false_c (env));
        define (env, "false", false_c (env));
        define_prim (
            env, { "not", "bool" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                auto obj = obj_map ["bool"];
                assert (bool_p (env, obj));
                env.obj_stack.push (make_bool (env, false_p (env, obj)));
            });
    }
      void
      expose_num_predicate (env_t &env)
      {
          define_prim (
              env, { "lt", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push (make_bool (env, x->num < y->num));
              });
          define_prim (
              env, { "gt", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push (make_bool (env, x->num > y->num));
              });
          define_prim (
              env, { "lteq", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push (make_bool (env, x->num <= y->num));
              });
          define_prim (
              env, { "gteq", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push (make_bool (env, x->num >= y->num));
              });
      }
      void
      expose_num_1 (env_t &env)
      {
          define_prim (
              env, { "add", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push
                      (make_num
                       (env, x->num + y->num));
              });
          define_prim (
              env, { "sub", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push
                      (make_num
                       (env, x->num - y->num));
              });
          define_prim (
              env, { "max", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push
                      (make_num
                       (env, fmax (x->num, y->num)));
              });
          define_prim (
              env, { "min", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push
                      (make_num
                       (env, fmin (x->num, y->num)));
              });
          define_prim (
              env, { "neg", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, - x->num));
              });
          define_prim (
              env, { "abs", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, fabs (x->num)));
              });
      }
      void
      expose_num_2 (env_t &env)
      {
          define_prim (
              env, { "mul", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push
                      (make_num
                       (env, x->num * y->num));
              });
          define_prim (
              env, { "div", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push
                      (make_num
                       (env, x->num / y->num));
              });
          define_prim (
              env, { "mod", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push
                      (make_num
                       (env, fmod (x->num, y->num)));
              });
          define_prim (
              env, { "hypot", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push
                      (make_num
                       (env, hypot (x->num, y->num)));
              });
          define_prim (
              env, { "sqrt", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, sqrt (x->num)));
              });
          define_prim (
              env, { "cbrt", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, cbrt (x->num)));
              });
      }
      void
      expose_num_3 (env_t &env)
      {
          define_prim (
              env, { "pow", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push
                      (make_num
                       (env, pow (x->num, y->num)));
              });
          define_prim (
              env, { "expe", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, exp (x->num)));
              });
          define_prim (
              env, { "exp2", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, exp2 (x->num)));
              });
          define_prim (
              env, { "expm1", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, expm1 (x->num)));
              });
          define_prim (
              env, { "loge", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, log (x->num)));
              });
          define_prim (
              env, { "log10", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, log10 (x->num)));
              });
          define_prim (
              env, { "log2", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, log2 (x->num)));
              });
          define_prim (
              env, { "log1p", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, log1p (x->num)));
              });
      }
      void
      expose_num_trigonometry (env_t &env)
      {
          define_prim (
              env, { "sin", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, sin (x->num)));
              });
          define_prim (
              env, { "cos", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, cos (x->num)));
              });
          define_prim (
              env, { "tan", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, tan (x->num)));
              });
          define_prim (
              env, { "asin", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, asin (x->num)));
              });
          define_prim (
              env, { "acos", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, acos (x->num)));
              });
          define_prim (
              env, { "atan", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, atan (x->num)));
              });
          define_prim (
              env, { "atan2", "x", "y" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  auto y = as_num (obj_map ["y"]);
                  env.obj_stack.push
                      (make_num
                       (env, atan2 (x->num, y->num)));
              });
          define_prim (
              env, { "sinh", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, sinh (x->num)));
              });
          define_prim (
              env, { "cosh", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, cosh (x->num)));
              });
          define_prim (
              env, { "tanh", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, tanh (x->num)));
              });
          define_prim (
              env, { "asinh", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, asinh (x->num)));
              });
          define_prim (
              env, { "acosh", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, acosh (x->num)));
              });
          define_prim (
              env, { "atanh", "x" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  auto x = as_num (obj_map ["x"]);
                  env.obj_stack.push
                      (make_num
                       (env, atanh (x->num)));
              });
      }
      void
      expose_num (env_t &env)
      {
          expose_num_predicate (env);
          expose_num_1 (env);
          expose_num_2 (env);
          expose_num_3 (env);
          expose_num_trigonometry (env);
      }
    void
    expose_str (env_t &env)
    {
        define_prim (
            env, { "str-print", "str" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                auto obj = obj_map ["str"];
                auto str = as_str (obj);
                cout << str->str;
                env.obj_stack.push (str);
            });
        define_prim (
            env, { "str-length", "str" },
            [] (env_t &env, obj_map_t &obj_map)
            {
            env.obj_stack.push (
                str_length (
                    env,
                    as_str (obj_map ["str"])));
            });
        define_prim (
            env, { "str-append", "ante", "succ" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    str_append (
                        env,
                        as_str (obj_map ["ante"]),
                        as_str (obj_map ["succ"])));
            });
        define_prim (
            env, { "str-slice", "str", "begin", "end" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    str_slice (
                        env,
                        as_str (obj_map ["str"]),
                        as_num (obj_map ["begin"]),
                        as_num (obj_map ["end"])));
            });
        define_prim (
            env, { "str-ref", "str", "index" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    str_ref (
                        env,
                        as_str (obj_map ["str"]),
                        as_num (obj_map ["index"])));
            });
        define_prim (
            env, { "str-head", "str" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    str_head (
                        env,
                        as_str (obj_map ["str"])));
            });
        define_prim (
            env, { "str-rest", "str" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    str_rest (
                        env,
                        as_str (obj_map ["str"])));
            });
    }
    void
    expose_sym (env_t &env)
    {
        define_prim (
            env, { "sym-print", "sym" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                auto obj = obj_map ["sym"];
                assert (sym_p (env, obj));
                auto sym = as_sym (obj);
                cout << sym->sym;
                env.obj_stack.push (sym);
            });
        define_prim (
            env, { "sym-length", "sym" },
            [] (env_t &env, obj_map_t &obj_map)
            {
            env.obj_stack.push (
                sym_length (
                    env,
                    as_sym (obj_map ["sym"])));
            });
        define_prim (
            env, { "sym-append", "ante", "succ" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    sym_append (
                        env,
                        as_sym (obj_map ["ante"]),
                        as_sym (obj_map ["succ"])));
            });
        define_prim (
            env, { "sym-slice", "sym", "begin", "end" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    sym_slice (
                        env,
                        as_sym (obj_map ["sym"]),
                        as_num (obj_map ["begin"]),
                        as_num (obj_map ["end"])));
            });
        define_prim (
            env, { "sym-ref", "sym", "index" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    sym_ref (
                        env,
                        as_sym (obj_map ["sym"]),
                        as_num (obj_map ["index"])));
            });
        define_prim (
            env, { "sym-head", "sym" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    sym_head (
                        env,
                        as_sym (obj_map ["sym"])));
            });
        define_prim (
            env, { "sym-rest", "sym" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    sym_rest (
                        env,
                        as_sym (obj_map ["sym"])));
            });
    }
      shared_ptr <data_cons_o>
      jj_cons_c (env_t &env)
      {
          return make_shared <data_cons_o> (
              env,
              cons_tag,
              name_vector_t ({ "car", "cdr" }),
              obj_map_t ());
      }
      void
      expose_list (env_t &env)
      {
          define (env, "null-c", null_c (env));
          define (env, "null", null_c (env));
          define (env, "cons-c", jj_cons_c (env));
          define (env, "cons", jj_cons_c (env));
          define_prim (
              env, { "list-length", "list" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  env.obj_stack.push (
                      list_length (
                          env,
                          obj_map ["list"]));
              });
          define_prim (
              env, { "list-reverse", "list" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  env.obj_stack.push (
                      list_reverse (
                          env,
                          obj_map ["list"]));
              });
          define_prim (
              env, { "list-append", "ante", "succ" },
              [] (env_t &env, obj_map_t &obj_map)
              {
                  env.obj_stack.push (
                      list_append (
                          env,
                          obj_map ["ante"],
                          obj_map ["succ"]));
              });
      }
    void
    expose_vect (env_t &env)
    {
        define_prim (
            env, { "list-to-vect", "list" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    list_to_vect (
                        env,
                        obj_map ["list"]));
            });
        define_prim (
            env, { "vect-to-list", "vect" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    vect_to_list (
                        env,
                        as_vect (obj_map ["vect"])));
            });
        define_prim (
            env, { "vect-length", "vect" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    vect_length (
                        env,
                        as_vect (obj_map ["vect"])));
            });
        define_prim (
            env, { "vect-append", "ante", "succ" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    vect_append (
                        env,
                        as_vect (obj_map ["ante"]),
                        as_vect (obj_map ["succ"])));
            });
        define_prim (
            env, { "vect-slice", "vect", "begin", "end" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    vect_slice (
                        env,
                        as_vect (obj_map ["vect"]),
                        as_num (obj_map ["begin"]),
                        as_num (obj_map ["end"])));
            });
        define_prim (
            env, { "vect-ref", "vect", "index" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    vect_ref (
                        env,
                        as_vect (obj_map ["vect"]),
                        as_num (obj_map ["index"])));
            });
        define_prim (
            env, { "vect-head", "vect" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    vect_head (
                        env,
                        as_vect (obj_map ["vect"])));
            });
        define_prim (
            env, { "vect-rest", "vect" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    vect_rest (
                        env,
                        as_vect (obj_map ["vect"])));
            });
        define_prim (
            env, { "unit-vect", "obj" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    unit_vect (
                        env,
                        obj_map ["obj"]));
            });
        define_prim (
            env, { "vect-reverse", "vect" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    vect_reverse (
                        env,
                        as_vect (obj_map ["vect"])));
            });
    }
      shared_ptr <data_cons_o>
      jj_just_c (env_t &env)
      {
          return make_shared <data_cons_o> (
              env,
              just_tag,
              name_vector_t ({ "value" }),
              obj_map_t ());
      }
      void
      expose_maybe (env_t &env)
      {
          define (env, "nothing-c", nothing_c (env));
          define (env, "nothing", nothing_c (env));
          define (env, "just-c", jj_just_c (env));
          define (env, "just", jj_just_c (env));
      }
    void
    expose_dict (env_t &env)
    {
        define_prim (
            env, { "list-to-dict", "list" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    list_to_dict (
                        env,
                        obj_map ["list"]));
            });
        define_prim (
            env, { "dict-to-list", "dict" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    dict_to_list (
                        env,
                        as_dict (obj_map ["dict"])));
            });
        define_prim (
            env, { "dict-length", "dict" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    dict_length (
                        env,
                        as_dict (obj_map ["dict"])));
            });
        define_prim (
            env, { "dict-key-list", "dict" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    dict_key_list (
                        env,
                        as_dict (obj_map ["dict"])));
            });
        define_prim (
            env, { "dict-value-list", "dict" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    dict_value_list (
                        env,
                        as_dict (obj_map ["dict"])));
            });
        define_prim (
            env, { "dict-insert", "dict", "key", "value" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    dict_insert (
                        env,
                        as_dict (obj_map ["dict"]),
                        as_sym (obj_map ["key"]),
                        obj_map ["value"]));
            });
        define_prim (
            env, { "dict-merge", "ante", "succ" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    dict_merge (
                        env,
                        as_dict (obj_map ["ante"]),
                        as_dict (obj_map ["succ"])));
            });
        define_prim (
            env, { "dict-find", "dict", "key" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    dict_find (
                        env,
                        as_dict (obj_map ["dict"]),
                        as_sym (obj_map ["key"])));
            });
    }
    void
    expose_sexp (env_t &env)
    {
        define_prim (
            env, { "scan-word-list", "code" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                // -- str-t -> (list-t str-t)
                env.obj_stack.push (
                    scan_word_list (
                        env,
                        as_str (obj_map ["code"])));
            });
        define_prim (
            env, { "parse-sexp", "word-list" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                // -- (list-t str-t) -> sexp-t
                env.obj_stack.push (
                    parse_sexp (
                        env,
                        obj_map ["word-list"]));
            });
        define_prim (
            env, { "parse-sexp-list", "word-list" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                // -- (list-t str-t) -> (list-t sexp-t)
                env.obj_stack.push (
                    parse_sexp_list (
                        env,
                        obj_map ["word-list"]));
            });
        define_prim (
            env, { "sexp-repr", "sexp" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    make_str (
                        env,
                        sexp_repr (env, obj_map ["sexp"])));
            });
        define_prim (
            env, { "sexp-list-repr", "sexp-list" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.push (
                    make_str (
                        env,
                        sexp_list_repr (env, obj_map ["sexp-list"])));
            });
    }
    void
    expose_top_keyword (env_t &env)
    {
    }
    void
    expose_keyword (env_t &env)
    {
    }
    void
    expose_system (env_t &env)
    {
        define_prim (
            env, { "system-env-find", "name" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                // -- str-t -> str-t
                auto str = as_str (obj_map ["name"]);
                auto name = str->str;
                env.obj_stack.push (make_str (env, system_env_find (name)));
            });
    }
      sig_t jj_import_sig = { "import", "module-path" };
      // -- str-t -> module-t
      void jj_import (env_t &env, obj_map_t &obj_map)
      {
          auto obj = obj_map ["module-path"];
          auto str = as_str (obj);
          auto module_path = path_t (str->str);
          module_path = respect_module_path
              (env, module_path);
          auto module_env = env_from_module_path (module_path);
          auto mod = make_shared <module_o> (env, module_env);
          env.obj_stack.push (mod);
      }
      void
      expose_module (env_t &env)
      {
          define_prim (env,
                       jj_import_sig,
                       jj_import);
      }
    void
    expose_stack (env_t &env)
    {
        define_prim (
            env, { "drop" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack.pop ();
            });
        define_prim (
            env, { "dup" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                auto obj = env.obj_stack.top ();
                env.obj_stack.push (obj);
            });
        define_prim (
            env, { "over" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                auto b = env.obj_stack.top ();
                env.obj_stack.pop ();
                auto a = env.obj_stack.top ();
                env.obj_stack.pop ();
                env.obj_stack.push (a);
                env.obj_stack.push (b);
                env.obj_stack.push (a);
            });
        define_prim (
            env, { "tuck" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                auto b = env.obj_stack.top ();
                env.obj_stack.pop ();
                auto a = env.obj_stack.top ();
                env.obj_stack.pop ();
                env.obj_stack.push (b);
                env.obj_stack.push (a);
                env.obj_stack.push (b);
            });
        define_prim (
            env, { "swap" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                auto b = env.obj_stack.top ();
                env.obj_stack.pop ();
                auto a = env.obj_stack.top ();
                env.obj_stack.pop ();
                env.obj_stack.push (b);
                env.obj_stack.push (a);
            });
    }
    void
    expose_syntax (env_t &env)
    {
        define_top_keyword (env, "=", tk_assign);
        define_keyword (env, "lambda", k_lambda);
        define_keyword (env, "macro", k_macro);
        define_keyword (env, "case", k_case);
        define_keyword (env, "quote", k_quote);
        define_keyword (env, "*", k_list);
        define_keyword (env, "note", k_note);
        define_keyword (env, "assert", k_assert);
        define_keyword (env, "if", k_if);
        define_keyword (env, "when", k_when);
        define_keyword (env, "begin", k_begin);
        define_prim_macro (env, "let", m_let);
        define_prim_macro (env, "quasiquote", m_quasiquote);
        define_prim_macro (env, "and", m_and);
        define_prim_macro (env, "or", m_or);
        define_prim_macro (env, "cond", m_cond);
    }
    void
    expose_misc (env_t &env)
    {
        define_prim (
            env, { "repr", "obj" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                auto obj = obj_map ["obj"];
                env.obj_stack.push (
                    make_str (
                        env,
                        obj->repr (env)));
            });
        define_prim (
            env, { "print", "obj" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                auto obj = obj_map ["obj"];
                cout << obj->repr (env) << flush;
                env.obj_stack.push (obj);
            });
        define_prim (
            env, { "println", "obj" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                auto obj = obj_map ["obj"];
                cout << obj->repr (env) << "\n" << flush;
                env.obj_stack.push (obj);
            });
        define_prim (
            env, { "nl" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                cout << "\n" << flush;
                auto nl = make_str (env, "\n");
                env.obj_stack.push (nl);
            });
        define_prim (
            env, { "eq", "lhs", "rhs" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                auto lhs = obj_map ["lhs"];
                auto rhs = obj_map ["rhs"];
                env.obj_stack.push (
                    make_bool (
                        env,
                        obj_eq (env, lhs, rhs)));
            });
        define_prim (
            env, { "env-report" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.report ();
                env.obj_stack.push (true_c (env));
            });
        define_prim (
            env, { "env-stack-report" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.obj_stack_report ();
                env.obj_stack.push (true_c (env));
            });
        define_prim (
            env, { "env-frame-report" },
            [] (env_t &env, obj_map_t &obj_map)
            {
                env.frame_stack_report ();
                env.obj_stack.push (true_c (env));
            });
    }
    void
    expose_core (env_t &env)
    {
        expose_bool (env);
        expose_num (env);
        expose_str (env);
        expose_sym (env);
        expose_list (env);
        expose_vect (env);
        expose_maybe (env);
        expose_dict (env);
        expose_sexp (env);
        expose_top_keyword (env);
        expose_keyword (env);
        expose_system (env);
        expose_module (env);
        expose_syntax (env);
        expose_type (env);
        expose_stack (env);
        expose_misc (env);
    }
      void
      test_step ()
      {
          auto env = env_t ();

          define (env, "s1", make_str (env, "bye"));
          define (env, "s2", make_str (env, "world"));

          jo_vector_t jo_vector = {
              new ref_jo_t (boxing (env, "s1")),
              new ref_jo_t (boxing (env, "s2")),
          };

          env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

          {
              env.run ();
              assert_stack_size (env, 2);
              assert_pop_eq (env, make_str (env, "world"));
              assert_pop_eq (env, make_str (env, "bye"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_data ()
      {
          auto env = env_t ();

          obj_map_t obj_map = {
              {"car", make_str (env, "bye")},
              {"cdr", make_str (env, "world")},
          };

          define (env, "last-cry", make_shared <data_o>
                  (env, cons_tag, obj_map));

          jo_vector_t jo_vector = {
              new ref_jo_t (boxing (env, "last-cry")),
              new field_jo_t ("car"),
              new ref_jo_t (boxing (env, "last-cry")),
              new field_jo_t ("cdr"),
              new ref_jo_t (boxing (env, "last-cry")),
          };

          env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

          {
              env.run ();

              assert_stack_size (env, 3);
              assert_pop_eq (env, make_shared <data_o>
                             (env,
                              cons_tag,
                              obj_map));
              assert_pop_eq (env, make_str (env, "world"));
              assert_pop_eq (env, make_str (env, "bye"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_apply ()
      {
          auto env = env_t ();

          define (env, "s1", make_str (env, "bye"));
          define (env, "s2", make_str (env, "world"));

          jo_vector_t body = {
              new local_ref_jo_t (0, 0),
              new local_ref_jo_t (0, 1),
          };

          jo_vector_t jo_vector = {
              new ref_jo_t (boxing (env, "s1")),
              new ref_jo_t (boxing (env, "s2")),
              new lambda_jo_t ({ "x", "y" },
                               make_shared <jojo_t> (body)),
              new apply_jo_t (2),
          };

          env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

          // env.double_report ();

          {
              env.run ();
              assert_stack_size (env, 2);
              assert_pop_eq (env, make_str (env, "world"));
              assert_pop_eq (env, make_str (env, "bye"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_lambda_curry ()
      {
          auto env = env_t ();

          define (env, "s1", make_str (env, "bye"));
          define (env, "s2", make_str (env, "world"));

          jo_vector_t body = {
              new local_ref_jo_t (0, 0),
              new local_ref_jo_t (0, 1),
          };

          jo_vector_t jo_vector = {
              new ref_jo_t (boxing (env, "s1")),
              new ref_jo_t (boxing (env, "s2")),
              new lambda_jo_t ({ "x", "y" },
                               make_shared <jojo_t> (body)),
              new apply_jo_t (1),
              new apply_jo_t (1),
          };

          env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

          // {
          //     env.report ();
          //     env.step_and_report ();
          //     env.step_and_report ();
          //     env.step_and_report ();
          //     env.step_and_report ();
          //     env.step_and_report ();
          //     env.step_and_report ();
          //     env.step_and_report ();
          // }

          {
              env.run ();
              assert_stack_size (env, 2);
              assert_pop_eq (env, make_str (env, "bye"));
              assert_pop_eq (env, make_str (env, "world"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_data_cons ()
      {
          auto env = env_t ();

          define (env, "s1", make_str (env, "bye"));
          define (env, "s2", make_str (env, "world"));
          define (env, "cons-c", make_shared <data_cons_o>
                  (env,
                   cons_tag,
                   name_vector_t ({ "car", "cdr" }),
                   obj_map_t ()));

          jo_vector_t jo_vector = {
              new ref_jo_t (boxing (env, "s1")),
              new ref_jo_t (boxing (env, "s2")),
              new ref_jo_t (boxing (env, "cons-c")),
              new apply_jo_t (2),
              new field_jo_t ("cdr"),
          };

          env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

          // env.double_report ();

          {
              env.run ();
              assert_stack_size (env, 1);
              assert_pop_eq (env, make_str (env, "world"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_data_cons_curry ()
      {
          auto env = env_t ();

          define (env, "s1", make_str (env, "bye"));
          define (env, "s2", make_str (env, "world"));
          define (env, "cons-c", make_shared <data_cons_o>
                  (env,
                   cons_tag,
                   name_vector_t ({ "car", "cdr" }),
                   obj_map_t ()));

          jo_vector_t jo_vector = {
              new ref_jo_t (boxing (env, "s1")),
              new ref_jo_t (boxing (env, "s2")),
              new ref_jo_t (boxing (env, "cons-c")),
              new apply_jo_t (1),
              new apply_jo_t (1),
              new field_jo_t ("car"),
          };

          env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

          // env.double_report ();

          {
              env.run ();
              assert_stack_size (env, 1);
              assert_pop_eq (env, make_str (env, "world"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_prim ()
      {
          auto env = env_t ();

          define (env, "s1", make_str (env, "bye"));
          define (env, "s2", make_str (env, "world"));

          auto swap =
              [] (env_t &env, obj_map_t &obj_map)
              {
                  env.obj_stack.push (obj_map ["y"]);
                  env.obj_stack.push (obj_map ["x"]);
              };

          define (env, "swap", make_shared <prim_o>
                  (env,
                   name_vector_t { "x", "y" },
                   swap,
                   obj_map_t ()));

          jo_vector_t jo_vector = {
              new ref_jo_t (boxing (env, "s1")),
              new ref_jo_t (boxing (env, "s2")),
              new ref_jo_t (boxing (env, "swap")),
              new apply_jo_t (2),
              new ref_jo_t (boxing (env, "s1")),
              new ref_jo_t (boxing (env, "s2")),
              new ref_jo_t (boxing (env, "swap")),
              new apply_jo_t (1),
              new apply_jo_t (1),
          };

          env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

          // env.double_report ();

          {
              env.run ();
              assert_stack_size (env, 4);
              assert_pop_eq (env, make_str (env, "world"));
              assert_pop_eq (env, make_str (env, "bye"));
              assert_pop_eq (env, make_str (env, "bye"));
              assert_pop_eq (env, make_str (env, "world"));
              assert_stack_size (env, 0);
          }
      }
    void
    test_all ()
    {
        test_step ();
        test_data ();
        test_apply ();
        test_lambda_curry ();
        test_data_cons ();
        test_data_cons_curry ();
        test_prim ();
        test_scan ();
    }
    void
    the_story_begins (string_vector_t arg_vector)
    {
        for (auto module_path_string: arg_vector) {
            auto module_path = path_t (module_path_string);
            env_from_module_path (module_path);
        }
    }
    int
    main (int argc, char **argv)
    {
        test_all ();
        auto arg_vector = string_vector_t ();
        for (auto i = 1; i < argc; i++) {
            arg_vector.push_back (string (argv[i]));
        }
        the_story_begins (arg_vector);
        return 0;
    }
