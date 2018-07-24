    #include <iostream>
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
    struct env_t;
    struct obj_t;
    struct jo_t;
    using name_t = string;
    using name_vector_t = vector <name_t>;
    using bind_t = pair <name_t, shared_ptr <obj_t>>;
    // index from end
    using bind_vector_t = vector <bind_t>;
    // index from end
    using local_scope_t = vector <bind_vector_t>;
    using jo_vector_t = vector <jo_t *>;
    struct jojo_t
    {
        jo_vector_t jo_vector;
        jojo_t (jo_vector_t jo_vector);
        ~jojo_t ();
    };
    struct jo_t
    {
        virtual ~jo_t ();
        virtual void exe (env_t &env, local_scope_t &local_scope);
        virtual string repr (env_t &env);
    };
    using tag_t = size_t;
    using tag_name_vector_t = vector <name_t>;
    using tag_map_t = map <name_t, tag_t>;
    struct obj_t
    {
        tag_t tag;
        virtual ~obj_t ();
        virtual void print (env_t &env);
        virtual bool equal (env_t &env, shared_ptr <obj_t> obj);
        virtual void apply (env_t &env, size_t arity);
    };
    using obj_map_t = map <name_t, shared_ptr <obj_t>>;
    using obj_vector_t = vector <shared_ptr <obj_t>>;
    struct frame_t
    {
        size_t index;
        shared_ptr <jojo_t> jojo;
        local_scope_t local_scope;
        frame_t (shared_ptr <jojo_t> jojo, local_scope_t local_scope);
    };
    struct box_t
    {
        shared_ptr <obj_t> obj;
        bool empty_p;
        box_t ();
        box_t (shared_ptr <obj_t> obj);
    };
    using box_map_t = map <name_t, box_t *>;
    using obj_stack_t = stack <shared_ptr <obj_t>>;
    using frame_stack_t = stack <shared_ptr <frame_t>>;
    struct env_t
    {
        box_map_t box_map;
        obj_stack_t obj_stack;
        frame_stack_t frame_stack;
        tag_name_vector_t tag_name_vector;
        tag_map_t tag_map;
        void step ();
        void run ();
        void report ();
    };
    string
    jojo_repr (env_t &env, shared_ptr <jojo_t> jojo)
    {
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
    string
    name_vector_repr (name_vector_t &name_vector)
    {
        string repr = "[ ";
        for (auto name: name_vector) {
            repr += name;
            repr += " ";
        }
        repr += "]";
        return repr;
    }
      tag_t
      tagging (env_t &env, name_t name)
      {
          auto it = env.tag_map.find (name);
          if (it != env.tag_map.end ()) {
              tag_t tag = it->second;
              return tag;
          }
          else {
              auto tag = env.tag_name_vector.size ();
              env.tag_map [name] = tag;
              env.tag_name_vector.push_back (name);
              return tag;
          }
      }
      name_t
      name_of_tag (env_t &env, tag_t tag)
      {
          return env.tag_name_vector [tag];
      }
      void
      bind_vector_print (env_t &env, bind_vector_t bind_vector)
      {
          for (auto it = bind_vector.rbegin ();
               it != bind_vector.rend ();
               it++) {
              cout << "("
                   << distance(bind_vector.rbegin (), it)
                   << " ";
              cout << it->first
                   << " = ";
              auto obj = it->second;
              obj->print (env);
              cout << ") ";
          }
      }
      void
      local_scope_print (env_t &env, local_scope_t local_scope)
      {
          for (auto it = local_scope.rbegin ();
               it != local_scope.rend ();
               it++) {
              cout << "    "
                   << distance(local_scope.rbegin (), it)
                   << " ";
              bind_vector_print (env, *it);
              cout << "\n";
          }
      }
      obj_t::~obj_t ()
      {
          // all classes that will be derived from
          // should have a virtual or protected destructor,
          // otherwise deleting an instance via a pointer
          // to a base class results in undefined behavior.
      }
      void
      obj_t::print (env_t &env)
      {
          cout << "#<"
               << name_of_tag (env, this->tag)
               << ">";
      }
      bool
      obj_t::equal (env_t &env, shared_ptr <obj_t> obj)
      {
          if (this->tag != this->tag)
              return false;
          else {
              cout << "- fatal error : obj_t::equal" << "\n"
                   << "  equal is not implemented for  : "
                   << name_of_tag (env, obj->tag) << "\n"
                   << "\n";
              exit (1);
          }
      }
      void
      obj_t::apply (env_t &env, size_t arity)
      {
          cout << "- fatal error : applying non applicable object" << "\n";
          exit (1);
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
      struct lambda_o: obj_t
      {
          name_vector_t name_vector;
          shared_ptr <jojo_t> jojo;
          bind_vector_t bind_vector;
          local_scope_t local_scope;
          lambda_o (env_t &env,
                    name_vector_t name_vector,
                    shared_ptr <jojo_t> jojo,
                    bind_vector_t bind_vector,
                    local_scope_t local_scope);
          bool equal (env_t &env, shared_ptr <obj_t> obj);
          void apply (env_t &env, size_t arity);
          void print (env_t &env);
      };
      lambda_o::
      lambda_o (env_t &env,
                name_vector_t name_vector,
                shared_ptr <jojo_t> jojo,
                bind_vector_t bind_vector,
                local_scope_t local_scope)
      {
          this->tag = tagging (env, "lambda-t");
          this->name_vector = name_vector;
          this->jojo = jojo;
          this->bind_vector = bind_vector;
          this->local_scope = local_scope;
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
          cout << "- fatal error ! bind_vector_insert_obj" << "\n"
               << "  the bind_vector is filled" << "\n"
               << "\n";
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
      local_scope_t
      local_scope_extend (local_scope_t old_local_scope,
                          bind_vector_t bind_vector)
      {
          auto local_scope = old_local_scope;
          local_scope.push_back (bind_vector);
          return local_scope;
      }
      void
      lambda_o::apply (env_t &env, size_t arity)
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
              auto lambda = make_shared <lambda_o>
                  (env,
                   this->name_vector,
                   this->jojo,
                   bind_vector,
                   this->local_scope);
              env.obj_stack.push (lambda);
          }
          else {
              cout << "- fatal error : lambda_o::apply" << "\n"
                   << "  over-arity apply" << "\n"
                   << "  arity > lack" << "\n"
                   << "  arity : " << arity << "\n"
                   << "  lack : " << lack << "\n"
                   << "\n";
              exit (1);
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
      bool
      lambda_o::equal (env_t &env, shared_ptr <obj_t> obj)
      {
          // raw pointers must be equal first
          if (this != obj.get ()) return false;
          auto that = static_pointer_cast <lambda_o> (obj);
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
      void
      lambda_o::print (env_t &env)
      {
          cout << "(lambda "
               << name_vector_repr (this->name_vector)
               << " "
               << jojo_repr (env, this->jojo)
               << ")";
      }
      struct string_o: obj_t
      {
          string str;
          string_o (env_t &env, string str);
          bool equal (env_t &env, shared_ptr <obj_t> obj);
          void print (env_t &env);
      };
      string_o::string_o (env_t &env, string str)
      {
          this->tag = tagging (env, "string-t");
          this->str = str;
      }
      void string_o::print (env_t &env)
      {
          cout << '"' << this->str << '"';
      }
      bool
      string_o::equal (env_t &env, shared_ptr <obj_t> obj)
      {
          if (this->tag != obj->tag) return false;
          auto that = static_pointer_cast <string_o> (obj);
          return (this->str == that->str);
      }
      struct data_o: obj_t
      {
          obj_map_t obj_map;
          data_o (env_t &env,
                  tag_t tag,
                  obj_map_t obj_map);
          bool equal (env_t &env, shared_ptr <obj_t> obj);
          void print (env_t &env);
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
      bool
      data_o::equal (env_t &env, shared_ptr <obj_t> obj)
      {
          if (this->tag != obj->tag) return false;
          auto that = static_pointer_cast <data_o> (obj);
          return obj_map_equal (env, this->obj_map, that->obj_map);

      }
      void
      data_o::print (env_t &env)
      {
          // ><><><
      }
      struct data_cons_o: obj_t
      {
          tag_t type_tag;
          name_vector_t name_vector;
          obj_map_t obj_map;
          data_cons_o (env_t &env,
                       tag_t type_tag,
                       name_vector_t name_vector,
                       obj_map_t obj_map);
          void apply (env_t &env, size_t arity);
          bool equal (env_t &env, shared_ptr <obj_t> obj);
          void print (env_t &env);
      };
      data_cons_o::
      data_cons_o (env_t &env,
                   tag_t type_tag,
                   name_vector_t name_vector,
                   obj_map_t obj_map)
      {
          this->tag = tagging (env, "data-cons-t");
          this->type_tag = type_tag;
          this->name_vector = name_vector;
          this->obj_map = obj_map;
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
              cout << "- fatal error : data_cons_o::apply" << "\n"
                   << "  over-arity apply" << "\n"
                   << "  arity > lack" << "\n"
                   << "  arity : " << arity << "\n"
                   << "  lack : " << lack << "\n"
                   << "\n";
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
      void
      data_cons_o::print (env_t &env)
      {
          // ><><><
      }
      using prim_fn = function <void (env_t &, obj_map_t &)>;
      struct prim_o: obj_t
      {
          name_vector_t name_vector;
          prim_fn fn;
          obj_map_t obj_map;
          prim_o (env_t &env,
                  name_vector_t name_vector,
                  prim_fn fn,
                  obj_map_t obj_map);
          void print (env_t &env);
          bool equal (env_t &env, shared_ptr <obj_t> obj);
          void apply (env_t &env, size_t arity);
      };
      prim_o::prim_o (env_t &env,
                      name_vector_t name_vector,
                      prim_fn fn,
                      obj_map_t obj_map)
      {
          this->tag = tagging (env, "prim-t");
          this->name_vector = name_vector;
          this->fn = fn;
          this->obj_map = obj_map;
      }
      void prim_o::print (env_t &env)
      {
          cout << "(prim "
               << name_vector_repr (this->name_vector)
               << ")";
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
              cout << "- fatal error : prim_o::apply" << "\n"
                   << "  over-arity apply" << "\n"
                   << "  arity > lack" << "\n"
                   << "  arity : " << arity << "\n"
                   << "  lack : " << lack << "\n"
                   << "\n";
              exit (1);
          }
      }
      frame_t::frame_t (shared_ptr <jojo_t> jojo,
                        local_scope_t local_scope)
      {
          this->index = 0;
          this->jojo = jojo;
          this->local_scope = local_scope;
      }
      void
      frame_report (env_t &env, shared_ptr <frame_t> frame)
      {
          cout << "  - [" << frame->index+1
               << "/" << frame->jojo->jo_vector.size ()
               << "] ";
          jojo_print_with_index (env, frame->jojo, frame->index);
          cout << "\n";
          cout << "  - [" << frame->local_scope.size () << "] "
               << "local_scope - "
               << "\n";
          local_scope_print (env, frame->local_scope);
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
          if (it != env.box_map.end ())
              return it->second;
          else {
              auto box = new box_t ();
              env.box_map [name] = box;
              return box;
          }
      }
      void
      box_map_report (env_t &env)
      {
          cout << "- [" << env.box_map.size () << "] "
               << "box_map - "
               << "\n";
          cout << "  ";
          for (auto &kv: env.box_map) {
              cout << "(" << kv.first << " = ";
              auto box = kv.second;
              box->obj->print (env);
              cout << ") ";
          }
          cout << "\n";
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
      void
      frame_stack_report (env_t &env)
      {
          cout << "- [" << env.frame_stack.size () << "] "
               << "frame_stack - "
               << "\n";
          frame_stack_t frame_stack = env.frame_stack;
          while (! frame_stack.empty ()) {
             auto frame = frame_stack.top ();
             frame_report (env, frame);
             frame_stack.pop ();
          }
      }
      void
      obj_stack_report (env_t &env)
      {
          cout << "- [" << env.obj_stack.size () << "] "
               << "obj_stack - "
               << "\n";
          cout << "  ";
          auto obj_stack = env.obj_stack;
          while (! obj_stack.empty ()) {
              auto obj = obj_stack.top ();
              obj->print (env);
              cout << " ";
              obj_stack.pop ();
          }
          cout << "\n";
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
    env_t::report ()
    {
        box_map_report (*this);
        frame_stack_report (*this);
        obj_stack_report (*this);
        cout << "\n";
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
          return "(unknown)";
      }
      struct ref_jo_t: jo_t
      {
          box_t *box;
          ref_jo_t (box_t *box);
          void exe (env_t &env, local_scope_t &local_scope);
          string repr (env_t &env);
      };
      ref_jo_t::ref_jo_t (box_t *box)
      {
          this->box = box;
      }
      void
      ref_jo_t::exe (env_t &env, local_scope_t &local_scope)
      {
          assert (! this->box->empty_p);
          env.obj_stack.push (this->box->obj);
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
          void exe (env_t &env, local_scope_t &local_scope);
          string repr (env_t &env);
      };
      local_ref_jo_t::
      local_ref_jo_t (size_t level, size_t index)
      {
          this->level = level;
          this->index = index;
      }
      template <class T>
      T
      vector_rev_ref (vector <T> vect, size_t rev_index)
      {
          size_t size = vect.size ();
          size_t index = size - rev_index - 1;
          return vect [index];
      }
      void
      local_ref_jo_t::exe (env_t &env, local_scope_t &local_scope)
      {
          // this is the only place where
          //   the local_scope in the arg of exe is uesd.
          auto bind_vector =
              vector_rev_ref (local_scope, this->level);
          auto bind =
              vector_rev_ref (bind_vector, this->index);
          auto obj = bind.second;
          env.obj_stack.push (obj);
      }
      string
      local_ref_jo_t::repr (env_t &env)
      {
          return "(local " +
              to_string (this->level) + " " +
              to_string (this->index) + ")";
      }
      struct lambda_jo_t: jo_t
      {
          name_vector_t name_vector;
          shared_ptr <jojo_t> jojo;
          lambda_jo_t (name_vector_t name_vector,
                       shared_ptr <jojo_t> jojo);
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
      void
      lambda_jo_t::exe (env_t &env, local_scope_t &local_scope)
      {
          auto frame = env.frame_stack.top ();
          auto lambda = make_shared <lambda_o>
              (env,
               this->name_vector,
               this->jojo,
               bind_vector_from_name_vector (this->name_vector),
               frame->local_scope);
          env.obj_stack.push (lambda);
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
      struct field_jo_t: jo_t
      {
          name_t name;
          field_jo_t (name_t name);
          void exe (env_t &env, local_scope_t &local_scope);
          string repr (env_t &env);
      };
      field_jo_t::field_jo_t (name_t name)
      {
          this->name = name;
      }
      void
      field_jo_t::exe (env_t &env, local_scope_t &local_scope)
      {
          auto obj = env.obj_stack.top ();
          env.obj_stack.pop ();
          auto data = static_pointer_cast <data_o> (obj);
          auto it = data->obj_map.find (this->name);
          if (it != data->obj_map.end ()) {
              env.obj_stack.push (it->second);
              return;
          }
          cout << "- fatal error ! unknown field : "
               << this->name
               << "\n";
          exit (1);
      }
      string
      field_jo_t::repr (env_t &env)
      {
          return "." + this->name;
      }
      struct apply_jo_t: jo_t
      {
          size_t arity;
          apply_jo_t (size_t arity);
          void exe (env_t &env, local_scope_t &local_scope);
          string repr (env_t &env);
      };
      apply_jo_t::
      apply_jo_t (size_t arity)
      {
          this->arity = arity;
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
          return "apply";
      }
    void
    define (env_t &env, name_t name, shared_ptr <obj_t> obj)
    {
        auto it = env.box_map.find (name);
        if (it != env.box_map.end ()) {
            box_t *box = it->second;
            box->empty_p = false;
            box->obj = obj;
        }
        else {
            env.box_map [name] = new box_t (obj);
        }
    }
    shared_ptr <data_o>
    true_c (env_t &env)
    {
       return make_shared <data_o>
           (env,
            tagging (env, "true-t"),
            obj_map_t ());
    }
    shared_ptr <data_o>
    false_c (env_t &env)
    {
       return make_shared <data_o>
           (env,
            tagging (env, "false-t"),
            obj_map_t ());
    }
    void import_bool (env_t &env)
    {
        define (env, "true-c", true_c (env));
        define (env, "false-c", false_c (env));
    }
    shared_ptr <data_o>
    null_c (env_t &env)
    {
       return make_shared <data_o>
           (env,
            tagging (env, "null-t"),
            obj_map_t ());
    }
    shared_ptr <data_cons_o>
    cons_c (env_t &env)
    {
        return make_shared <data_cons_o>
            (env,
             tagging (env, "cons-t"),
             name_vector_t ({ "car", "cdr" }),
             obj_map_t ());
    }
    void import_list (env_t &env)
    {
        define (env, "null-c", null_c (env));
        define (env, "cons-c", cons_c (env));
    }
    using string_vector_t = vector <string> ;
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
    string_vector_t
    code_scan (string code)
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
            // else if (doublequote_char_p (c)) {
            // }
            else {
                auto word_length = find_word_length (code, i);
                string word = code.substr (i, word_length);
                string_vector.push_back (word);
                i += word_length;
            }
        }
        return string_vector;
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
      void
      test_step ()
      {
          auto env = env_t ();

          define (env, "s1", make_shared <string_o> (env, "bye"));
          define (env, "s2", make_shared <string_o> (env, "world"));

          jo_vector_t jo_vector = {
              new ref_jo_t (boxing (env, "s1")),
              new ref_jo_t (boxing (env, "s2")),
          };

          env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

          {
              env.run ();
              assert_stack_size (env, 2);
              assert_pop_eq (env, make_shared <string_o> (env, "world"));
              assert_pop_eq (env, make_shared <string_o> (env, "bye"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_data ()
      {
          auto env = env_t ();

          obj_map_t obj_map = {
              {"car", make_shared <string_o> (env, "bye")},
              {"cdr", make_shared <string_o> (env, "world")},
          };

          define (env, "last-cry", make_shared <data_o>
                  (env, tagging (env, "cons-t"), obj_map));

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
                              tagging (env, "cons-t"),
                              obj_map));
              assert_pop_eq (env, make_shared <string_o> (env, "world"));
              assert_pop_eq (env, make_shared <string_o> (env, "bye"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_apply ()
      {
          auto env = env_t ();

          define (env, "s1", make_shared <string_o> (env, "bye"));
          define (env, "s2", make_shared <string_o> (env, "world"));

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

          // {
          //     env.report ();
          //     env.run ();
          //     env.report ();
          // }

          {
              env.run ();
              assert_stack_size (env, 2);
              assert_pop_eq (env, make_shared <string_o> (env, "world"));
              assert_pop_eq (env, make_shared <string_o> (env, "bye"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_lambda_curry ()
      {
          auto env = env_t ();

          define (env, "s1", make_shared <string_o> (env, "bye"));
          define (env, "s2", make_shared <string_o> (env, "world"));

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
          //     env.step (); env.report ();
          //     env.step (); env.report ();
          //     env.step (); env.report ();
          //     env.step (); env.report ();
          //     env.step (); env.report ();
          //     env.step (); env.report ();
          //     env.step (); env.report ();
          // }

          {
              env.run ();
              assert_stack_size (env, 2);
              assert_pop_eq (env, make_shared <string_o> (env, "bye"));
              assert_pop_eq (env, make_shared <string_o> (env, "world"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_data_cons ()
      {
          auto env = env_t ();

          define (env, "s1", make_shared <string_o> (env, "bye"));
          define (env, "s2", make_shared <string_o> (env, "world"));
          define (env, "cons-c", make_shared <data_cons_o>
                  (env,
                   tagging (env, "cons-t"),
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

          // {
          //     env.report ();
          //     env.run ();
          //     env.report ();
          // }

          {
              env.run ();
              assert_stack_size (env, 1);
              assert_pop_eq (env, make_shared <string_o> (env, "world"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_data_cons_curry ()
      {
          auto env = env_t ();

          define (env, "s1", make_shared <string_o> (env, "bye"));
          define (env, "s2", make_shared <string_o> (env, "world"));
          define (env, "cons-c", make_shared <data_cons_o>
                  (env,
                   tagging (env, "cons-t"),
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

          // {
          //     env.report ();
          //     env.run ();
          //     env.report ();
          // }

          {
              env.run ();
              assert_stack_size (env, 1);
              assert_pop_eq (env, make_shared <string_o> (env, "world"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_prim ()
      {
          auto env = env_t ();

          define (env, "s1", make_shared <string_o> (env, "bye"));
          define (env, "s2", make_shared <string_o> (env, "world"));

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

          // {
          //     env.report ();
          //     env.run ();
          //     env.report ();
          // }

          {
              env.run ();
              assert_stack_size (env, 4);
              assert_pop_eq (env, make_shared <string_o> (env, "world"));
              assert_pop_eq (env, make_shared <string_o> (env, "bye"));
              assert_pop_eq (env, make_shared <string_o> (env, "bye"));
              assert_pop_eq (env, make_shared <string_o> (env, "world"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_bool ()
      {
          auto env = env_t ();

          import_bool (env);

          jo_vector_t jo_vector = {
              new ref_jo_t (boxing (env, "true-c")),
              new ref_jo_t (boxing (env, "false-c")),
          };

          env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

          // {
          //     env.report ();
          //     env.run ();
          //     env.report ();
          // }

          {
              env.run ();
              assert_stack_size (env, 2);
              assert_pop_eq (env, false_c (env));
              assert_pop_eq (env, true_c (env));
              assert_stack_size (env, 0);
          }
      }
      void
      test_list ()
      {
          auto env = env_t ();

          import_list (env);

          define (env, "s1", make_shared <string_o> (env, "bye"));
          define (env, "s2", make_shared <string_o> (env, "world"));

          jo_vector_t jo_vector = {
              new ref_jo_t (boxing (env, "s1")),
              new ref_jo_t (boxing (env, "s2")),
              new ref_jo_t (boxing (env, "cons-c")),
              new apply_jo_t (2),
              new field_jo_t ("cdr"),
          };

          env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

          // {
          //     env.report ();
          //     env.run ();
          //     env.report ();
          // }

          {
              env.run ();
              assert_stack_size (env, 1);
              assert_pop_eq (env, make_shared <string_o> (env, "world"));
              assert_stack_size (env, 0);
          }
      }
      void
      test_scan ()
      {
          auto code = "(cons-c <car> <cdr>)";
          auto string_vector = code_scan (code);
          assert (string_vector.size () == 5);
          assert (string_vector [0] == "(");
          assert (string_vector [1] == "cons-c");
          assert (string_vector [2] == "<car>");
          assert (string_vector [3] == "<cdr>");
          assert (string_vector [4] == ")");
      }
    void
    test_all ()
    {
        // core
        test_step ();
        test_data ();
        test_apply ();
        test_lambda_curry ();
        test_data_cons ();
        test_data_cons_curry ();
        test_prim ();
        // data
        test_bool ();
        test_list ();
        // parser
        test_scan ();
    }
    int
    main ()
    {
        test_all ();
        return 0;
    }
