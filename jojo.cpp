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
    using namespace std;
    struct obj_t;
    struct jo_t;
    struct env_t;
    struct box_t;
    struct jojo_t;
    struct frame_t;
    struct module_t;
    using name_t = string;
    using name_vector_t = vector <name_t>;
    using name_stack_t = stack <name_t>;
    using bind_t = pair <name_t, shared_ptr <obj_t>>;
    using bind_vector_t = vector <bind_t>; // index from end
    using local_scope_t = vector <bind_vector_t>; // index from end
    using jo_vector_t = vector <jo_t *>;
    using tag_t = string;
    using obj_map_t = map <name_t, shared_ptr <obj_t>>;
    using obj_vector_t = vector <shared_ptr <obj_t>>;
    using box_map_t = map <name_t, box_t *>;
    using obj_stack_t = stack <shared_ptr <obj_t>>;
    using frame_stack_t = stack <shared_ptr <frame_t>>;
    using jojo_map_t = map <tag_t, shared_ptr <jojo_t>>;
    using string_vector_t = vector <string> ;
    using local_ref_t = pair <size_t, size_t>;
    using local_ref_map_t = map <name_t, local_ref_t>;
    using module_map_t = map <name_t, shared_ptr <module_t>>;
    struct obj_t
    {
        tag_t tag;
        obj_map_t obj_map;
        virtual ~obj_t ();
        virtual string repr (env_t &env);
        virtual bool equal (env_t &env, shared_ptr <obj_t> obj);
        virtual void apply (env_t &env, size_t arity);
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
        name_stack_t module_name_stack;
        module_map_t module_map;
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
        box_t (shared_ptr <obj_t> obj);
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
    struct module_t
    {
        name_t module_name;
        box_map_t box_map;
        name_vector_t export_name_vector;
        module_t (name_t module_name,
                  box_map_t box_map,
                  name_vector_t export_name_vector);
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
      bind_equal (env_t &env,
                  bind_t &lhs,
                  bind_t &rhs)
      {
          if (lhs.first != rhs.first) return false;
          return lhs.second->equal (env, rhs.second);
      }
      string
      bind_vector_repr (env_t &env, bind_vector_t bind_vector)
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
      bind_vector_insert_obj (bind_vector_t &bind_vector,
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
      bind_vector_merge_obj_vector (bind_vector_t &old_bind_vector,
                                    obj_vector_t &obj_vector)
      {
          auto bind_vector = old_bind_vector;
          for (auto obj: obj_vector)
              bind_vector_insert_obj (bind_vector, obj);
          return bind_vector;
      }
      bind_vector_t
      bind_vector_from_name_vector (name_vector_t &name_vector)
      {
          auto bind_vector = bind_vector_t ();
          auto begin = name_vector.begin ();
          auto end = name_vector.end ();
          for (auto it = begin; it != end; it++)
              bind_vector.push_back (make_pair (*it, nullptr));
          return bind_vector;
      }
      bool
      bind_vector_equal (env_t &env,
                         bind_vector_t &lhs,
                         bind_vector_t &rhs)
      {
          if (lhs.size () != rhs.size ()) return false;
          auto size = lhs.size ();
          auto index = 0;
          while (index < size) {
              if (! bind_equal (env, lhs [index], rhs [index]))
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
      local_scope_equal (env_t &env,
                         local_scope_t &lhs,
                         local_scope_t &rhs)
      {
          if (lhs.size () != rhs.size ()) return false;
          auto size = lhs.size ();
          auto index = 0;
          while (index < size) {
              if (! bind_vector_equal (env, lhs [index], rhs [index]))
                  return false;
              index++;
          }
          return true;
      }
      local_scope_t
      local_scope_extend (local_scope_t old_local_scope,
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
      obj_map_equal (env_t &env, obj_map_t &lhs, obj_map_t &rhs)
      {
          if (lhs.size () != rhs.size ()) return false;
          for (auto &kv: lhs) {
              auto name = kv.first;
              auto it = rhs.find (name);
              if (it == rhs.end ()) return false;
              if (! kv.second->equal (env, it->second)) return false;
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
      name_vector_obj_map_lack (name_vector_t &old_name_vector,
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
      name_vector_obj_map_arity_lack (name_vector_t &old_name_vector,
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
      pick_up_obj_map_and_merge (env_t &env,
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
      name_vector_and_obj_map_repr (env_t &env,
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
      local_ref_map_extend (env_t &env,
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
          auto index = 0;
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
          assert (obj->equal (env, that));
          env.obj_stack.pop ();
      }
      void
      assert_tos_eq (env_t &env, shared_ptr <obj_t> obj)
      {
          auto that = env.obj_stack.top ();
          assert (obj->equal (env, that));
      }
      void
      assert_stack_size (env_t &env, size_t size)
      {
          assert (env.obj_stack.size () == size);
      }
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
            cout << "- fatal error : obj_t::equal" << "\n";
            cout << "  equal is not implemented for  : ";
            cout << obj->tag << "\n";
            exit (1);
        }
    }
    void
    obj_t::apply (env_t &env, size_t arity)
    {
        cout << "- fatal error : applying non applicable object" << "\n";
        exit (1);
    }
    void
    define (env_t &env,
            name_t name,
            shared_ptr <obj_t> obj)
    {
        auto it = env.box_map.find (name);
        if (it != env.box_map.end ()) {
            auto box = it->second;
            box->empty_p = false;
            box->obj = obj;
        }
        else {
            env.box_map [name] = new box_t (obj);
        }
    }
    bool
    obj_equal (env_t &env,
               shared_ptr <obj_t> &lhs,
               shared_ptr <obj_t> &rhs)
    {
        return lhs->equal (env, rhs);
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

      box_t::box_t (shared_ptr <obj_t> obj)
      {
          this->empty_p = false;
          this->obj = obj;
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
      jojo_append (shared_ptr <jojo_t> ante,
                   shared_ptr <jojo_t> succ)
      {
          auto jo_vector = jo_vector_t ();
          for (auto x: ante->jo_vector) jo_vector.push_back (x->copy ());
          for (auto x: succ->jo_vector) jo_vector.push_back (x->copy ());
          return make_shared <jojo_t> (jo_vector);
      }
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
      frame_t::
      frame_t (shared_ptr <jojo_t> jojo,
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
      module_t::
      module_t (name_t module_name,
                box_map_t box_map,
                name_vector_t export_name_vector)
      {
          this->module_name = module_name;
          this->box_map = box_map;
          this->export_name_vector = export_name_vector;
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
        bool equal (env_t &env, shared_ptr <obj_t> obj);
        void apply (env_t &env, size_t arity);
        string repr (env_t &env);
    };
    closure_o::
    closure_o (env_t &env,
               name_vector_t name_vector,
               shared_ptr <jojo_t> jojo,
               bind_vector_t bind_vector,
               local_scope_t local_scope)
    {
        this->tag = "closure-t";
        this->name_vector = name_vector;
        this->jojo = jojo;
        this->bind_vector = bind_vector;
        this->local_scope = local_scope;
    }
    void
    closure_o::apply (env_t &env, size_t arity)
    {
        auto size = this->name_vector.size ();
        auto have = number_of_obj_in_bind_vector (this->bind_vector);
        auto lack = size - have;
        if (lack == arity) {
            auto obj_vector = pick_up_obj_vector
                (env, arity);
            auto bind_vector = bind_vector_merge_obj_vector
                 (this->bind_vector, obj_vector);
            auto local_scope = local_scope_extend
                (this->local_scope, bind_vector);
            auto frame = make_shared <frame_t>
                (this->jojo, local_scope);
            env.frame_stack.push (frame);
        }
        else if (arity < lack) {
            auto obj_vector = pick_up_obj_vector
                (env, arity);
            auto bind_vector = bind_vector_merge_obj_vector
                (this->bind_vector, obj_vector);
            auto closure = make_shared <closure_o>
                (env,
                 this->name_vector,
                 this->jojo,
                 bind_vector,
                 this->local_scope);
            env.obj_stack.push (closure);
        }
        else {
            cout << "- fatal error : closure_o::apply" << "\n";
            cout << "  over-arity apply" << "\n";
            cout << "  arity > lack" << "\n";
            cout << "  arity : " << arity << "\n";
            cout << "  lack : " << lack << "\n";
            exit (1);
        }
    }
    bool
    closure_o::equal (env_t &env, shared_ptr <obj_t> obj)
    {
        // raw pointers must be equal first
        if (this != obj.get ()) return false;
        auto that = static_pointer_cast <closure_o> (obj);
        // then scopes
        if (local_scope_equal
            (env,
             this->local_scope,
             that->local_scope)) return false;
        // then bindings
        if (bind_vector_equal
            (env,
             this->bind_vector,
             that->bind_vector)) return false;
        else return true;
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
        tag_t type_tag;
        type_o (env_t &env,
                tag_t type_tag,
                obj_map_t obj_map);
        bool equal (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    type_o::type_o (env_t &env,
                    tag_t type_tag,
                    obj_map_t obj_map)
    {
        this->tag = "type-t";
        this->type_tag = type_tag;
        this->obj_map = obj_map;
    }
    string
    type_o::repr (env_t &env)
    {
        return this->type_tag;
    }
    bool
    type_o::equal (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = static_pointer_cast <type_o> (obj);
        if (this->type_tag != that->type_tag) return false;
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
            if (obj->tag != "type-t") return nullptr;
            auto type = static_pointer_cast <type_o> (obj);
            auto begin = string_vector.begin () + 1;
            auto end = string_vector.end ();
            for (auto it = begin; it != end; it++) {
                auto field = *it;
                field += "-t";
                auto obj = type->obj_map [field];
                if (obj->tag != "type-t") return nullptr;
                type = static_pointer_cast <type_o> (obj);
            }
            return type;
        }
        return nullptr;
    }
    void
    assign (env_t &env,
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
    assign_type (env_t &env,
                 name_t prefix,
                 name_t type_name,
                 name_t type_tag)
    {
        auto type = make_shared <type_o>
            (env, type_tag, obj_map_t ());
        assign (env, prefix, type_name, type);
    }
    void
    define_type (env_t &env, name_t name)
    {
        auto type_name = name;
        auto type_tag = name;
        assign_type (env, "", type_name, type_tag);
    }
    shared_ptr <type_o>
    type_of (env_t &env, shared_ptr <obj_t> obj)
    {
        auto prefix = obj->tag;
        prefix.pop_back ();
        prefix.pop_back ();
        auto type = find_type_from_prefix (env, prefix);
        assert (type);
        return type;
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
    struct data_o: obj_t
    {
        data_o (env_t &env,
                tag_t tag,
                obj_map_t obj_map);
        bool equal (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    data_o::
    data_o (env_t &env,
            tag_t tag,
            obj_map_t obj_map)
    {
        this->tag = tag;
        this->obj_map = obj_map;
    }
    bool
    data_o::equal (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = static_pointer_cast <data_o> (obj);
        return obj_map_equal (env, this->obj_map, that->obj_map);

    }
    string
    data_o::repr (env_t &env)
    {
        if (this->obj_map.size () == 0) {
            string repr = "";
            repr += this->tag;
            repr.pop_back ();
            repr.pop_back ();
            repr += "-c";
            return repr;
        }
        else {
            string repr = "(";
            repr += this->tag;
            repr.pop_back ();
            repr.pop_back ();
            repr += "-c ";
            repr += obj_map_repr (env, this->obj_map);
            repr += ")";
            return repr;
        }
    }
    void
    assign_data (env_t &env,
                 name_t prefix,
                 name_t data_name,
                 name_t type_tag)
    {
        auto data = make_shared <data_o>
            (env, type_tag, obj_map_t ());
        assign (env, prefix, data_name, data);
    }
    shared_ptr <obj_t>
    true_c (env_t &env)
    {
       return make_shared <data_o>
           (env, "true-t", obj_map_t ());
    }
    bool
    true_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == "true-t";
    }
    shared_ptr <obj_t>
    false_c (env_t &env)
    {
       return make_shared <data_o>
           (env, "false-t", obj_map_t ());
    }
    bool
    false_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == "false-t";
    }
    bool
    bool_p (env_t &env, shared_ptr <obj_t> a)
    {
        return true_p (env, a)
            || false_p (env, a);
    }
    void
    test_bool ()
    {
    }
    struct data_pred_o: obj_t
    {
        tag_t type_tag;
        data_pred_o (env_t &env,
                     tag_t type_tag);
        void apply (env_t &env, size_t arity);
        bool equal (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    data_pred_o::
    data_pred_o (env_t &env,
                 tag_t type_tag)
    {
        this->tag = "data-pred-t";
        this->type_tag = type_tag;
    }
    void
    data_pred_o::apply (env_t &env, size_t arity)
    {
        if (arity == 1) {
            auto obj = env.obj_stack.top ();
            env.obj_stack.pop ();
            if (obj->tag == this->type_tag)
                env.obj_stack.push (true_c (env));
            else
                env.obj_stack.push (false_c (env));
        }
        else {
            cout << "- fatal error : data_pred_o::apply" << "\n";
            cout << "  arity of this kind of apply must be 1" << "\n";
            cout << "  arity : " << arity << "\n";
            exit (1);
        }
    }
    bool
    data_pred_o::equal (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = static_pointer_cast <data_pred_o> (obj);
        if (this->type_tag != that->type_tag) return false;
        return true;
    }
    string
    data_pred_o::repr (env_t &env)
    {
        string repr = "";
        repr += this->type_tag;
        repr.pop_back ();
        repr.pop_back ();
        repr += "-p";
        return repr;
    }
    void
    assign_data_pred (env_t &env,
                      name_t prefix,
                      name_t pred_name,
                      name_t type_tag)
    {
        auto data_pred = make_shared <data_pred_o>
            (env, type_tag);
        assign (env, prefix, pred_name, data_pred);
    }
    void
    define_data_pred (env_t &env,
                      name_t pred_name,
                      name_t type_tag)
    {
        assign_data_pred (env, "", pred_name, type_tag);
    }
    struct data_cons_o: obj_t
    {
        tag_t type_tag;
        name_vector_t name_vector;
        data_cons_o (env_t &env,
                     tag_t type_tag,
                     name_vector_t name_vector,
                     obj_map_t obj_map);
        void apply (env_t &env, size_t arity);
        bool equal (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    data_cons_o::
    data_cons_o (env_t &env,
                 tag_t type_tag,
                 name_vector_t name_vector,
                 obj_map_t obj_map)
    {
        this->tag = "data-cons-t";
        this->type_tag = type_tag;
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
                (env, this->type_tag, obj_map);
            env.obj_stack.push (data);
        }
        else if (arity < lack) {
            auto lack_name_vector = name_vector_obj_map_arity_lack
                (this->name_vector, this->obj_map, arity);
            auto obj_map = pick_up_obj_map_and_merge
                (env, lack_name_vector, this->obj_map);
            auto data_cons = make_shared <data_cons_o>
                (env, this->type_tag, this->name_vector, obj_map);
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
    data_cons_o::equal (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = static_pointer_cast <data_cons_o> (obj);
        if (this->type_tag != that->type_tag) return false;
        return obj_map_equal (env, this->obj_map, that->obj_map);
    }
    string
    data_cons_o::repr (env_t &env)
    {
        if (this->name_vector.size () == 0) {
            string repr = "";
            repr += this->type_tag;
            repr.pop_back ();
            repr.pop_back ();
            repr += "-c";
            return repr;
        }
        else {
            string repr = "(";
            repr += this->type_tag;
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
    assign_data_cons (env_t &env,
                      name_t prefix,
                      name_t data_name,
                      name_t type_tag,
                      name_vector_t name_vector)
    {
        auto data_cons = make_shared <data_cons_o>
            (env, type_tag, name_vector, obj_map_t ());
        assign (env, prefix, data_name, data_cons);
    }
    using prim_fn = function
        <void (env_t &, obj_map_t &)>;
    struct prim_o: obj_t
    {
        name_vector_t name_vector;
        prim_fn fn;
        prim_o (env_t &env,
                name_vector_t name_vector,
                prim_fn fn,
                obj_map_t obj_map);
        bool equal (env_t &env, shared_ptr <obj_t> obj);
        void apply (env_t &env, size_t arity);
        string repr (env_t &env);
    };
    prim_o::prim_o (env_t &env,
                    name_vector_t name_vector,
                    prim_fn fn,
                    obj_map_t obj_map)
    {
        this->tag = "prim-t";
        this->name_vector = name_vector;
        this->fn = fn;
        this->obj_map = obj_map;
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
    bool prim_o::equal (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = static_pointer_cast <prim_o> (obj);
        if (this != obj.get ()) return false;
        return obj_map_equal (env, this->obj_map, that->obj_map);
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
        define (env, name, make_shared <prim_o>
                (env, name_vector, fn, obj_map_t ()));
    }
    struct int_o: obj_t
    {
        int i;
        int_o (env_t &env, int i);
        bool equal (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    int_o::int_o (env_t &env, int i)
    {
        this->tag = "int-t";
        this->i = i;
    }
    string
    int_o::repr (env_t &env)
    {
        return to_string (this->i);
    }
    bool
    int_o::equal (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = static_pointer_cast <int_o> (obj);
        return (this->i == that->i);
    }
    bool
    int_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == "int-t";
    }
    void
    test_int ()
    {
    }
    struct str_o: obj_t
    {
        string str;
        str_o (env_t &env, string str);
        bool equal (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    str_o::str_o (env_t &env, string str)
    {
        this->tag = "str-t";
        this->str = str;
    }
    string
    str_o::repr (env_t &env)
    {
        return "\"" + this->str + "\"";
    }
    bool
    str_o::equal (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = static_pointer_cast <str_o> (obj);
        return (this->str == that->str);
    }
    bool
    str_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == "str-t";
    }
    void
    test_str ()
    {
    }
    shared_ptr <obj_t>
    null_c (env_t &env)
    {
       return make_shared <data_o>
           (env,
            "null-t",
            obj_map_t ());
    }
    shared_ptr <obj_t>
    cons_c (env_t &env,
            shared_ptr <obj_t> car,
            shared_ptr <obj_t> cdr)
    {
        auto obj_map = obj_map_t ();
        obj_map ["car"] = car;
        obj_map ["cdr"] = cdr;
        return make_shared <data_o>
            (env,
             "cons-t",
             obj_map);
    }
    bool
    cons_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == "cons-t";
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
    null_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == "null-t";
    }
    bool
    list_p (env_t &env, shared_ptr <obj_t> a)
    {
        return null_p (env, a)
            || cons_p (env, a);
    }
    size_t
    list_length (env_t &env, shared_ptr <obj_t> l)
    {
        assert (list_p (env, l));
        auto length = 0;
        while (! null_p (env, l)) {
            length++;
            l = cdr (env, l);
        }
        return length;
    }
    void
    test_list ()
    {
    }
    struct vect_o: obj_t
    {
        obj_vector_t obj_vector;
        vect_o (env_t &env, obj_vector_t obj_vector);
        bool equal (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    vect_o::vect_o (env_t &env, obj_vector_t obj_vector)
    {
        this->tag = "vect-t";
        this->obj_vector = obj_vector;
    }
    bool
    vect_equal (env_t &env,
                obj_vector_t &lhs,
                obj_vector_t &rhs);

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
    vect_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == "vect-t";
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
    void
    test_vect ()
    {
    }
    struct dict_o: obj_t
    {
        dict_o (env_t &env, obj_map_t obj_map);
        bool equal (env_t &env, shared_ptr <obj_t> obj);
        string repr (env_t &env);
    };
    dict_o::dict_o (env_t &env, obj_map_t obj_map)
    {
        this->tag = "dict-t";
        this->obj_map = obj_map;
    }
    bool
    dict_o::equal (env_t &env, shared_ptr <obj_t> obj)
    {
        if (this->tag != obj->tag) return false;
        auto that = static_pointer_cast <dict_o> (obj);
        return obj_map_equal (env, this->obj_map, that->obj_map);
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
        return a->tag == "dict-t";
    }
    shared_ptr <dict_o>
    list_to_dict (env_t &env, shared_ptr <obj_t> l)
    {
        auto obj_map = obj_map_t ();
        while (cons_p (env, l)) {
            auto head = car (env, l);
            assert (str_p (env, head));
            auto key = static_pointer_cast <str_o> (head);
            assert (cons_p (env, cdr (env, l)));
            auto obj = car (env, cdr (env, l));
            obj_map [key->str] = obj;
            l = cdr (env, cdr (env, l));
        }
        return make_shared <dict_o> (env, obj_map);
    }
    shared_ptr <obj_t>
    dict_to_list (env_t &env, shared_ptr <dict_o> dict)
    {
        auto result = null_c (env);
        for (auto &kv: dict->obj_map) {
            auto str = make_shared <str_o> (env, kv.first);
            auto obj = kv.second;
            result = cons_c (env, obj, result);
            result = cons_c (env, str, result);
        }
        return result;
    }
    void
    test_dict ()
    {
    }
    bool space_char_p (char c)
    {
        return (c == ' '  ||
                c == '\n' ||
                c == '\t');
    }
    bool delimiter_char_p (char c)
    {
        return (c == '(' ||
                c == ')' ||
                c == '[' ||
                c == ']' ||
                c == '{' ||
                c == '}' ||
                c == ',' ||
                c == ';' ||
                c == '`' ||
                c == '\'');
    }
    string
    string_from_char (char c)
    {
        string str = "";
        str.push_back (c);
        return str;
    }
    bool doublequote_char_p (char c)
    {
        return c == '"';
    }
    size_t find_word_length (string code, size_t begin)
    {
        size_t length = code.length ();
        size_t index = begin;
        while (true) {
            if (index == length)
                return index - begin;
            char c = code [index];
            if (space_char_p (c) or
                doublequote_char_p (c) or
                delimiter_char_p (c))
                return index - begin;
            index++;
        }
    }
    size_t find_string_length (string code, size_t begin)
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
    string_vector_t
    scan_word_vector (string code)
    {
        auto string_vector = string_vector_t ();
        size_t i = 0;
        size_t length = code.length ();
        while (i < length) {
            char c = code [i];
            if (space_char_p (c)) i++;
            else if (delimiter_char_p (c)) {
                string_vector.push_back (string_from_char (c));
                i++;
            }
            else if (doublequote_char_p (c)) {
                auto string_length = find_string_length (code, i);
                string str = code.substr (i, string_length);
                string_vector.push_back (str);
                i += string_length;
            }
            else {
                auto word_length = find_word_length (code, i);
                string word = code.substr (i, word_length);
                string_vector.push_back (word);
                i += word_length;
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
                auto obj = make_shared <str_o> (env, word);
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
            || word == "["
            || word == "{";
    }
    bool
    ket_word_p (string word)
    {
        return word == ")"
            || word == "]"
            || word == "}";
    }
    bool
    quote_word_p (string word)
    {
        return word == "'"
            || word == "`";
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
    word_list_head_with_bar_ket_counter
    (env_t &env,
     shared_ptr <obj_t> word_list,
     string bar,
     string ket,
     size_t counter)
    {
        if (counter == 0)
            return null_c (env);
        auto head = static_pointer_cast <str_o>
            (car (env, word_list));
        auto word = head->str;
        if (word == bar)
            return cons_c
                (env, head, word_list_head_with_bar_ket_counter
                 (env,
                  cdr (env, word_list),
                  bar, ket, counter + 1));
        if (word == ket)
            return cons_c
                (env, head, word_list_head_with_bar_ket_counter
                 (env,
                  cdr (env, word_list),
                  bar, ket, counter - 1));
        else
            return cons_c
                (env, head, word_list_head_with_bar_ket_counter
                 (env,
                  cdr (env, word_list),
                  bar, ket, counter));
    }
    shared_ptr <obj_t>
    word_list_head (env_t &env, shared_ptr <obj_t> word_list)
    {
        assert (cons_p (env, word_list));
        auto head = static_pointer_cast <str_o>
            (car (env, word_list));
        auto word = head->str;
        if (bar_word_p (word)) {
            auto bar = word;
            auto ket = bar_word_to_ket_word (word);
            return cons_c
                (env, head, word_list_head_with_bar_ket_counter
                 (env,
                  cdr (env, word_list),
                  bar, ket, 1));
        }
        else if (quote_word_p (word))
            return cons_c (env, head,
                           word_list_head (env, cdr (env, word_list)));
        else
            return cons_c (env, head, null_c (env));
    }
    shared_ptr <obj_t>
    word_list_rest_with_bar_ket_counter
    (env_t &env,
     shared_ptr <obj_t> word_list,
     string bar,
     string ket,
     size_t counter)
    {
        if (counter == 0)
            return word_list;
        auto head = static_pointer_cast <str_o>
            (car (env, word_list));
        auto word = head->str;
        if (word == bar)
            return word_list_rest_with_bar_ket_counter
                (env,
                 cdr (env, word_list),
                 bar, ket, counter + 1);
        if (word == ket)
            return word_list_rest_with_bar_ket_counter
                (env,
                 cdr (env, word_list),
                 bar, ket, counter - 1);
        else
            return word_list_rest_with_bar_ket_counter
                (env,
                 cdr (env, word_list),
                 bar, ket, counter);
    }
    shared_ptr <obj_t>
    word_list_rest (env_t &env, shared_ptr <obj_t> word_list)
    {
        assert (cons_p (env, word_list));
        auto head = static_pointer_cast <str_o>
            (car (env, word_list));
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
        else
            return cdr (env, word_list);
    }
    shared_ptr <obj_t>
    word_list_drop_ket
    (env_t &env,
     shared_ptr <obj_t> word_list,
     string ket)
    {
        auto head = car (env, word_list);
        auto rest = cdr (env, word_list);
        if (null_p (env, rest))
            return null_c (env);
        auto cdr_rest = cdr (env, rest);
        auto car_rest = static_pointer_cast <str_o> (car (env, rest));
        auto word = car_rest->str;
        if (null_p (env, cdr_rest)) {
            assert (word == ket);
            return cons_c (env, head, null_c (env));
        }
        else {
            return cons_c (env, head,
                           word_list_drop_ket (env, rest, ket));
        }
    }
    shared_ptr <obj_t>
    parse_sexp_list (env_t &env, shared_ptr <obj_t> word_list);

    shared_ptr <obj_t>
    parse_sexp (env_t &env, shared_ptr <obj_t> word_list)
    {
        auto head = static_pointer_cast <str_o>
            (car (env, word_list));
        auto word = head->str;
        auto rest = cdr (env, word_list);
        if (word == "(")
            return parse_sexp_list
                (env, word_list_drop_ket (env, rest, ")"));
        else if (word == "[")
            return list_to_vect
                (env, parse_sexp_list
                 (env, word_list_drop_ket (env, rest, "]")));
        else if (word == "{")
            return list_to_dict
                (env, parse_sexp_list
                 (env, word_list_drop_ket (env, rest, "}")));
        else if (word == "'")
            return cons_c (env, make_shared <str_o> (env, "quote"),
                           cons_c (env, parse_sexp (env, rest),
                                   null_c (env)));
        else if (word == "`")
            return cons_c (env, make_shared <str_o> (env, "partquote"),
                           cons_c (env, parse_sexp (env, rest),
                                   null_c (env)));
        else
            return head;
    }
    shared_ptr <obj_t>
    parse_sexp_list (env_t &env, shared_ptr <obj_t> word_list)
    {
        if (null_p (env, word_list))
            return word_list;
        else
            return cons_c
                (env,
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
            auto v = static_pointer_cast <vect_o> (a);
            auto l = vect_to_list (env, v);
            return "[" + sexp_list_repr (env, l) + "]";
        }
        else if (dict_p (env, a)) {
            auto d = static_pointer_cast <dict_o> (a);
            auto l = dict_to_list (env, d);
            return "{" + sexp_list_repr (env, l) + "}";
        }
        else {
            assert (str_p (env, a));
            auto str = static_pointer_cast <str_o> (a);
            return str->str;
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
      using keyword_fn = function
          <shared_ptr <jojo_t>
           (env_t &,
            local_ref_map_t &,
            shared_ptr <obj_t>)>;
      struct keyword_o: obj_t
      {
          keyword_fn fn;
          keyword_o (env_t &env, keyword_fn fn);
          bool equal (env_t &env, shared_ptr <obj_t> obj);
      };
      keyword_o::
      keyword_o (env_t &env, keyword_fn fn)
      {
          this->tag = "keyword-t";
          this->fn = fn;
      }
      bool
      keyword_o::equal (env_t &env, shared_ptr <obj_t> obj)
      {
          if (this->tag != obj->tag) return false;
          return this != obj.get ();
      }
      bool
      keyword_p (env_t &env, shared_ptr <obj_t> a)
      {
          return a->tag == "keyword-t";
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
          if (! str_p (env, (car (env, sexp)))) return false;
          auto head = static_pointer_cast <str_o> (car (env, sexp));
          auto name = head->str;
          auto it = env.box_map.find (name);
          if (it != env.box_map.end ()) {
              box_t *box = it->second;
              if (box->empty_p) return false;
              if (keyword_p (env, box->obj)) return true;
              else return false;
          }
          else {
              return false;
          }
      }
      keyword_fn
      get_keyword_fn (env_t &env, name_t name)
      {
          auto it = env.box_map.find (name);
          if (it != env.box_map.end ()) {
              box_t *box = it->second;
              if (box->empty_p) {
                  cout << "- fatal error: get_keyword_fn fail\n";
                  exit (1);
              }
              if (keyword_p (env, box->obj)) {
                  auto keyword = static_pointer_cast <keyword_o>
                      (box->obj);
                  return keyword->fn;
              }
              else {
                  cout << "- fatal error: get_keyword_fn fail\n";
                  exit (1);
              };
          }
          else {
              cout << "- fatal error: get_keyword_fn fail\n";
              exit (1);
          }
      }
    shared_ptr <jojo_t>
    string_compile (env_t &env,
                    local_ref_map_t &local_ref_map,
                    string str);

    shared_ptr <jojo_t>
    sexp_compile (env_t &env,
                  local_ref_map_t &local_ref_map,
                  shared_ptr <obj_t> sexp);

    shared_ptr <jojo_t>
    sexp_list_compile (env_t &env,
                       local_ref_map_t &local_ref_map,
                       shared_ptr <obj_t> sexp_list);
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
                      if (it->second->tag == "closure-t") {
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
          struct ref_jo_t: jo_t
          {
              box_t *box;
              ref_jo_t (box_t *box);
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
              auto index = 0;
              auto obj_vector = obj_vector_t ();
              while (index < this->counter) {
                  auto obj = env.obj_stack.top ();
                  env.obj_stack.pop ();
                  obj_vector.push_back (obj);
                  index++;
              }
              reverse (obj_vector.begin (),
                       obj_vector.end ());
              auto vect = make_shared <vect_o> (env, obj_vector);
              env.obj_stack.push (vect);
          }
          string
          collect_vect_jo_t::repr (env_t &env)
          {
              return "(collect-vect " + to_string (this->counter) + ")";
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
          collect_dict_jo_t::exe (env_t &env, local_scope_t &local_scope)
          {
              auto index = 0;
              auto obj_map = obj_map_t ();
              while (index < this->counter) {
                  auto obj = env.obj_stack.top ();
                  env.obj_stack.pop ();
                  auto str = env.obj_stack.top ();
                  env.obj_stack.pop ();
                  assert (str_p (env, str));
                  auto key = static_pointer_cast <str_o> (str);
                  obj_map [key->str] = obj;
                  index++;
              }
              auto dict = make_shared <dict_o> (env, obj_map);
              env.obj_stack.push (dict);
          }
          string
          collect_dict_jo_t::repr (env_t &env)
          {
              return "(collect-dict " + to_string (this->counter) + ")";
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
        bool
        field_string_p (string str)
        {
            auto pos = str.find (".");
            return (pos != string::npos);
        }
        shared_ptr <jojo_t>
        field_string_compile (env_t &env,
                            local_ref_map_t &local_ref_map,
                            string str)
        {
            auto string_vector = string_split (str, '.');
            auto jojo = string_compile
                (env, local_ref_map, string_vector [0]);
            auto begin = string_vector.begin () + 1;
            auto end = string_vector.end ();
            for (auto it = begin; it != end; it++) {
                jo_vector_t jo_vector = {
                    new field_jo_t (*it),
                };
                auto field_jojo = make_shared <jojo_t> (jo_vector);
                jojo = jojo_append (jojo, field_jojo);
            }
            return jojo;
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
        shared_ptr <jojo_t>
        string_string_compile (env_t &env,
                               local_ref_map_t &local_ref_map,
                               string str)
        {
            str = string_string_to_string (str);
            jo_vector_t jo_vector = {
                new lit_jo_t (make_shared <str_o> (env, str)),
            };
            auto jojo = make_shared <jojo_t> (jo_vector);
            return jojo;
        }
        bool
        key_string_p (string str)
        {
            auto size = str.size ();
            if (size < 2) return false;
            if (str [size-1] != ':') return false;
            return true;
        }
        shared_ptr <jojo_t>
        key_string_compile (env_t &env,
                            local_ref_map_t &local_ref_map,
                            string str)
        {
            auto size = str.size () - 1;
            str = str.substr (0, size);
            jo_vector_t jo_vector = {
                new lit_jo_t (make_shared <str_o> (env, str)),
            };
            auto jojo = make_shared <jojo_t> (jo_vector);
            return jojo;
        }
        bool
        int_string_p (string str)
        {
            auto size = str.size ();
            if (size < 1) return false;
            if (str [0] == '-')
                return int_string_p (str.substr (1, size - 1));
            auto pos = str.find_first_not_of ("0123456789");
            return pos == string::npos;
        }
        shared_ptr <jojo_t>
        int_string_compile (env_t &env,
                            local_ref_map_t &local_ref_map,
                            string str)
        {
            auto i = stoi (str);
            jo_vector_t jo_vector = {
                new lit_jo_t (make_shared <int_o> (env, i)),
            };
            auto jojo = make_shared <jojo_t> (jo_vector);
            return jojo;
        }
      shared_ptr <jojo_t>
      ref_compile (env_t &env,
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
      vect_compile (env_t &env,
                    local_ref_map_t &local_ref_map,
                    shared_ptr <vect_o> vect)
      {
          auto sexp_list = vect_to_list (env, vect);
          auto jojo = sexp_list_compile
              (env, local_ref_map, sexp_list);
          auto counter = list_length (env, sexp_list);
          jo_vector_t jo_vector = {
              new collect_vect_jo_t (counter),
          };
          auto ending_jojo = make_shared <jojo_t> (jo_vector);
          return jojo_append (jojo, ending_jojo);
      }
      shared_ptr <jojo_t>
      dict_compile (env_t &env,
                    local_ref_map_t &local_ref_map,
                    shared_ptr <dict_o> dict)
      {
          auto sexp_list = dict_to_list (env, dict);
          auto jojo = sexp_list_compile
              (env, local_ref_map, sexp_list);
          auto counter = list_length (env, sexp_list);
          counter = counter / 2;
          jo_vector_t jo_vector = {
              new collect_dict_jo_t (counter),
          };
          auto ending_jojo = make_shared <jojo_t> (jo_vector);
          return jojo_append (jojo, ending_jojo);
      }
      shared_ptr <jojo_t>
      keyword_compile (env_t &env,
                       local_ref_map_t &local_ref_map,
                       shared_ptr <obj_t> sexp)
      {
          auto head = static_pointer_cast <str_o> (car (env, sexp));
          auto body = cdr (env, sexp);
          auto name = head->str;
          auto fn = get_keyword_fn (env, name);
          return fn (env, local_ref_map, body);
      }
        bool
        field_head_p (env_t &env, shared_ptr <obj_t> head)
        {
            if (! str_p (env, head)) return false;
            auto str = static_pointer_cast <str_o> (head);
            return field_string_p (str->str);
        }
        shared_ptr <jojo_t>
        call_compile (env_t &env,
                      local_ref_map_t &local_ref_map,
                      shared_ptr <obj_t> sexp)
        {
            auto head = car (env, sexp);
            auto body = cdr (env, sexp);
            auto jo_vector = jo_vector_t ();
            // if (field_head_p (env, head)) {
            //     jo_vector.push_back
            //         (new apply_jo_t (list_length (env, body) + 1));
            // }
            // else {
            //     jo_vector.push_back
            //         (new apply_jo_t (list_length (env, body)));
            // }
            jo_vector.push_back
                (new apply_jo_t (list_length (env, body)));
            auto jojo = make_shared <jojo_t> (jo_vector);
            auto head_jojo = sexp_compile (env, local_ref_map, head);
            auto body_jojo = sexp_list_compile (env, local_ref_map, body);
            jojo = jojo_append (head_jojo, jojo);
            jojo = jojo_append (body_jojo, jojo);
            return jojo;
        }
    shared_ptr <jojo_t>
    string_compile (env_t &env,
                    local_ref_map_t &local_ref_map,
                    string str)
    {
        if (field_string_p (str))
            return field_string_compile (env, local_ref_map, str);
        else if (string_string_p (str))
            return string_string_compile (env, local_ref_map, str);
        else if (key_string_p (str))
            return key_string_compile (env, local_ref_map, str);
        else if (int_string_p (str))
            return int_string_compile (env, local_ref_map, str);
        else
            return ref_compile (env, local_ref_map, str);
    }
    shared_ptr <jojo_t>
    sexp_compile (env_t &env,
                  local_ref_map_t &local_ref_map,
                  shared_ptr <obj_t> sexp)
    {
        if (str_p (env, sexp)) {
            auto str = static_pointer_cast <str_o> (sexp);
            return string_compile (env, local_ref_map, str->str);
        }
        if (vect_p (env, sexp)) {
            auto vect = static_pointer_cast <vect_o> (sexp);
            return vect_compile (env, local_ref_map, vect);
        }
        if (dict_p (env, sexp)) {
            auto dict = static_pointer_cast <dict_o> (sexp);
            return dict_compile (env, local_ref_map, dict);
        }
        if (keyword_sexp_p (env, sexp)) {
            return keyword_compile (env, local_ref_map, sexp);
        }
        else {
            assert (cons_p (env, sexp));
            return call_compile (env, local_ref_map, sexp);
        }
    }
    shared_ptr <jojo_t>
    sexp_list_compile (env_t &env,
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
          bool equal (env_t &env, shared_ptr <obj_t> obj);
      };
      top_keyword_o::
      top_keyword_o (env_t &env, top_keyword_fn fn)
      {
          this->tag = "top-keyword-t";
          this->fn = fn;
      }
      bool
      top_keyword_o::equal (env_t &env, shared_ptr <obj_t> obj)
      {
          if (this->tag != obj->tag) return false;
          return this != obj.get ();
      }
      bool
      top_keyword_p (env_t &env, shared_ptr <obj_t> a)
      {
          return a->tag == "top-keyword-t";
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
          if (! str_p (env, (car (env, sexp)))) return false;
          auto head = static_pointer_cast <str_o> (car (env, sexp));
          auto name = head->str;
          auto it = env.box_map.find (name);
          if (it != env.box_map.end ()) {
              box_t *box = it->second;
              if (box->empty_p) return false;
              if (top_keyword_p (env, box->obj)) return true;
              else return false;
          }
          else {
              return false;
          }
      }
      top_keyword_fn
      get_top_keyword_fn (env_t &env, name_t name)
      {
          auto it = env.box_map.find (name);
          if (it != env.box_map.end ()) {
              box_t *box = it->second;
              if (box->empty_p) {
                  cout << "- fatal error: get_top_keyword_fn fail\n";
                  exit (1);
              }
              if (top_keyword_p (env, box->obj)) {
                  auto top_keyword = static_pointer_cast <top_keyword_o>
                      (box->obj);
                  return top_keyword->fn;
              }
              else {
                  cout << "- fatal error: get_top_keyword_fn fail\n";
                  exit (1);
              };
          }
          else {
              cout << "- fatal error: get_top_keyword_fn fail\n";
              exit (1);
          }
      }
    void
    jojo_run (env_t &env, shared_ptr <jojo_t> jojo)
    {
        auto base = env.frame_stack.size ();
        env.frame_stack.push (new_frame_from_jojo (jojo));
        env.run_with_base (base);
    }
    void
    sexp_eval (env_t &env, shared_ptr <obj_t> sexp)
    {
        if (top_keyword_sexp_p (env, sexp)) {
            auto head = static_pointer_cast <str_o> (car (env, sexp));
            auto body = cdr (env, sexp);
            auto name = head->str;
            auto fn = get_top_keyword_fn (env, name);
            fn (env, body);
        }
        else {
            auto local_ref_map = local_ref_map_t ();
            auto jojo = sexp_compile (env, local_ref_map, sexp);
            jojo_run (env, jojo);
        }
    }
    void
    sexp_list_eval (env_t &env, shared_ptr <obj_t> sexp_list)
    {
        if (null_p (env, sexp_list))
            return;
        else {
            sexp_eval (env, car (env, sexp_list));
            sexp_list_eval (env, cdr (env, sexp_list));
        }
    }
    void
    top_sexp_eval (env_t &env, shared_ptr <obj_t> sexp)
    {
        if (top_keyword_sexp_p (env, sexp)) {
            auto head = static_pointer_cast <str_o> (car (env, sexp));
            auto body = cdr (env, sexp);
            auto name = head->str;
            auto fn = get_top_keyword_fn (env, name);
            fn (env, body);
        }
        else {
            auto local_ref_map = local_ref_map_t ();
            auto jojo = sexp_compile (env, local_ref_map, sexp);
            jojo_run (env, jojo);
            if (! env.obj_stack.empty ())
                env.obj_stack.pop ();
        }
    }
    void
    top_sexp_list_eval (env_t &env, shared_ptr <obj_t> sexp_list)
    {
        if (null_p (env, sexp_list))
            return;
        else {
            top_sexp_eval (env, car (env, sexp_list));
            top_sexp_list_eval (env, cdr (env, sexp_list));
        }
    }
    void
    code_eval (env_t &env, shared_ptr <str_o> code)
    {
        auto word_list = scan_word_list (env, code);
        auto sexp_list = parse_sexp_list (env, word_list);
        top_sexp_list_eval (env, sexp_list);
    }
    shared_ptr <str_o>
    code_of_file (env_t &env, string file_name)
    {
        auto input_file = ifstream (file_name);
        auto buffer = stringstream ();
        buffer << input_file.rdbuf ();
        auto code = make_shared <str_o> (env, buffer.str ());
        return code;
    }
    void
    eval_file (env_t &env, string file_name)
    {
        auto code = code_of_file (env, file_name);
        code_eval (env, code);
    }
      string
      file_name_to_module_name (string file_name);

      void
      eval_module_file (env_t &env, string file_name);
      void
      mk_import (env_t &env, shared_ptr <obj_t> body)
      {
          assert (cons_p (env, body));
          auto head = car (env, body);
          assert (str_p (env, head));
          auto str = static_pointer_cast <str_o> (head);
          auto file_string = str->str;
          auto file_name = string_string_to_string (file_string);
          auto module_name = file_name_to_module_name (file_name);
          auto it = env.module_map.find (module_name);
          if (it != env.module_map.end ()) {
              // do not reload module;
          }
          else {
              auto box_map_back_up = env.box_map;
              eval_module_file (env, file_name);
              env.box_map = box_map_back_up;
          }
          auto it_mod = env.module_map.find (module_name);
          assert (it_mod != env.module_map.end ());
          auto mod = it_mod->second;
          for (auto name: mod->export_name_vector) {
              auto it_obj = mod->box_map.find (name);
              assert (it_obj != mod->box_map.end ());
              auto obj = it_obj->second;
              env.box_map [name] = obj;
          }
      }
      void
      mk_include (env_t &env, shared_ptr <obj_t> body)
      {
          auto vect = list_to_vect (env, body);
          auto file_string_vector = name_vector_t ();
          for (auto sexp: vect->obj_vector) {
              assert (str_p (env, sexp));
              auto str = static_pointer_cast <str_o> (sexp);
              file_string_vector.push_back (str->str);
          }
          for (auto file_string: file_string_vector) {
              assert (string_string_p (file_string));
              auto file_name = string_string_to_string (file_string);
              eval_file (env, file_name);
          }
      }
      void
      mk_export (env_t &env, shared_ptr <obj_t> body)
      {
          auto vect = list_to_vect (env, body);
          auto export_name_vector = name_vector_t ();
          for (auto sexp: vect->obj_vector) {
              assert (str_p (env, sexp));
              auto str = static_pointer_cast <str_o> (sexp);
              export_name_vector.push_back (str->str);
          }
          auto module_name = env.module_name_stack.top ();
          auto box_map = env.box_map;
          auto mod = make_shared <module_t>
              (module_name,
               box_map,
               export_name_vector);
          // ownership of `box_map` in `env` is transformed to `mod`
          env.box_map = box_map_t ();
          env.module_map [module_name] = mod;
      }
    void
    module_sexp_eval (env_t &env, shared_ptr <obj_t> sexp)
    {
        if (! cons_p (env, sexp)) return;
        auto head = car (env, sexp);
        if (! str_p (env, head)) return;
        auto body = cdr (env, sexp);
        auto str = static_pointer_cast <str_o> (head);
        auto name = str->str;
        if (name == "import")
            mk_import (env, body);
        else if (name == "export")
            mk_export (env, body);
        else if (name == "include")
            mk_include (env, body);
        else return;
    }
    void
    module_sexp_list_eval (env_t &env, shared_ptr <obj_t> sexp_list)
    {
        if (null_p (env, sexp_list))
            return;
        else {
            module_sexp_eval (env, car (env, sexp_list));
            module_sexp_list_eval (env, cdr (env, sexp_list));
        }
    }
    void
    module_code_eval (env_t &env, shared_ptr <str_o> code)
    {
        auto word_list = scan_word_list (env, code);
        auto sexp_list = parse_sexp_list (env, word_list);
        module_sexp_list_eval (env, sexp_list);
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
            if (! str_p (env, car (env, body)))
                return false;
            if (! cons_p (env, cdr (env, body)))
                return false;
            if (! cons_p (env, car (env, cdr (env, body))))
                return false;
            if (! str_p (env, car (env, car (env, cdr (env, body)))))
                return false;
            auto str = static_pointer_cast <str_o>
                (car (env, car (env, cdr (env, body))));
            return str->str == "data";
        }
        void
        tk_assign_data (env_t &env, shared_ptr <obj_t> body)
        {
            auto head = static_pointer_cast <str_o> (car (env, body));
            auto prefix = prefix_of_string (head->str);
            auto type_name = name_of_string (head->str);
            auto data_name = name_t2c (type_name);
            auto pred_name = name_t2p (type_name);
            auto type_tag = head->str;
            auto rest = cdr (env, body);
            auto data_body = cdr (env, (car (env, rest)));
            if (null_p (env, data_body)) {
                assign_type
                    (env, prefix, type_name, type_tag);
                assign_data_pred
                    (env, prefix, pred_name, type_tag);
                assign_data
                    (env, prefix, data_name, type_tag);
            }
            else {
                auto name_vect = list_to_vect (env, data_body);
                auto name_vector = name_vector_t ();
                for (auto obj: name_vect->obj_vector) {
                    auto str = static_pointer_cast <str_o> (obj);
                    name_vector.push_back (str->str);
                }
                assign_type
                    (env, prefix, type_name, type_tag);
                assign_data_pred
                    (env, prefix, pred_name, type_tag);
                assign_data_cons
                    (env, prefix, data_name, type_tag, name_vector);
            }
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
                 make_shared <str_o> (env, "lambda"),
                 lambda_body);
            lambda_body = cons_c
                (env,
                 lambda_body,
                 null_c (env));
            return cons_c (env, name, lambda_body);
        }
        shared_ptr <obj_t>
        sexp_substitute_recur (env_t &env,
                               shared_ptr <obj_t> sub,
                               shared_ptr <obj_t> sexp)
        {
            if (str_p (env, sexp)) {
                auto str = static_pointer_cast <str_o> (sexp);
                if (str->str == "recur")
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
                auto vect_sexp = static_pointer_cast <vect_o> (sexp);
                auto list_sexp = vect_to_list (env, vect_sexp);
                auto new_list_sexp = sexp_substitute_recur (env, sub, list_sexp);
                return list_to_vect (env, new_list_sexp);
            }
            else
                return sexp;
        }
        shared_ptr <obj_t>
        rest_patch_this (env_t &env, shared_ptr <obj_t> rest)
        {
            auto this_str = make_shared <str_o> (env, "this");
            obj_vector_t obj_vector = { this_str };
            auto vect = make_shared <vect_o> (env, obj_vector);
            auto lambda_body = cons_c (env, vect, rest);
            lambda_body = cons_c
                (env,
                 make_shared <str_o> (env, "lambda"),
                 lambda_body);
            lambda_body = cons_c
                (env,
                 lambda_body,
                 null_c (env));
            return lambda_body;
        }
        void
        tk_assign_value (env_t &env, shared_ptr <obj_t> body)
        {
            auto head = static_pointer_cast <str_o> (car (env, body));
            auto rest = cdr (env, body);
            auto name = name_of_string (head->str);
            auto prefix = prefix_of_string (head->str);
            if (prefix != "")
                rest = rest_patch_this (env, rest);
            rest = sexp_substitute_recur (env, head, rest);
            sexp_list_eval (env, rest);
            auto obj = env.obj_stack.top ();
            env.obj_stack.pop ();
            assign (env, prefix, name, obj);
        }
      void
      tk_assign (env_t &env, shared_ptr <obj_t> body)
      {
          if (assign_data_p (env, body))
              tk_assign_data (env, body);
          else if (assign_lambda_sugar_p (env, body))
              tk_assign_value (env, assign_lambda_desugar (env, body));
          else
              tk_assign_value (env, body);
      }
        struct lambda_jo_t: jo_t
        {
            name_vector_t name_vector;
            shared_ptr <jojo_t> jojo;
            lambda_jo_t (name_vector_t name_vector,
                         shared_ptr <jojo_t> jojo);
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        lambda_jo_t::
        lambda_jo_t (name_vector_t name_vector,
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
            auto closure = make_shared <closure_o>
                (env,
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
        struct case_jo_t: jo_t
        {
            jojo_map_t jojo_map;
            shared_ptr <jojo_t> default_jojo;
            case_jo_t (jojo_map_t jojo_map);
            case_jo_t (jojo_map_t jojo_map,
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
        struct assert_jo_t: jo_t
        {
            shared_ptr <obj_t> body;
            shared_ptr <jojo_t> jojo;
            assert_jo_t (shared_ptr <obj_t> body,
                         shared_ptr <jojo_t> jojo);
            jo_t * copy ();
            void exe (env_t &env, local_scope_t &local_scope);
            string repr (env_t &env);
        };
        assert_jo_t::
        assert_jo_t (shared_ptr <obj_t> body,
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
        bool
        true_p (env_t &env, shared_ptr <obj_t> a);

        string
        sexp_list_repr (env_t &env, shared_ptr <obj_t> a);

        void
        assert_jo_t::exe (env_t &env, local_scope_t &local_scope)
        {
            auto base = env.frame_stack.size ();
            auto jojo = this->jojo;
            auto frame = make_shared <frame_t> (jojo, local_scope);
            env.frame_stack.push (frame);
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
      shared_ptr <obj_t>
      lambda_patch_drop (env_t &env, shared_ptr <obj_t> sexp_list)
      {
          assert (cons_p (env, sexp_list));
          auto head = car (env, sexp_list);
          auto rest = cdr (env, sexp_list);
          if (null_p (env, rest)) return sexp_list;
          else {
              auto drop = cons_c
                  (env, make_shared <str_o> (env, "drop"),
                   null_c (env));
              sexp_list = lambda_patch_drop (env, rest);
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
              assert (str_p (env, obj));
              auto str = static_pointer_cast <str_o> (obj);
              name_vector.push_back (str->str);
          }
          return name_vector;
      }
      shared_ptr <jojo_t>
      k_lambda (env_t &env,
                local_ref_map_t &old_local_ref_map,
                shared_ptr <obj_t> body)
      {
          auto name_vect = static_pointer_cast <vect_o> (car (env, body));
          auto rest = cdr (env, body);
          auto name_vector = obj_vector_to_name_vector
              (env, name_vect->obj_vector);
          auto local_ref_map = local_ref_map_extend
              (env, old_local_ref_map, name_vector);
          rest = lambda_patch_drop (env, rest);
          auto rest_jojo = sexp_list_compile
              (env, local_ref_map, rest);
          jo_vector_t jo_vector = {
              new lambda_jo_t (name_vector, rest_jojo),
          };
          return make_shared <jojo_t> (jo_vector);
      }
      shared_ptr <jojo_t>
      case_compile (env_t &env,
                    local_ref_map_t &local_ref_map,
                    shared_ptr <obj_t> body)
      {
          auto jojo_map = jojo_map_t ();
          shared_ptr <jojo_t> default_jojo = nullptr;
          while (! null_p (env, body)) {
              auto one = car (env, body);
              auto head = static_pointer_cast <str_o> (car (env, one));
              auto rest = cdr (env, one);
              auto name = head->str;
              if (name == "_") {
                  auto jojo = sexp_list_compile (env, local_ref_map, rest);
                  body = cdr (env, body);
                  default_jojo = jojo;
              }
              else {
                  auto tag = name;
                  auto jojo = sexp_list_compile (env, local_ref_map, rest);
                  jojo_map.insert (make_pair (tag, jojo));
                  body = cdr (env, body);
              }
          }
          jo_vector_t jo_vector = {
              new case_jo_t (jojo_map, default_jojo),
          };
          return make_shared <jojo_t> (jo_vector);
      }
      shared_ptr <jojo_t>
      k_case (env_t &env,
              local_ref_map_t &local_ref_map,
              shared_ptr <obj_t> body)
      {
          auto head = car (env, body);
          auto rest = cdr (env, body);
          auto head_jojo = sexp_compile (env, local_ref_map, head);
          auto rest_jojo = case_compile (env, local_ref_map, rest);
          return jojo_append (head_jojo, rest_jojo);
      }
        shared_ptr <obj_t>
        sexp_literalize (env_t &env, shared_ptr <obj_t> sexp);
        shared_ptr <obj_t>
        str_sexp_literalize (env_t &env, shared_ptr <str_o> sexp)
        {
            auto str = sexp->str;
            if (int_string_p (str)) {
                auto i = stoi (str);
                return make_shared <int_o> (env, i);
            }
            if (string_string_p (str)) {
                str = string_string_to_string (str);
                return make_shared <str_o> (env, str);
            }
            if (key_string_p (str)) {
                auto size = str.size () - 1;
                str = str.substr (0, size);
                return make_shared <str_o> (env, str);
            }
            else {
                return sexp;
            }
        }
        shared_ptr <obj_t>
        vect_sexp_literalize (env_t &env, shared_ptr <vect_o> vect)
        {
            auto obj_vector = obj_vector_t ();
            for (auto &sexp: vect->obj_vector) {
                obj_vector.push_back (sexp_literalize (env, sexp));
            }
            return make_shared <vect_o> (env, obj_vector);
        }
        shared_ptr <obj_t>
        dict_sexp_literalize (env_t &env, shared_ptr <dict_o> dict)
        {
            auto obj_map = obj_map_t ();
            for (auto &kv: dict->obj_map) {
                auto key = kv.first;
                auto sexp = kv.second;
                obj_map.insert
                    (make_pair (key, sexp_literalize (env, sexp)));
            }
            return make_shared <dict_o> (env, obj_map);
        }
        shared_ptr <obj_t>
        sexp_literalize (env_t &env, shared_ptr <obj_t> sexp)
        {
            if (str_p (env, sexp))
                return str_sexp_literalize
                    (env, static_pointer_cast <str_o> (sexp));
            if (vect_p (env, sexp))
                return vect_sexp_literalize
                    (env, static_pointer_cast <vect_o> (sexp));
            if (dict_p (env, sexp))
                return dict_sexp_literalize
                    (env, static_pointer_cast <dict_o> (sexp));
            if (null_p (env, sexp))
                return sexp;
            else {
                assert (cons_p (env, sexp));
                return cons_c
                    (env,
                     sexp_literalize (env, car (env, sexp)),
                     sexp_literalize (env, cdr (env, sexp)));
            }
        }
      shared_ptr <jojo_t>
      k_quote (env_t &env,
               local_ref_map_t &local_ref_map,
               shared_ptr <obj_t> body)
      {
          assert (cons_p (env, body));
          assert (null_p (env, cdr (env, body)));
          auto sexp = car (env, body);
          jo_vector_t jo_vector = {
              new lit_jo_t (sexp_literalize (env, sexp)),
          };
          auto jojo = make_shared <jojo_t> (jo_vector);
          return jojo;
      }
      shared_ptr <jojo_t>
      k_note (env_t &env,
              local_ref_map_t &local_ref_map,
              shared_ptr <obj_t> body)
      {
          body = cons_c (env, make_shared <str_o> (env, "note"),
                         body);
          jo_vector_t jo_vector = {
              new lit_jo_t (body),
          };
          auto jojo = make_shared <jojo_t> (jo_vector);
          return jojo;
      }
      shared_ptr <jojo_t>
      k_assert (env_t &env,
                local_ref_map_t &local_ref_map,
                shared_ptr <obj_t> body)
      {
          auto jojo = sexp_list_compile (env, local_ref_map, body);
          jo_vector_t jo_vector = {
              new assert_jo_t (body, jojo),
          };
          return make_shared <jojo_t> (jo_vector);
      }
      void
      def_type (env_t &env, name_t name)
      {
          define_type (env, name);
          define_data_pred (env, name_t2p (name), name);
      }
      sig_t jj_type_of_sig = { "type-of", "obj" };
      void jj_type_of (env_t &env, obj_map_t &obj_map)
      {
          auto obj = obj_map ["obj"];
          env.obj_stack.push (type_of (env, obj));
      }
      void
      expose_type (env_t &env)
      {
          def_type (env, "type-t");
          define_prim (env,
                       jj_type_of_sig,
                       jj_type_of);
      }
      shared_ptr <obj_t>
      jj_true_c (env_t &env)
      {
         return make_shared <data_o>
             (env,
              "true-t",
              obj_map_t ());
      }
      shared_ptr <obj_t>
      jj_false_c (env_t &env)
      {
         return make_shared <data_o>
             (env,
              "false-t",
              obj_map_t ());
      }
      sig_t jj_not_sig = { "not", "bool" };
      // -- bool-t -> bool-t
      void jj_not (env_t &env, obj_map_t &obj_map)
      {
          auto obj = obj_map ["bool"];
          assert (bool_p (env, obj));
          if (true_p (env, obj))
              env.obj_stack.push (false_c (env));
          else
              env.obj_stack.push (true_c (env));
      }
      void
      expose_bool (env_t &env)
      {
          def_type (env, "true-t");
          def_type (env, "false-t");
          define (env, "true-c", jj_true_c (env));
          define (env, "false-c", jj_false_c (env));
          define_prim (env,
                       jj_not_sig,
                       jj_not);
      }
      void
      expose_int (env_t &env)
      {
          def_type (env, "int-t");
      }
      sig_t jj_str_print_sig = { "str-print", "str" };
      // -- str-t ->
      void jj_str_print (env_t &env, obj_map_t &obj_map)
      {
          auto obj = obj_map ["str"];
          assert (str_p (env, obj));
          auto str = static_pointer_cast <str_o> (obj);
          cout << str->str;
      }
      void
      expose_str (env_t &env)
      {
          def_type (env, "str-t");
          define_prim (env,
                       jj_str_print_sig,
                       jj_str_print);
      }
      shared_ptr <obj_t>
      jj_null_c (env_t &env)
      {
         return make_shared <data_o>
             (env,
              "null-t",
              obj_map_t ());
      }
      shared_ptr <data_cons_o>
      jj_cons_c (env_t &env)
      {
          return make_shared <data_cons_o>
              (env,
               "cons-t",
               name_vector_t ({ "car", "cdr" }),
               obj_map_t ());
      }
      void
      expose_list (env_t &env)
      {
          def_type (env, "null-t");
          def_type (env, "cons-t");
          define (env, "null-c", jj_null_c (env));
          define (env, "cons-c", jj_cons_c (env));
      }
      sig_t jj_list_to_vect_sig = { "list-to-vect", "list" };
      // -- (list-t t) -> (vect-t t)
      void jj_list_to_vect (env_t &env, obj_map_t &obj_map)
      {
          env.obj_stack.push (list_to_vect (env, obj_map ["list"]));
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
      expose_vect (env_t &env)
      {
          def_type (env, "vect-t");
          define_prim (env,
                       jj_list_to_vect_sig,
                       jj_list_to_vect);
          define_prim (env,
                       jj_vect_to_list_sig,
                       jj_vect_to_list);
      }
      void
      expose_dict (env_t &env)
      {
          def_type (env, "dict-t");
      }
      sig_t jj_scan_word_list_sig = { "scan-word-list", "code" };
      // -- str-t -> (list-t str-t)
      void jj_scan_word_list (env_t &env, obj_map_t &obj_map)
      {
          auto obj = obj_map ["code"];
          assert (str_p (env, obj));
          auto code = static_pointer_cast <str_o> (obj);
          env.obj_stack.push (scan_word_list (env, code));
      }
      sig_t jj_parse_sexp_sig = { "parse-sexp", "word-list" };
      // -- (list-t str-t) -> sexp-t
      void jj_parse_sexp (env_t &env, obj_map_t &obj_map)
      {
          env.obj_stack.push (parse_sexp (env, obj_map ["word-list"]));
      }
      sig_t jj_parse_sexp_list_sig = { "parse-sexp-list", "word-list" };
      // -- (list-t str-t) -> (list-t sexp-t)
      void jj_parse_sexp_list (env_t &env, obj_map_t &obj_map)
      {
          env.obj_stack.push (parse_sexp_list (env, obj_map ["word-list"]));
      }
      sig_t jj_sexp_repr_sig = { "sexp-repr", "sexp" };
      // -- sexp-t ->
      void jj_sexp_repr (env_t &env, obj_map_t &obj_map)
      {
          auto str = sexp_repr (env, obj_map ["sexp"]);
          env.obj_stack.push (make_shared <str_o> (env, str));
      }
      sig_t jj_sexp_list_repr_sig = { "sexp-list-repr", "sexp-list" };
      // -- (list-t sexp-t) ->
      void jj_sexp_list_repr (env_t &env, obj_map_t &obj_map)
      {
          auto str = sexp_list_repr (env, obj_map ["sexp-list"]);
          env.obj_stack.push (make_shared <str_o> (env, str));
      }
      void
      expose_sexp (env_t &env)
      {
          define_prim (env,
                       jj_scan_word_list_sig,
                       jj_scan_word_list);
          define_prim (env,
                       jj_parse_sexp_list_sig,
                       jj_parse_sexp_list);
          define_prim (env,
                       jj_parse_sexp_sig,
                       jj_parse_sexp);
          define_prim (env,
                       jj_sexp_repr_sig,
                       jj_sexp_repr);
          define_prim (env,
                       jj_sexp_list_repr_sig,
                       jj_sexp_list_repr);
      }
      void
      expose_top_keyword (env_t &env)
      {
          def_type (env, "top-keyword-t");
      }
      void
      expose_keyword (env_t &env)
      {
          def_type (env, "keyword-t");
      }
      void
      expose_syntax (env_t &env)
      {
          define_top_keyword (env, "=", tk_assign);
          define_keyword (env, "lambda", k_lambda);
          define_keyword (env, "case", k_case);
          define_keyword (env, "quote", k_quote);
          define_keyword (env, "note", k_note);
          define_keyword (env, "assert", k_assert);
      }
      sig_t jj_drop_sig = { "drop" };
      void jj_drop (env_t &env, obj_map_t &obj_map)
      {
          env.obj_stack.pop ();
      }
      void
      expose_stack (env_t &env)
      {
          define_prim (env,
                       jj_drop_sig,
                       jj_drop);
      }
      sig_t jj_repr_sig = { "repr", "obj" };
      void jj_repr (env_t &env, obj_map_t &obj_map)
      {
          auto obj = obj_map ["obj"];
          env.obj_stack.push
              (make_shared <str_o>
               (env, obj->repr (env)));
      }
      sig_t jj_print_sig = { "print", "obj" };
      void jj_print (env_t &env, obj_map_t &obj_map)
      {
          auto obj = obj_map ["obj"];
          cout << obj->repr (env) << flush;
          env.obj_stack.push (obj);
      }
      sig_t jj_println_sig = { "println", "obj" };
      void jj_println (env_t &env, obj_map_t &obj_map)
      {
          auto obj = obj_map ["obj"];
          cout << obj->repr (env) << "\n" << flush;
          env.obj_stack.push (obj);
      }
      sig_t jj_nl_sig = { "nl" };
      void jj_nl (env_t &env, obj_map_t &obj_map)
      {
          cout << "\n" << flush;
          auto nl = make_shared <str_o> (env, "\n");
          env.obj_stack.push (nl);
      }
      sig_t jj_equal_sig = { "equal", "lhs", "rhs" };
      void jj_equal (env_t &env, obj_map_t &obj_map)
      {
          auto lhs = obj_map ["lhs"];
          auto rhs = obj_map ["rhs"];
          if (obj_equal (env, lhs, rhs))
              env.obj_stack.push (true_c (env));
          else
              env.obj_stack.push (false_c (env));
      }
      sig_t jj_env_report_sig = { "env-report" };
      void jj_env_report (env_t &env, obj_map_t &obj_map)
      {
          env.report ();
          env.obj_stack.push (true_c (env));
      }
      void
      expose_misc (env_t &env)
      {
          def_type (env, "closure-t");
          def_type (env, "data-pred-t");
          def_type (env, "data-cons-t");
          def_type (env, "prim-t");
          define_prim (env,
                       jj_repr_sig,
                       jj_repr);
          define_prim (env,
                       jj_print_sig,
                       jj_print);
          define_prim (env,
                       jj_println_sig,
                       jj_println);
          define_prim (env,
                       jj_nl_sig,
                       jj_nl);
          define_prim (env,
                       jj_equal_sig,
                       jj_equal);
          define_prim (env,
                       jj_env_report_sig,
                       jj_env_report);
      }
      void
      test_step ()
      {
          auto env = env_t ();

          define (env, "s1", make_shared <str_o> (env, "bye"));
          define (env, "s2", make_shared <str_o> (env, "world"));

          jo_vector_t jo_vector = {
              new ref_jo_t (boxing (env, "s1")),
              new ref_jo_t (boxing (env, "s2")),
          };

          env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

          {
              env.run ();
              assert_stack_size (env, 2);
              assert_pop_eq (env, make_shared <str_o> (env, "world"));
              assert_pop_eq (env, make_shared <str_o> (env, "bye"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_data ()
      {
          auto env = env_t ();

          obj_map_t obj_map = {
              {"car", make_shared <str_o> (env, "bye")},
              {"cdr", make_shared <str_o> (env, "world")},
          };

          define (env, "last-cry", make_shared <data_o>
                  (env, "cons-t", obj_map));

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
                              "cons-t",
                              obj_map));
              assert_pop_eq (env, make_shared <str_o> (env, "world"));
              assert_pop_eq (env, make_shared <str_o> (env, "bye"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_apply ()
      {
          auto env = env_t ();

          define (env, "s1", make_shared <str_o> (env, "bye"));
          define (env, "s2", make_shared <str_o> (env, "world"));

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
              assert_pop_eq (env, make_shared <str_o> (env, "world"));
              assert_pop_eq (env, make_shared <str_o> (env, "bye"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_lambda_curry ()
      {
          auto env = env_t ();

          define (env, "s1", make_shared <str_o> (env, "bye"));
          define (env, "s2", make_shared <str_o> (env, "world"));

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
              assert_pop_eq (env, make_shared <str_o> (env, "bye"));
              assert_pop_eq (env, make_shared <str_o> (env, "world"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_data_cons ()
      {
          auto env = env_t ();

          define (env, "s1", make_shared <str_o> (env, "bye"));
          define (env, "s2", make_shared <str_o> (env, "world"));
          define (env, "cons-c", make_shared <data_cons_o>
                  (env,
                   "cons-t",
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
              assert_pop_eq (env, make_shared <str_o> (env, "world"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_data_cons_curry ()
      {
          auto env = env_t ();

          define (env, "s1", make_shared <str_o> (env, "bye"));
          define (env, "s2", make_shared <str_o> (env, "world"));
          define (env, "cons-c", make_shared <data_cons_o>
                  (env,
                   "cons-t",
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
              assert_pop_eq (env, make_shared <str_o> (env, "world"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_prim ()
      {
          auto env = env_t ();

          define (env, "s1", make_shared <str_o> (env, "bye"));
          define (env, "s2", make_shared <str_o> (env, "world"));

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
              assert_pop_eq (env, make_shared <str_o> (env, "world"));
              assert_pop_eq (env, make_shared <str_o> (env, "bye"));
              assert_pop_eq (env, make_shared <str_o> (env, "bye"));
              assert_pop_eq (env, make_shared <str_o> (env, "world"));
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
        test_bool ();
        test_int ();
        test_str ();
        test_list ();
        test_vect ();
        test_dict ();
        test_scan ();
    }
    void
    expose_all (env_t &env)
    {
        expose_bool (env);
        expose_int (env);
        expose_str (env);
        expose_list (env);
        expose_vect (env);
        expose_dict (env);
        expose_sexp (env);
        expose_top_keyword (env);
        expose_keyword (env);
        expose_syntax (env);
        expose_type (env);
        expose_stack (env);
        expose_misc (env);
    }
    bool
    mo_file_name_p (string file_name)
    {
        auto size = file_name.size ();
        if (size <= 3) return false;
        if (file_name [size - 1] != 'o') return false;
        if (file_name [size - 2] != 'm') return false;
        if (file_name [size - 3] != '.') return false;
        return true;
    }
    string
    file_name_to_module_name (string file_name)
    {
        auto module_name = file_name;
        module_name.pop_back ();
        module_name.pop_back ();
        module_name.pop_back ();
        return module_name;
    }
    void
    eval_module_file (env_t &env, string file_name)
    {
        assert (mo_file_name_p (file_name));
        auto code = code_of_file (env, file_name);
        auto module_name = file_name_to_module_name (file_name);
        env.module_name_stack.push (module_name);
        expose_all (env);
        module_code_eval (env, code);
        auto the_same_module_name = env.module_name_stack.top ();
        assert (module_name == the_same_module_name);
        env.module_name_stack.pop ();
    }
    void
    the_story_begins (string_vector_t arg_vector)
    {
        auto env = env_t ();
        expose_all (env);
        for (auto file_name: arg_vector) {
            if (mo_file_name_p (file_name))
                eval_module_file (env, file_name);
            else
                eval_file (env, file_name);
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
