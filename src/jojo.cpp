    #include <iostream>
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
    using local_map_t = map<name_t, shared_ptr<obj_t>>;
    using jojo_t = vector<jo_t *>;
    struct jo_t
    {
        virtual void exe (env_t &env, local_map_t &local_map);
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
        local_map_t local_map;
        frame_t (jojo_t jojo, local_map_t local_map);
    };
    using name_map_t = map<name_t, shared_ptr<obj_t>>;
    using obj_stack_t = stack<shared_ptr<obj_t>>;
    using frame_stack_t = stack<frame_t>;
    struct env_t
    {
        name_map_t name_map;
        obj_stack_t obj_stack;
        frame_stack_t frame_stack;
        void step ();
        void run ();
        void report ();
    };
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
      using arg_vector_t = vector<name_t>;
      struct lambda_o: obj_t
      {
          jojo_t jojo;
          arg_vector_t arg_vector;
          local_map_t local_map;
          lambda_o (env_t &env,
                    arg_vector_t arg_vector,
                    jojo_t jojo,
                    local_map_t local_map);
          void apply (env_t &env);
      };
      lambda_o::
      lambda_o (env_t &env,
                arg_vector_t arg_vector,
                jojo_t jojo,
                local_map_t local_map)
      {
          this->t = "lambda-t";
          this->arg_vector = arg_vector;
          this->jojo = jojo;
          this->local_map = local_map;
      }
      void
      lambda_o::apply (env_t &env)
      {
          auto frame = frame_t (this->jojo, this->local_map);
          env.frame_stack.push (frame);
      }
      struct string_o: obj_t
      {
          string s;
          string_o (env_t &env, string s);
      };
      string_o::string_o (env_t &env, string s)
      {
          this->t = "string-t";
          this->s = s;
      }
      using field_map_t = map<name_t, shared_ptr<obj_t>>;
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
      using field_vector_t = vector<name_t>;
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
          shared_ptr<type_o> type;
          data_constructor_o (env_t &env, shared_ptr<type_o> type);
          void apply (env_t &env);
      };
      data_constructor_o::
      data_constructor_o (env_t &env, shared_ptr<type_o> type)
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
              shared_ptr<obj_t> obj = env.obj_stack.top ();
              env.obj_stack.pop ();
              field_map.insert (make_pair (name, obj));
          }
          auto data = make_shared<data_o>
              (env, this->type->type_tag, field_map);
          env.obj_stack.push (data);
      }
      void
      jojo_print (env_t &env,
                  jojo_t jojo)
      {
          for (auto &jo: jojo)
              cout << jo->repr (env) << " ";
      }
      void
      jojo_print_with_index (env_t &env,
                             jojo_t jojo,
                             size_t index)
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
      frame_t::frame_t (jojo_t jojo, local_map_t local_map)
      {
          this->index = 0;
          this->jojo = jojo;
          this->local_map = local_map;
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

          cout << "  - local_map # " << frame.local_map.size () << "\n";
          for (auto &kv: frame.local_map) {
              cout << "    " << kv.first << " : ";
              auto obj = kv.second;
              obj->print (env);
              cout << "\n";
          }
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
          cout << "- frame_stack # " << env.frame_stack.size () << "\n";
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
          cout << "- obj_stack # " << env.obj_stack.size () << "\n";
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

        // handle empty jojo
        if (index >= size) {
            this->frame_stack.pop ();
            return;
        }

        // get jo only for non empty jojo
        jo_t *jo = frame.jojo [index];

        frame.index++;

        // handle proper tail call
        if (index+1 == size)
            this->frame_stack.pop ();

        // since the last frame might be drop,
        //   we pass local_map the last frame
        //   as an extra argument.
        jo->exe (*this, frame.local_map);
    }
    void
    env_t::run ()
    {
        while (!this->frame_stack.empty ()) {
            this->report ();
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
      jo_t::exe (env_t &env, local_map_t &local_map)
      {
          cout << "fatal error : unknown jo" << "\n";
          exit (1);
      }
      string
      jo_t::repr (env_t &env)
      {
          return "(unknown)";
      }
      struct call_jo_t: jo_t
      {
          name_t name;

          call_jo_t (name_t name);

          void exe (env_t &env, local_map_t &local_map);
          string repr (env_t &env);
      };
      call_jo_t::call_jo_t (name_t name)
      {
          this->name = name;
      }
      void
      call_jo_t::exe (env_t &env, local_map_t &local_map)
      {
          // local_map first
          auto it = local_map.find (this->name);
          if (it != local_map.end ()) {
              env.obj_stack.push (it->second);
              return;
          }
          // name_map second
          it = env.name_map.find (this->name);
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
      call_jo_t::repr (env_t &env)
      {
          return "(call " + this->name + ")";
      }
      struct lambda_jo_t: jo_t
      {
          jojo_t jojo;
          arg_vector_t arg_vector;
          lambda_jo_t (arg_vector_t arg_vector, jojo_t jojo);
          void exe (env_t &env, local_map_t &local_map);
          string repr (env_t &env);
      };
      lambda_jo_t::lambda_jo_t (arg_vector_t arg_vector, jojo_t jojo)
      {
          this->arg_vector = arg_vector;
          this->jojo = jojo;
      }
      void
      lambda_jo_t::exe (env_t &env, local_map_t &local_map)
      {
          // create lambda_o by closure
          // and push it to obj_stack
          auto frame = env.frame_stack.top ();
          // ><><>< need to copy the local_map
          auto lambda = make_shared<lambda_o>
              (env, this->arg_vector, this->jojo, frame.local_map);
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
          void exe (env_t &env, local_map_t &local_map);
          string repr (env_t &env);
      };
      field_jo_t::field_jo_t (name_t name)
      {
          this->name = name;
      }
      void
      field_jo_t::exe (env_t &env, local_map_t &local_map)
      {
          auto obj = env.obj_stack.top ();
          env.obj_stack.pop ();
          auto data = static_pointer_cast<data_o> (obj);
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
          void exe (env_t &env, local_map_t &local_map);
          string repr (env_t &env);
      };
      void
      apply_jo_t::exe (env_t &env, local_map_t &local_map)
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
    int
    main ()
    {
        auto env = env_t ();
        field_map_t field_map = {
            {"f1", make_shared<string_o> (env, "fs1")},
            {"f2", make_shared<string_o> (env, "fs2")},
        };

        env.name_map = {
            {"k1", make_shared<string_o> (env, "s1")},
            {"k2", make_shared<string_o> (env, "s2")},
            {"d1", make_shared<data_o> (env, "d-t", field_map)},
        };

        jojo_t jojo = {
            new call_jo_t ("k1"),
            new call_jo_t ("k2"),
            new lambda_jo_t ({ "x", "y" },
                             { new call_jo_t ("k1"),
                               new call_jo_t ("k2") }),
            new apply_jo_t,
            new call_jo_t ("d1"),
            new call_jo_t ("d1"),
            new field_jo_t ("f1"),
        };

        auto frame = frame_t (jojo, local_map_t ());
        env.frame_stack.push (frame);
        env.report ();
        env.run ();
        env.report ();
    }
