    #include <iostream>
    #include <algorithm>
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
    using jojo_t = vector <jo_t *>;
    struct jo_t
    {
        virtual void exe (env_t &env, local_scope_t &local_scope);
        virtual string repr (env_t &env);
    };
    using tag_t = string;
    struct obj_t
    {
        tag_t t;
        virtual ~obj_t ();
        virtual void print (env_t &env);
        virtual void apply (env_t &env, size_t arity);
    };
    using obj_map_t = map <name_t, shared_ptr <obj_t>>;
    using obj_vector_t = vector <shared_ptr <obj_t>>;
    struct frame_t
    {
        size_t index;
        jojo_t jojo;
        local_scope_t local_scope;
        frame_t (jojo_t jojo, local_scope_t local_scope);
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
        void step ();
        void run ();
        void report ();
    };
      struct lambda_jo_t: jo_t
      {
          name_vector_t name_vector;
          jojo_t jojo;
          lambda_jo_t (name_vector_t name_vector, jojo_t jojo);
          void exe (env_t &env, local_scope_t &local_scope);
          string repr (env_t &env);
      };
      void
      bind_vector_print (env_t &env, bind_vector_t bind_vector)
      {
          for (auto it = bind_vector.rbegin ();
               it != bind_vector.rend ();
               it++) {
              cout << "(#"
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
          cout << "- local_scope\n";
          for (auto it = local_scope.rbegin ();
               it != local_scope.rend ();
               it++) {
              cout << "  - level # "
                   << distance(local_scope.rbegin (), it)
                   << " : ";
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
          cout << this->t;
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
          lambda_jo_t *lambda_jo;
          bind_vector_t bind_vector;
          local_scope_t local_scope;
          lambda_o (env_t &env,
                    lambda_jo_t *lambda_jo,
                    bind_vector_t bind_vector,
                    local_scope_t local_scope);
          void apply (env_t &env, size_t arity);
      };
      lambda_o::
      lambda_o (env_t &env,
                lambda_jo_t *lambda_jo,
                bind_vector_t bind_vector,
                local_scope_t local_scope)
      {
          this->t = "lambda-t";
          this->lambda_jo = lambda_jo;
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
          auto size = this->lambda_jo->name_vector.size ();
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
                  (this->lambda_jo->jojo, local_scope);
              env.frame_stack.push (frame);
          }
          else if (arity < lack) {
              auto obj_vector = pick_up_obj_vector
                  (env, arity);
              auto bind_vector = bind_vector_merge_obj_vector
                  (this->bind_vector, obj_vector);
              auto lambda = make_shared <lambda_o>
                  (env,
                   this->lambda_jo,
                   bind_vector,
                   this->local_scope);
              env.obj_stack.push (lambda);
          }
          else {
              cout << "- fatal error : over-arity apply" << "\n"
                   << "  arity > lack" << "\n"
                   << "  arity : " << arity << "\n"
                   << "  lack : " << lack << "\n"
                   << "\n";
              exit (1);
          }
      }
      struct string_o: obj_t
      {
          string s;
          string_o (env_t &env, string s);
          void print (env_t &env);
      };
      string_o::string_o (env_t &env, string s)
      {
          this->t = "string-t";
          this->s = s;
      }
      void string_o::print (env_t &env)
      {
          cout << '"' << this->s << '"';
      }
      struct data_o: obj_t
      {
          obj_map_t obj_map;
          data_o (env_t &env, tag_t t, obj_map_t obj_map);
      };
      data_o::data_o (env_t &env, tag_t t, obj_map_t obj_map)
      {
          this->t = t;
          this->obj_map = obj_map;
      }
      struct type_o: obj_t
      {
          tag_t type_tag;
          name_vector_t name_vector;
          type_o (env_t &env,
                  tag_t type_tag,
                  name_vector_t name_vector);
      };
      type_o::
      type_o (env_t &env,
              tag_t type_tag,
              name_vector_t name_vector)
      {
          this->t = "type-t";
          this->type_tag = type_tag;
          this->name_vector = name_vector;
      }
      struct data_cons_o: obj_t
      {
          shared_ptr <type_o> type;
          data_cons_o (env_t &env, shared_ptr <type_o> type);
          void apply (env_t &env, size_t arity);
      };
      data_cons_o::
      data_cons_o (env_t &env, shared_ptr <type_o> type)
      {
          this->t = "data-cons-t";
          this->type = type;
      }
      void
      data_cons_o::apply (env_t &env, size_t arity)
      {
          auto obj_map = obj_map_t ();
          name_vector_t &name_vector = this->type->name_vector;
          for (auto it = name_vector.rbegin();
               it != name_vector.rend();
               it++) {
              name_t name = *it;
              auto obj = env.obj_stack.top ();
              env.obj_stack.pop ();
              auto bind = make_pair (name, obj);
              obj_map.insert (bind);
          }
          auto data = make_shared <data_o>
              (env, this->type->type_tag, obj_map);
          env.obj_stack.push (data);
      }
      void
      jojo_print (env_t &env, jojo_t jojo)
      {
          for (auto &jo: jojo)
              cout << jo->repr (env) << " ";
      }
      void
      jojo_print_with_index (env_t &env, jojo_t jojo, size_t index)
      {
          for (auto it = jojo.begin ();
               it != jojo.end ();
               it++) {
              size_t it_index = it - jojo.begin ();
              jo_t *jo = *it;
              if (index == it_index) {
                  cout << "->> " << jo->repr (env) << " ";
              }
              else {
                  cout << jo->repr (env) << " ";
              }
          }
      }
      frame_t::frame_t (jojo_t jojo, local_scope_t local_scope)
      {
          this->index = 0;
          this->jojo = jojo;
          this->local_scope = local_scope;
      }
      void
      frame_report (env_t &env, shared_ptr <frame_t> frame)
      {
          cout << "  - ["
               << frame->index+1
               << "/"
               << frame->jojo.size ()
               << "] ";
          jojo_print_with_index (env, frame->jojo, frame->index);
          cout << "\n";

          cout << "  - local_scope # "
               << frame->local_scope.size ()
               << "\n";
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
              env.box_map.insert (make_pair (name, box));
              return box;
          }
      }
      void
      box_map_report (env_t &env)
      {
          cout << "- box_map # " << env.box_map.size () << "\n";
          for (auto &kv: env.box_map) {
              cout << "  " << kv.first << " : ";
              auto box = kv.second;
              box->obj->print (env);
              cout << "\n";
          }
      }
      void
      frame_stack_report (env_t &env)
      {
          cout << "- frame_stack # "
               << env.frame_stack.size ()
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
          cout << "- obj_stack # "
               << env.obj_stack.size ()
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
        size_t size = frame->jojo.size ();
        size_t index = frame->index;
        // it is assumed that jojo in frame are not empty
        jo_t *jo = frame->jojo [index];
        frame->index++;
        // handle proper tail call
        if (index+1 == size) this->frame_stack.pop ();
        // since the last frame might be drop,
        //   we pass last local_scope as an extra argument.
        jo->exe (*this, frame->local_scope);
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
          // return "(ref " + this->name + ")";
          return "(ref)";
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
          // {
          //     local_scope_print (env, local_scope);
          //     cout << "- local_ref_jo_t::exe\n"
          //          << "  this->level : " << this->level << "\n"
          //          << "  this->index : " << this->index << "\n"
          //          << "  bind.first : " << bind.first << "\n";
          //     cout << "  bind.second->print () : ";
          //     bind.second->print (env);
          //     cout << "\n";
          //     cout << "\n";
          // }
          auto obj = bind.second;
          env.obj_stack.push (obj);
      }
      string
      local_ref_jo_t::repr (env_t &env)
      {
          return "(local-ref " +
              to_string (this->level) + " " +
              to_string (this->index) + ")";
      }
      lambda_jo_t::lambda_jo_t (name_vector_t name_vector, jojo_t jojo)
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
              (env, this,
               bind_vector_from_name_vector (this->name_vector),
               frame->local_scope);
          env.obj_stack.push (lambda);
      }
      string
      lambda_jo_t::repr (env_t &env)
      {
          return "(lambda)";
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
          return "(field " + this->name + ")";
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
          return "(apply)";
      }
      void
      test_step ()
      {
          auto env = env_t ();

          env.box_map = {
              {"string-1", new box_t (make_shared <string_o> (env, "bye"))},
              {"string-2", new box_t (make_shared <string_o> (env, "world"))},
          };

          jojo_t jojo = {
              new ref_jo_t (boxing (env, "string-1")),
              new ref_jo_t (boxing (env, "string-2")),
          };
          auto frame = make_shared <frame_t> (jojo, local_scope_t ());
          env.frame_stack.push (frame);
          env.run ();

          assert (env.obj_stack.size () == 2);

          auto string_2 = static_pointer_cast <string_o>
              (env.obj_stack.top ());
          assert (string_2->t == "string-t");
          assert (string_2->s == "world");
          env.obj_stack.pop ();

          assert (env.obj_stack.size () == 1);

          auto string_1 = static_pointer_cast <string_o>
              (env.obj_stack.top ());
          assert (string_1->t == "string-t");
          assert (string_1->s == "bye");
          env.obj_stack.pop ();

          assert (env.obj_stack.size () == 0);
      }
      void
      test_data ()
      {
          auto env = env_t ();

          obj_map_t obj_map = {
              {"field-1", make_shared <string_o> (env, "bye")},
              {"field-2", make_shared <string_o> (env, "world")},
          };

          env.box_map = {
              {"data-1", new box_t (make_shared <data_o> (env, "data-1-t", obj_map))},
          };

          jojo_t jojo = {
              new ref_jo_t (boxing (env, "data-1")),
              new field_jo_t ("field-1"),
              new ref_jo_t (boxing (env, "data-1")),
              new field_jo_t ("field-2"),
              new ref_jo_t (boxing (env, "data-1")),
          };
          auto frame = make_shared <frame_t> (jojo, local_scope_t ());
          env.frame_stack.push (frame);
          env.run ();

          assert (env.obj_stack.size () == 3);

          auto data_1 = static_pointer_cast <data_o>
              (env.obj_stack.top ());
          assert (data_1->t == "data-1-t");
          env.obj_stack.pop ();

          assert (env.obj_stack.size () == 2);

          auto string_2 = static_pointer_cast <string_o>
              (env.obj_stack.top ());
          assert (string_2->t == "string-t");
          assert (string_2->s == "world");
          env.obj_stack.pop ();

          assert (env.obj_stack.size () == 1);

          auto string_1 = static_pointer_cast <string_o>
              (env.obj_stack.top ());
          assert (string_1->t == "string-t");
          assert (string_1->s == "bye");
          env.obj_stack.pop ();

          assert (env.obj_stack.size () == 0);
      }
      void
      test_apply ()
      {
          auto env = env_t ();

          env.box_map = {
              {"string-1", new box_t (make_shared <string_o> (env, "bye"))},
              {"string-2", new box_t (make_shared <string_o> (env, "world"))},
          };

          jojo_t jojo = {
              new ref_jo_t (boxing (env, "string-1")),
              new ref_jo_t (boxing (env, "string-2")),
              new lambda_jo_t ({ "x", "y" },
                               { new local_ref_jo_t (0, 0),
                                 new local_ref_jo_t (0, 1) }),
              new apply_jo_t (2),
          };
          auto frame = make_shared <frame_t> (jojo, local_scope_t ());
          env.frame_stack.push (frame);

          // {
          //     env.report ();
          //     env.run ();
          //     env.report ();
          // }

          {
              env.run ();

              assert (env.obj_stack.size () == 2);

              auto string_2 = static_pointer_cast <string_o>
                  (env.obj_stack.top ());
              assert (string_2->t == "string-t");
              assert (string_2->s == "world");
              env.obj_stack.pop ();

              assert (env.obj_stack.size () == 1);

              auto string_1 = static_pointer_cast <string_o>
                  (env.obj_stack.top ());
              assert (string_1->t == "string-t");
              assert (string_1->s == "bye");
              env.obj_stack.pop ();

              assert (env.obj_stack.size () == 0);
          }
      }
      void
      test_curry ()
      {
          auto env = env_t ();

          env.box_map = {
              {"string-1", new box_t (make_shared <string_o> (env, "bye"))},
              {"string-2", new box_t (make_shared <string_o> (env, "world"))},
          };

          jojo_t jojo = {
              new ref_jo_t (boxing (env, "string-1")),
              new ref_jo_t (boxing (env, "string-2")),
              new lambda_jo_t ({ "x", "y" },
                               { new local_ref_jo_t (0, 0),
                                 new local_ref_jo_t (0, 1) }),
              new apply_jo_t (1),
              new apply_jo_t (1),
          };
          auto frame = make_shared <frame_t> (jojo, local_scope_t ());
          env.frame_stack.push (frame);

          // {
          //     env.report ();
          //     env.run ();
          //     env.report ();
          // }

          {
              env.run ();

              assert (env.obj_stack.size () == 2);

              auto string_1 = static_pointer_cast <string_o>
                  (env.obj_stack.top ());
              assert (string_1->t == "string-t");
              assert (string_1->s == "bye");
              env.obj_stack.pop ();

              assert (env.obj_stack.size () == 1);

              auto string_2 = static_pointer_cast <string_o>
                  (env.obj_stack.top ());
              assert (string_2->t == "string-t");
              assert (string_2->s == "world");
              env.obj_stack.pop ();

              assert (env.obj_stack.size () == 0);
          }
      }
      void
      test_data_cons ()
      {

      }
      void test_all ()
      {
          test_step ();
          test_data ();
          test_apply ();
          test_curry ();
          test_data_cons ();
      }
    int
    main ()
    {
        test_all ();
        return 0;
    }
