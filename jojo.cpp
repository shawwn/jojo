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
        virtual jo_t * copy ();
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
        void run_with_base (size_t base);
        void run_and_report ();
    };
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
          if (this->tag != obj->tag)
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
      struct str_o: obj_t
      {
          string str;
          str_o (env_t &env, string str);
          bool equal (env_t &env, shared_ptr <obj_t> obj);
          void print (env_t &env);
      };
      str_o::str_o (env_t &env, string str)
      {
          this->tag = tagging (env, "str-t");
          this->str = str;
      }
      void str_o::print (env_t &env)
      {
          cout << '"' << this->str << '"';
      }
      bool
      str_o::equal (env_t &env, shared_ptr <obj_t> obj)
      {
          if (this->tag != obj->tag) return false;
          auto that = static_pointer_cast <str_o> (obj);
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
          // [todo]
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
          // [todo]
      }
      using prim_fn = function
          <void (env_t &, obj_map_t &)>;
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
    void
    env_t::run_with_base (size_t base)
    {
        while (this->frame_stack.size () > base) {
            this->step ();
        }
    }
    void
    env_t::run_and_report ()
    {
        this->report ();
        this->run ();
        this->report ();
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
          return "(unknown)";
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
        define (env, name,
                make_shared <prim_o>
                (env, name_vector, fn, obj_map_t ()));
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
    void import_test (env_t &env)
    {

    }
    shared_ptr <data_o>
    jj_true_c (env_t &env)
    {
       return make_shared <data_o>
           (env,
            tagging (env, "true-t"),
            obj_map_t ());
    }
    shared_ptr <data_o>
    jj_false_c (env_t &env)
    {
       return make_shared <data_o>
           (env,
            tagging (env, "false-t"),
            obj_map_t ());
    }
    void
    import_bool (env_t &env)
    {
        define (env, "true-c", jj_true_c (env));
        define (env, "false-c", jj_false_c (env));
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
            assert_pop_eq (env, jj_false_c (env));
            assert_pop_eq (env, jj_true_c (env));
            assert_stack_size (env, 0);
        }
    }
    shared_ptr <data_o>
    null_c (env_t &env)
    {
       return make_shared <data_o>
           (env,
            tagging (env, "null-t"),
            obj_map_t ());
    }
    shared_ptr <data_o>
    cons_c (env_t &env,
            shared_ptr <obj_t> car,
            shared_ptr <obj_t> cdr)
    {
        auto obj_map = obj_map_t ();
        obj_map ["car"] = car;
        obj_map ["cdr"] = cdr;
        return make_shared <data_o>
            (env,
             tagging (env, "cons-t"),
             obj_map);
    }
    shared_ptr <obj_t>
    car (env_t &env, shared_ptr <obj_t> a)
    {
        assert (a->tag == tagging (env, "cons-t"));
        auto cons = static_pointer_cast <data_o> (a);
        return cons->obj_map ["car"];
    }
    shared_ptr <obj_t>
    cdr (env_t &env, shared_ptr <obj_t> a)
    {
        assert (a->tag == tagging (env, "cons-t"));
        auto cons = static_pointer_cast <data_o> (a);
        return cons->obj_map ["cdr"];
    }
    bool
    null_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == tagging (env, "null-t");
    }
    bool
    cons_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == tagging (env, "cons-t");
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
    shared_ptr <data_o>
    jj_null_c (env_t &env)
    {
       return make_shared <data_o>
           (env,
            tagging (env, "null-t"),
            obj_map_t ());
    }
    shared_ptr <data_cons_o>
    jj_cons_c (env_t &env)
    {
        return make_shared <data_cons_o>
            (env,
             tagging (env, "cons-t"),
             name_vector_t ({ "car", "cdr" }),
             obj_map_t ());
    }
    void
    import_list (env_t &env)
    {
        define (env, "null-c", jj_null_c (env));
        define (env, "cons-c", jj_cons_c (env));
    }
    void
    test_list ()
    {
        auto env = env_t ();

        import_list (env);

        define (env, "s1", make_shared <str_o> (env, "bye"));
        define (env, "s2", make_shared <str_o> (env, "world"));

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
            assert_pop_eq (env, make_shared <str_o> (env, "world"));
            assert_stack_size (env, 0);
        }
    }
    bool
    str_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == tagging (env, "str-t");
    }
    sig_t jj_str_print_sig = { "str-print", "string" };
    // -- str-t ->
    void jj_str_print (env_t &env, obj_map_t &obj_map)
    {
        auto str = static_pointer_cast <str_o> (obj_map ["string"]);
        cout << str->str;
    }
    void
    import_str (env_t &env)
    {
        define_prim (env,
                     jj_str_print_sig,
                     jj_str_print);
    }
    void
    test_str ()
    {

    }
    using obj_vect_t = vector <shared_ptr <obj_t>>;
    struct vect_o: obj_t
    {
        obj_vect_t vect;
        vect_o (env_t &env, obj_vect_t vect);
        bool equal (env_t &env, shared_ptr <obj_t> obj);
        // void print (env_t &env);
    };
    vect_o::vect_o (env_t &env, vector <shared_ptr <obj_t>> vect)
    {
        this->tag = tagging (env, "vect-t");
        this->vect = vect;
    }
    bool
    obj_equal (env_t &env,
               shared_ptr <obj_t> &lhs,
               shared_ptr <obj_t> &rhs)
    {
        return lhs->equal (env, rhs);
    }
    bool
    vect_equal (env_t &env,
                obj_vect_t &lhs,
                obj_vect_t &rhs)
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
        return vect_equal (env, this->vect, that->vect);
    }
    // void
    // vect_o::print (env_t &env)
    // {

    // }
    bool
    vect_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == tagging (env, "vect-t");
    }
    shared_ptr <vect_o>
    list_to_vect (env_t &env, shared_ptr <obj_t> a)
    {
        auto vect = obj_vect_t ();
        auto l = static_pointer_cast <data_o> (a);
        while (! null_p (env, l)) {
            vect.push_back (car (env, l));
            l = static_pointer_cast <data_o>
                (cdr (env, l));
        }
        return make_shared <vect_o> (env, vect);
    }
    sig_t jj_list_to_vect_sig = { "list-to-vect", "list" };
    // -- (list-t t) -> (vect-t t)
    void jj_list_to_vect (env_t &env, obj_map_t &obj_map)
    {
        env.obj_stack.push (list_to_vect (env, obj_map ["list"]));
    }
    shared_ptr <data_o>
    vect_to_list (env_t &env, shared_ptr <obj_t> a)
    {
        auto v = static_pointer_cast <vect_o> (a);
        auto vect = v->vect;
        auto result = null_c (env);
        auto begin = vect.rbegin ();
        auto end = vect.rend ();
        for (auto it = begin; it != end; it++)
            result = cons_c (env, *it, result);
        return result;
    }
    sig_t jj_vect_to_list_sig = { "vect-to-list", "vect" };
    // -- (vect-t t) -> (list-t t)
    void jj_vect_to_list (env_t &env, obj_map_t &obj_map)
    {
        env.obj_stack.push (vect_to_list (env, obj_map ["vect"]));
    }
    void
    import_vect (env_t &env)
    {
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

        // {
        //     env.report ();
        //     env.run ();
        //     env.report ();
        // }

        {
            env.run ();
            assert_stack_size (env, 1);
            assert_pop_eq (env, make_shared <str_o> (env, "world"));
            assert_stack_size (env, 0);
        }
    }
    void
    import_dict (env_t &env)
    {

    }
    void
    test_dict ()
    {

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
    shared_ptr <data_o>
    string_vector_to_string_list
    (env_t &env, string_vector_t &string_vector)
    {
        auto begin = string_vector.rbegin ();
        auto end = string_vector.rend ();
        auto collect = null_c (env);
        for (auto it = begin; it != end; it++) {
            auto obj = make_shared <str_o> (env, *it);
            collect = cons_c (env, obj, collect);
        }
        return collect;
    }
    shared_ptr <data_o>
    // scan_word_list (env_t &env, shared_ptr <str_o> code)
    scan_word_list (env_t &env, shared_ptr <obj_t> a)
    {
        auto code = static_pointer_cast <str_o> (a);
        auto word_vector = scan_word_vector (code->str);
        return string_vector_to_string_list
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
    shared_ptr <data_o>
    word_list_head_with_bar_ket_counter
    (env_t &env,
     shared_ptr <obj_t> a,
     string bar,
     string ket,
     size_t counter)
    {
        auto word_list = static_pointer_cast <data_o> (a);
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
    shared_ptr <data_o>
    word_list_head (env_t &env, shared_ptr <obj_t> a)
    {
        assert (cons_p (env, a));
        auto word_list = static_pointer_cast <data_o> (a);
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
        else {
            return cons_c (env, head, null_c (env));
        }
    }
    shared_ptr <data_o>
    word_list_rest_with_bar_ket_counter
    (env_t &env,
     shared_ptr <obj_t> a,
     string bar,
     string ket,
     size_t counter)
    {
        auto word_list = static_pointer_cast <data_o> (a);
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
    shared_ptr <data_o>
    word_list_rest (env_t &env, shared_ptr <obj_t> a)
    {
        assert (cons_p (env, a));
        auto word_list = static_pointer_cast <data_o> (a);
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
        else
            return static_pointer_cast <data_o>
                (cdr (env, word_list));
    }
    shared_ptr <data_o>
    word_list_drop_ket
    (env_t &env,
     shared_ptr <obj_t> a,
     string ket)
    {
        auto word_list = static_pointer_cast <data_o> (a);
        auto head = car (env, word_list);
        auto rest = cdr (env, word_list);
        auto next = cdr (env, cdr (env, word_list));
        auto car_rest = static_pointer_cast <str_o> (car (env, rest));
        auto word = car_rest->str;
        if (null_p (env, next)) {
            assert (word == ket);
            return cons_c (env, head, null_c (env));
        }
        else {
            return cons_c (env, head,
                           word_list_drop_ket (env, rest, ket));
        }
    }
    shared_ptr <data_o>
    parse_sexp_list (env_t &env, shared_ptr <obj_t> a);

    shared_ptr <obj_t>
    parse_sexp (env_t &env, shared_ptr <obj_t> a)
    {
        auto word_list = static_pointer_cast <data_o> (a);
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
        else if (word == "'")
            return cons_c (env, make_shared <str_o> (env, "quote"),
                           cons_c (env,
                                   parse_sexp (env, rest),
                                   null_c (env)));
        else if (word == "`")
            return cons_c (env, make_shared <str_o> (env, "partquote"),
                           cons_c (env,
                                   parse_sexp (env, rest),
                                   null_c (env)));
        else
            return head;
    }
    shared_ptr <data_o>
    parse_sexp_list (env_t &env, shared_ptr <obj_t> a)
    {
        auto word_list = static_pointer_cast <data_o> (a);
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
            auto l = vect_to_list (env, a);
            return "[" + sexp_list_repr (env, l) + "]";
        }
        else {
            auto str = static_pointer_cast <str_o> (a);
            assert (str->tag == tagging (env, "str-t"));
            return str->str;
        }
    }
    string
    sexp_list_repr (env_t &env, shared_ptr <obj_t> a)
    {
        auto sexp_list = static_pointer_cast <data_o> (a);
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
    sig_t jj_scan_word_list_sig = { "scan-word-list", "code" };
    // -- str-t -> (list-t str-t)
    void jj_scan_word_list (env_t &env, obj_map_t &obj_map)
    {
        env.obj_stack.push (scan_word_list (env, obj_map ["code"]));
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
    import_sexp (env_t &env)
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
    test_sexp_scan ()
    {
        auto code = "(cons-c <car> <cdr>)";
        auto string_vector = scan_word_vector (code);
        assert (string_vector.size () == 5);
        assert (string_vector [0] == "(");
        assert (string_vector [1] == "cons-c");
        assert (string_vector [2] == "<car>");
        assert (string_vector [3] == "<cdr>");
        assert (string_vector [4] == ")");
    }
    void
    test_sexp_list ()
    {
        auto env = env_t ();

        import_sexp (env);
        import_str (env);

        auto code =
            "(cons-c <car> <cdr>)"
            "(cons-c (cons-c <car> <cdr>) (cons-c <car> <cdr>))";
        auto word_list = scan_word_list
            (env, make_shared <str_o> (env, code));
        env.obj_stack.push (word_list);

        jo_vector_t jo_vector = {
            new ref_jo_t (boxing (env, "parse-sexp-list")),
            new apply_jo_t (1),
            new ref_jo_t (boxing (env, "sexp-list-repr")),
            new apply_jo_t (1),
            new ref_jo_t (boxing (env, "str-print")),
            new apply_jo_t (1),
        };

        env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

        // {
        //     env.report ();
        //     env.run ();
        //     env.report ();
        // }
    }
    void
    test_sexp_vect ()
    {
        auto env = env_t ();

        import_sexp (env);
        import_str (env);

        auto code = "(a [a b c] c) [a b c] (a [a b c] c)";
        auto word_list = scan_word_list
            (env, make_shared <str_o> (env, code));
        env.obj_stack.push (word_list);

        jo_vector_t jo_vector = {
            new ref_jo_t (boxing (env, "parse-sexp-list")),
            new apply_jo_t (1),
            new ref_jo_t (boxing (env, "sexp-list-repr")),
            new apply_jo_t (1),
            new ref_jo_t (boxing (env, "str-print")),
            new apply_jo_t (1),
        };

        env.frame_stack.push (new_frame_from_jo_vector (jo_vector));

        // {
        //     env.report ();
        //     env.run ();
        //     env.report ();
        // }
    }
    void
    test_sexp ()
    {
        test_sexp_scan ();
        test_sexp_list ();
        test_sexp_vect ();
    }
    using top_keyword_fn = function
        <void (env_t &, shared_ptr <data_o>)>;
    struct top_keyword_o: obj_t
    {
        top_keyword_fn fn;
        top_keyword_o (env_t &env, top_keyword_fn fn);
        // bool equal (env_t &env, shared_ptr <obj_t> obj);
        // void print (env_t &env);
    };
    top_keyword_o::
    top_keyword_o (env_t &env, top_keyword_fn fn)
    {
        this->tag = tagging (env, "top-keyword-t");
        this->fn = fn;
    }
    bool
    top_keyword_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == tagging (env, "top-keyword-t");
    }
    void
    define_top_keyword (env_t &env, name_t name, top_keyword_fn fn)
    {
        define (env, name, make_shared <top_keyword_o> (env, fn));
    }
    void
    sexp_list_eval (env_t &env, shared_ptr <obj_t> sexp_list);

    void tk_assign (env_t &env, shared_ptr <data_o> body)
    {
        auto head = static_pointer_cast <str_o> (car (env, body));
        auto rest = cdr (env, body);
        auto name = head->str;
        sexp_list_eval (env, rest);
        auto obj = env.obj_stack.top ();
        env.obj_stack.pop ();
        define (env, name, obj);
    }
    void
    import_top_keyword (env_t &env)
    {
        define_top_keyword (env, "=", tk_assign);
    }
    using keyword_fn = function
        <shared_ptr <jojo_t> (env_t &, shared_ptr <data_o>)>;
    struct keyword_o: obj_t
    {
        keyword_fn fn;
        keyword_o (env_t &env, keyword_fn fn);
        // bool equal (env_t &env, shared_ptr <obj_t> obj);
        // void print (env_t &env);
    };
    keyword_o::
    keyword_o (env_t &env, keyword_fn fn)
    {
        this->tag = tagging (env, "keyword-t");
        this->fn = fn;
    }
    bool
    keyword_p (env_t &env, shared_ptr <obj_t> a)
    {
        return a->tag == tagging (env, "keyword-t");
    }
    void
    define_keyword (env_t &env, name_t name, keyword_fn fn)
    {
        define (env, name, make_shared <keyword_o> (env, fn));
    }
    shared_ptr <jojo_t>
    k_lambda (env_t &env, shared_ptr <data_o> body)
    {

    }
    void
    import_keyword (env_t &env)
    {
        define_keyword (env, "lambda", k_lambda);
    }
    bool
    keyword_sexp_p (env_t &env, shared_ptr <obj_t> a)
    {
        if (! cons_p (env, a)) return false;
        auto sexp = static_pointer_cast <data_o> (a);
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
    jojo_append (shared_ptr <jojo_t> ante,
                 shared_ptr <jojo_t> succ)
    {
        auto jo_vector = jo_vector_t ();
        for (auto x: ante->jo_vector) jo_vector.push_back (x->copy ());
        for (auto x: succ->jo_vector) jo_vector.push_back (x->copy ());
        return make_shared <jojo_t> (jo_vector);
    }
    bool
    dot_string_p (string str)
    {
        auto pos = str.find (".");
        return (pos != string::npos);
    }
    shared_ptr <jojo_t>
    dot_string_compile (env_t &env, string str)
    {
        cout << "- WIP\n";
        exit (1);
    }
    shared_ptr <jojo_t>
    ref_compile (env_t &env, name_t name)
    {
        jo_vector_t jo_vector = {
            new ref_jo_t (boxing (env, name)),
        };
        return make_shared <jojo_t> (jo_vector);
    }
    shared_ptr <jojo_t>
    string_compile (env_t &env, string str)
    {
        if (dot_string_p (str)) {
            return dot_string_compile (env, str);
        }
        // else if (string_string_p) {
        // }
        // else if (int_string_p) {
        // }
        else {
            return ref_compile (env, str);
        }
    }
    shared_ptr <jojo_t>
    sexp_compile (env_t &env, shared_ptr <obj_t> sexp);

    shared_ptr <jojo_t>
    sexp_list_compile (env_t &env, shared_ptr <obj_t> a);

    shared_ptr <jojo_t>
    call_compile (env_t &env, shared_ptr <obj_t> sexp)
    {
        auto head = car (env, sexp);
        auto body = cdr (env, sexp);
        jo_vector_t jo_vector = {
            new apply_jo_t (list_length (env, body)),
        };
        auto jojo = make_shared <jojo_t> (jo_vector);
        jojo = jojo_append (sexp_compile (env, head), jojo);
        jojo = jojo_append (sexp_list_compile (env, body), jojo);
        return jojo;
    }
    shared_ptr <jojo_t>
    sexp_list_compile (env_t &env, shared_ptr <obj_t> a)
    {
        auto sexp_list = static_pointer_cast <data_o> (a);
        auto jojo = make_shared <jojo_t> (jo_vector_t ());
        if (null_p (env, sexp_list))
            return jojo;
        else {
            assert (cons_p (env, sexp_list));
            return jojo_append
                (sexp_compile (env, car (env, sexp_list)),
                 sexp_list_compile (env, cdr (env, sexp_list)));
        }
    }
    shared_ptr <jojo_t>
    sexp_compile (env_t &env, shared_ptr <obj_t> sexp)
    {
        if (str_p (env, sexp)) {
            auto str = static_pointer_cast <str_o> (sexp);
            return string_compile (env, str->str);
        }
        else if (keyword_sexp_p (env, sexp)) {
            auto head = static_pointer_cast <str_o> (car (env, sexp));
            auto body = static_pointer_cast <data_o> (cdr (env, sexp));
            auto name = head->str;
            auto fn = get_keyword_fn (env, name);
            return fn (env, body);
        }
        else {
            assert (cons_p (env, sexp));
            return call_compile (env, sexp);
        }
    }
    void
    import_compile (env_t &env)
    {

    }
    void
    test_compile ()
    {

    }
    bool
    top_keyword_sexp_p (env_t &env, shared_ptr <obj_t> a)
    {
        if (! cons_p (env, a)) return false;
        auto sexp = static_pointer_cast <data_o> (a);
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
            auto body = static_pointer_cast <data_o> (cdr (env, sexp));
            auto name = head->str;
            auto fn = get_top_keyword_fn (env, name);
            fn (env, body);
        }
        else {
            auto jojo = sexp_compile (env, sexp);
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
    code_eval (env_t &env, shared_ptr <obj_t> a)
    {
        auto code = static_pointer_cast <str_o> (a);
        auto word_list = scan_word_list (env, code);
        auto sexp_list = parse_sexp_list (env, word_list);
        sexp_list_eval (env, sexp_list);
    }
    sig_t jj_code_eval_sig = { "code-eval", "code" };
    void jj_code_eval (env_t &env, obj_map_t &obj_map)
    {
        code_eval (env, obj_map ["code"]);
    }
    void
    import_eval (env_t &env)
    {
        define_prim (env,
                     jj_code_eval_sig,
                     jj_code_eval);
    }
    void
    test_eval ()
    {

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

          // {
          //     env.report ();
          //     env.run ();
          //     env.report ();
          // }

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

          // {
          //     env.report ();
          //     env.run ();
          //     env.report ();
          // }

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
        test_list ();
        test_str ();
        test_vect ();
        test_dict ();
        test_sexp ();
        test_eval ();
    }
    void
    import_all (env_t &env)
    {
        import_test (env);
        import_bool (env);
        import_list (env);
        import_str (env);
        import_vect (env);
        import_dict (env);
        import_sexp (env);
        import_top_keyword (env);
        import_keyword (env);
        import_compile (env);
        import_eval (env);
    }
    void
    eval_file (env_t &env, string file_name)
    {
        auto input_file = ifstream (file_name);
        auto buffer = stringstream ();
        buffer << input_file.rdbuf ();
        auto code = make_shared <str_o> (env, buffer.str ());
        cout << buffer.str ();
        code_eval (env, code);
    }
    void
    the_story_begin (string_vector_t arg_vector)
    {
        auto env = env_t ();
        import_all (env);
        for (auto file_name: arg_vector)
            eval_file (env, file_name);
        env.run_and_report ();
    }
    int
    main (int argc, char **argv)
    {
        test_all ();
        auto arg_vector = string_vector_t ();
        for (auto i = 1; i < argc; i++) {
            arg_vector.push_back (string (argv[i]));
        }
        the_story_begin (arg_vector);
        return 0;
    }
