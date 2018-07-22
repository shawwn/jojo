    #include <iostream>
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
    using bind_t = pair <name_t, shared_ptr <obj_t>>;
    // index from end
    using local_level_t = vector <bind_t>;
    // index from end
    using local_scope_t = vector <local_level_t>;
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
        virtual void apply (env_t &env);
    };
    struct frame_t
    {
        size_t index;
        jojo_t jojo;
        local_scope_t local_scope;
        frame_t (jojo_t jojo, local_scope_t local_scope);
    };
    using name_map_t = map <name_t, shared_ptr <obj_t>>;
    using obj_stack_t = stack <shared_ptr <obj_t>>;
    using frame_stack_t = stack <frame_t>;
    struct env_t
    {
        name_map_t name_map;
        obj_stack_t obj_stack;
        frame_stack_t frame_stack;
        void step ();
        void run ();
        void report ();
    };
      void
      local_level_print (env_t &env, local_level_t local_level)
      {
          for (auto it = local_level.rbegin ();
               it != local_level.rend ();
               it++) {
              cout << "(#"
                   << distance(local_level.rbegin (), it)
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
              local_level_print (env, *it);
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
      obj_t::apply (env_t &env)
      {
          cout << "fatal error : applying non applicable object" << "\n";
          exit (1);
      }
      using arg_vector_t = vector <name_t>;
      local_level_t
      local_level_from_arg_vector (env_t &env, arg_vector_t arg_vector)
      {
          auto local_level = local_level_t ();
          for (auto it = arg_vector.rbegin ();
               it != arg_vector.rend ();
               it++) {
              name_t name = *it;
              auto obj = env.obj_stack.top ();
              env.obj_stack.pop ();
              auto bind = make_pair (name, obj);
              local_level.push_back (bind);
          }
          return local_level;
      }
      struct lambda_o: obj_t
      {
          jojo_t jojo;
          arg_vector_t arg_vector;
          local_scope_t local_scope;
          lambda_o (env_t &env,
                    arg_vector_t arg_vector,
                    jojo_t jojo,
                    local_scope_t local_scope);
          void apply (env_t &env);
      };
      lambda_o::
      lambda_o (env_t &env,
                arg_vector_t arg_vector,
                jojo_t jojo,
                local_scope_t local_scope)
      {
          this->t = "lambda-t";
          this->arg_vector = arg_vector;
          this->jojo = jojo;
          this->local_scope = local_scope;
      }
      void
      lambda_o::apply (env_t &env)
      {
          auto local_scope = this->local_scope;
          local_scope.push_back
              (local_level_from_arg_vector (env, this->arg_vector));
          auto frame = frame_t (this->jojo, local_scope);
          env.frame_stack.push (frame);
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
      using field_map_t = map <name_t, shared_ptr <obj_t>>;
      struct data_o: obj_t
      {
          field_map_t field_map;
          data_o (env_t &env, tag_t t, field_map_t field_map);
      };
      data_o::data_o (env_t &env, tag_t t, field_map_t field_map)
      {
          this->t = t;
          this->field_map = field_map;
      }
      using field_vector_t = vector <name_t>;
      struct type_o: obj_t
      {
          tag_t type_tag;
          field_vector_t field_vector;
          type_o (env_t &env,
                  tag_t type_tag,
                  field_vector_t field_vector);
      };
      type_o::
      type_o (env_t &env,
              tag_t type_tag,
              field_vector_t field_vector)
      {
          this->t = "type-t";
          this->type_tag = type_tag;
          this->field_vector = field_vector;
      }
      struct data_constructor_o: obj_t
      {
          shared_ptr <type_o> type;
          data_constructor_o (env_t &env, shared_ptr <type_o> type);
          void apply (env_t &env);
      };
      data_constructor_o::
      data_constructor_o (env_t &env, shared_ptr <type_o> type)
      {
          this->t = "data-constructor-t";
          this->type = type;
      }
      void
      data_constructor_o::apply (env_t &env)
      {
          auto field_map = field_map_t ();
          field_vector_t &field_vector = this->type->field_vector;
          for (auto it = field_vector.rbegin();
               it != field_vector.rend();
               it++) {
              name_t name = *it;
              auto obj = env.obj_stack.top ();
              env.obj_stack.pop ();
              auto bind = make_pair (name, obj);
              field_map.insert (bind);
          }
          auto data = make_shared <data_o>
              (env, this->type->type_tag, field_map);
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
      frame_report (env_t &env, frame_t frame)
      {
          cout << "  - ["
               << frame.index+1
               << "/"
               << frame.jojo.size ()
               << "] ";
          jojo_print_with_index (env, frame.jojo, frame.index);
          cout << "\n";

          cout << "  - local_scope # "
               << frame.local_scope.size ()
               << "\n";
      }
      void
      name_map_report (env_t &env)
      {
          cout << "- name_map # " << env.name_map.size () << "\n";
          for (auto &kv: env.name_map) {
              cout << "  " << kv.first << " : ";
              auto obj = kv.second;
              obj->print (env);
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
        frame_t &frame = this->frame_stack.top ();
        size_t size = frame.jojo.size ();
        size_t index = frame.index;
        // it is assumed that jojo in frame are not empty
        jo_t *jo = frame.jojo [index];
        frame.index++;
        // handle proper tail call
        if (index+1 == size)
            this->frame_stack.pop ();
        // since the last frame might be drop,
        //   we pass local_scope the last frame
        //   as an extra argument.
        jo->exe (*this, frame.local_scope);
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
        name_map_report (*this);
        frame_stack_report (*this);
        obj_stack_report (*this);
        cout << "\n";
    }
      void
      jo_t::exe (env_t &env, local_scope_t &local_scope)
      {
          cout << "fatal error : unknown jo" << "\n";
          exit (1);
      }
      string
      jo_t::repr (env_t &env)
      {
          return "(unknown)";
      }
      struct ref_jo_t: jo_t
      {
          name_t name;
          ref_jo_t (name_t name);
          void exe (env_t &env, local_scope_t &local_scope);
          string repr (env_t &env);
      };
      ref_jo_t::ref_jo_t (name_t name)
      {
          this->name = name;
      }
      void
      ref_jo_t::exe (env_t &env, local_scope_t &local_scope)
      {
          auto it = env.name_map.find (this->name);
          if (it != env.name_map.end ()) {
              env.obj_stack.push (it->second);
              return;
          }
          cout << "fatal error ! unknown name : "
               << this->name
               << "\n";
          exit (1);
      }
      string
      ref_jo_t::repr (env_t &env)
      {
          return "(ref " + this->name + ")";
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
          auto local_level =
              vector_rev_ref (local_scope, this->level);
          auto bind =
              vector_rev_ref (local_level, this->index);
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
      struct lambda_jo_t: jo_t
      {
          jojo_t jojo;
          arg_vector_t arg_vector;
          lambda_jo_t (arg_vector_t arg_vector, jojo_t jojo);
          void exe (env_t &env, local_scope_t &local_scope);
          string repr (env_t &env);
      };
      lambda_jo_t::lambda_jo_t (arg_vector_t arg_vector, jojo_t jojo)
      {
          this->arg_vector = arg_vector;
          this->jojo = jojo;
      }
      void
      lambda_jo_t::exe (env_t &env, local_scope_t &local_scope)
      {
          auto frame = env.frame_stack.top ();
          auto lambda = make_shared <lambda_o>
              (env,
               this->arg_vector,
               this->jojo,
               frame.local_scope);
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
          auto it = data->field_map.find (this->name);
          if (it != data->field_map.end ()) {
              env.obj_stack.push (it->second);
              return;
          }
          cout << "fatal error ! unknown field : "
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
          void exe (env_t &env, local_scope_t &local_scope);
          string repr (env_t &env);
      };
      void
      apply_jo_t::exe (env_t &env, local_scope_t &local_scope)
      {
          auto obj = env.obj_stack.top ();
          env.obj_stack.pop ();
          obj->apply (env);
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

          env.name_map = {
              {"string-1", make_shared <string_o> (env, "bye")},
              {"string-2", make_shared <string_o> (env, "world")},
          };

          jojo_t jojo = {
              new ref_jo_t ("string-1"),
              new ref_jo_t ("string-2"),
          };
          auto frame = frame_t (jojo, local_scope_t ());
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

          field_map_t field_map = {
              {"field-1", make_shared <string_o> (env, "bye")},
              {"field-2", make_shared <string_o> (env, "world")},
          };

          env.name_map = {
              {"data-1", make_shared <data_o> (env, "data-1-t", field_map)},
          };

          jojo_t jojo = {
              new ref_jo_t ("data-1"),
              new field_jo_t ("field-1"),
              new ref_jo_t ("data-1"),
              new field_jo_t ("field-2"),
              new ref_jo_t ("data-1"),
          };
          auto frame = frame_t (jojo, local_scope_t ());
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

          env.name_map = {
              {"string-1", make_shared <string_o> (env, "bye")},
              {"string-2", make_shared <string_o> (env, "world")},
          };

          jojo_t jojo = {
              new ref_jo_t ("string-1"),
              new ref_jo_t ("string-2"),
              new lambda_jo_t ({ "x", "y" },
                               { new local_ref_jo_t (0, 0),
                                 new local_ref_jo_t (0, 0),
                                 new local_ref_jo_t (0, 1),
                                 new local_ref_jo_t (0, 0) }),
              new apply_jo_t,
          };
          auto frame = frame_t (jojo, local_scope_t ());
          env.frame_stack.push (frame);
          env.report ();
          env.run ();
          env.report ();
      }
    int
    main ()
    {
        test_step ();
        test_data ();
        test_apply ();
        return 0;
    }
