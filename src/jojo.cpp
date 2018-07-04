    #include <iostream>
    #include <vector>
    #include <map>
    #include <set>
    #include <stack>
    using namespace std;
    struct env_t;
    struct obj_t;
    struct jo_t;
    using name_t = string;
    using local_map_t = map<name_t, obj_t *>;
    using jojo_t = vector<jo_t *>;
      struct jo_t
      {
          virtual
          void exe (env_t *env, local_map_t *local_map);
          virtual
          string repr (env_t *env);
      };
      struct call_jo_t: jo_t
      {
          name_t name;

          call_jo_t (name_t name);

          void exe (env_t *env, local_map_t *local_map);
          string repr (env_t *env);
      };
      struct let_jo_t: jo_t
      {
          name_t name;

          let_jo_t (name_t name);

          void exe (env_t *env, local_map_t *local_map);
          string repr (env_t *env);
      };
      struct lambda_jo_t: jo_t
      {
          jojo_t *jojo;

          lambda_jo_t (jojo_t *jojo);
          lambda_jo_t (jojo_t jojo);

          void exe (env_t *env, local_map_t *local_map);
          string repr (env_t *env);
      };
      struct field_jo_t: jo_t
      {
          name_t name;

          field_jo_t (name_t name);

          void exe (env_t *env, local_map_t *local_map);
          string repr (env_t *env);
      };
      using tag_t = string;
      struct obj_t
      {
          tag_t t;
          virtual
          void apply (env_t *env);
          virtual
          void print (env_t *env);
      };
      struct lambda_obj_t: obj_t
      {
          jojo_t *jojo;
          local_map_t *local_map;

          lambda_obj_t (jojo_t* jojo, local_map_t *local_map);

          void apply (env_t *env);
      };
      typedef void (*prim_fn) (env_t *);
      struct primitive_obj_t: obj_t
      {
          prim_fn fn;

          primitive_obj_t (prim_fn fn);

          void apply (env_t *env);
      };
      struct int_obj_t: obj_t
      {
          int i;

          int_obj_t (int i);
      };
      struct str_obj_t: obj_t
      {
          string s;

          str_obj_t (string s);
      };
      using field_map_t = map<name_t, obj_t *>;
      struct data_obj_t: obj_t
      {
          field_map_t *field_map;

          data_obj_t (tag_t t, field_map_t *field_map);
      };
    struct frame_t
    {
       size_t index;
       jojo_t *jojo;
       local_map_t *local_map;

       frame_t (jojo_t *jojo, local_map_t *local_map);
    };
    using name_map_t = map<name_t, obj_t *>;
    using obj_stack_t = stack<obj_t *>;
    using frame_stack_t = stack<frame_t *>;
    struct env_t
    {
        name_map_t *name_map;
        obj_stack_t *obj_stack;
        frame_stack_t *frame_stack;

        env_t ();

        void step ();
        void run ();
        void report ();
    };
      int_obj_t::int_obj_t (int i)
      {
          this->t = "int-t";
          this->i = i;
      }
      str_obj_t::str_obj_t (string s)
      {
          this->t = "string-t";
          this->s = s;
      }
      lambda_obj_t::lambda_obj_t (jojo_t* jojo, local_map_t *local_map)
      {
          this->t = "lambda-t";
          this->jojo = jojo;
          this->local_map = local_map;
      }
      primitive_obj_t::primitive_obj_t (prim_fn fn)
      {
          this->t = "primitive-t";
          this->fn = fn;
      }
      data_obj_t::data_obj_t (tag_t t, field_map_t *field_map)
      {
          this->t = t;
          this->field_map = field_map;
      }
      void
      obj_t::print (env_t *env)
      {
          cout << this->t;
      }
    void
    obj_print (env_t *env, obj_t *obj)
    {
        if (obj->t == "lambda-t") {
            cout << "lambda-t";
        }
        else if (obj->t == "int-t") {
            cout << "int-t";
        }
        else if (obj->t == "string-t") {
            cout << "string-t";
        }
        else {
            cout << obj->t;
        }
    }
      void
      obj_t::apply (env_t *env)
      {
          env->obj_stack->push (this);
      }
      void
      lambda_obj_t::apply (env_t *env)
      {
          frame_t *frame = new frame_t (this->jojo, this->local_map);
          env->frame_stack->push (frame);
      }
      void
      primitive_obj_t::apply (env_t *env)
      {
          this->fn (env);
      }
      void
      jojo_print (env_t *env,
                  jojo_t *jojo)
      {
          for (auto &jo: *jojo)
              cout << jo->repr (env) << " ";
      }
      void
      jojo_print_with_index (env_t *env,
                             jojo_t *jojo,
                             size_t index)
      {
          jojo_t::iterator it;
          for (it = jojo->begin ();
               it != jojo->end ();
               it++) {
              size_t it_index = it - jojo->begin ();
              jo_t *jo = *it;
              if (index == it_index) {
                  cout << "->> " << jo->repr (env) << " ";
              }
              else {
                  cout << jo->repr (env) << " ";
              }
          }
      }
      frame_t::frame_t (jojo_t *jojo, local_map_t *local_map)
      {
          this->index = 0;
          this->jojo = jojo;
          this->local_map = local_map;
      }
      void
      frame_report (env_t *env, frame_t *frame)
      {
          cout << "  - ["
               << frame->index+1
               << "/"
               << frame->jojo->size ()
               << "] ";
          jojo_print_with_index (env, frame->jojo, frame->index);
          cout << "\n";

          cout << "  - local_map # " << frame->local_map->size () << "\n";
          for (auto &kv: *(frame->local_map)) {
              cout << "    " << kv.first << " : ";
              kv.second->print (env);
              cout << "\n";
          }
      }
      void
      name_map_report (env_t *env)
      {
          cout << "- name_map # " << env->name_map->size () << "\n";
          for (auto &kv: *(env->name_map)) {
              cout << "  " << kv.first << " : ";
              kv.second->print (env);
              cout << "\n";
          }
      }
      void
      frame_stack_report (env_t *env)
      {
          cout << "- frame_stack # " << env->frame_stack->size () << "\n";
          frame_stack_t frame_stack = *(env->frame_stack);
          while (!frame_stack.empty ()) {
             frame_t *frame = frame_stack.top ();
             frame_report (env, frame);
             frame_stack.pop ();
          }
      }
      void
      obj_stack_report (env_t *env)
      {
          cout << "- obj_stack # " << env->obj_stack->size () << "\n";
          cout << "  ";
          obj_stack_t obj_stack = *(env->obj_stack);
          while (!obj_stack.empty ()) {
              obj_t *obj = obj_stack.top ();
              obj->print (env);
              cout << " ";
              obj_stack.pop ();
          }
          cout << "\n";
      }
    env_t::env_t ()
    {
        this->name_map = new name_map_t;
        this->obj_stack = new obj_stack_t;
        this->frame_stack = new frame_stack_t;
    }
    void
    env_t::step ()
    {
        frame_t *frame = this->frame_stack->top ();
        size_t size = frame->jojo->size ();
        size_t index = frame->index;

        // handle empty jojo
        if (index >= size) {
            this->frame_stack->pop ();
            return;
        }

        // get jo only for non empty jojo
        jojo_t jojo = *(frame->jojo);
        jo_t *jo = jojo [index];

        frame->index++;

        // handle proper tail call
        if (index+1 == size)
            this->frame_stack->pop ();

        // since the last frame might be drop,
        //   we pass local_map the last frame
        //   as an extra argument.
        jo->exe (this, frame->local_map);
    }
    void
    env_t::run ()
    {
        while (!this->frame_stack->empty ())
            this->step ();
    }
    void
    env_t::report ()
    {
        name_map_report (this);
        frame_stack_report (this);
        obj_stack_report (this);
        cout << "\n";
    }
      void
      jo_t::exe (env_t *env, local_map_t *local_map)
      {
          cout << "fatal error : unknown jo" << "\n";
      }
      void
      call_jo_t::exe (env_t *env, local_map_t *local_map)
      {
          // local_map first
          auto it = local_map->find (this->name);
          if (it != local_map->end ()) {
              it->second->apply (env);
              return;
          }
          // name_map second
          it = env->name_map->find (this->name);
          if (it != env->name_map->end ()) {
              it->second->apply (env);
              return;
          }
          cout << "fatal error ! unknown name : "
               << this->name
               << "\n";
      }
      void
      let_jo_t::exe (env_t *env, local_map_t *local_map)
      {
           obj_t *obj = env->obj_stack->top ();
           env->obj_stack->pop ();
           local_map->insert (pair<name_t, obj_t *> (this->name, obj));
      }
      void
      lambda_jo_t::exe (env_t *env, local_map_t *local_map)
      {
          // create lambda_obj_t by closure
          // and push it to obj_stack
          frame_t *frame = env->frame_stack->top ();
          lambda_obj_t *lambda_obj =
              new lambda_obj_t (this->jojo, frame->local_map);
          env->obj_stack->push (lambda_obj);
      }
      void
      field_jo_t::exe (env_t *env, local_map_t *local_map)
      {
          obj_t *obj = env->obj_stack->top ();
          env->obj_stack->pop ();
          data_obj_t *data_obj =
              static_cast<data_obj_t *> (obj);
          auto it = data_obj->field_map->find (this->name);
          if (it != data_obj->field_map->end ()) {
              it->second->apply (env);
              return;
          }
          cout << "fatal error ! unknown field : "
               << this->name
               << "\n";
      }
      string
      jo_t::repr (env_t *env)
      {
          return "(unknown)";
      }
      string
      call_jo_t::repr (env_t *env)
      {
          return "(call " + this->name + ")";
      }
      string
      let_jo_t::repr (env_t *env)
      {
          return "(let " + this->name + ")";
      }
      string
      lambda_jo_t::repr (env_t *env)
      {
          return "(lambda)";
      }
      string
      field_jo_t::repr (env_t *env)
      {
          return "(field " + this->name + ")";
      }
      call_jo_t::call_jo_t (name_t name)
      {
          this->name = name;
      }
      let_jo_t::let_jo_t (name_t name)
      {
          this->name = name;
      }
      lambda_jo_t::lambda_jo_t (jojo_t *jojo)
      {
          this->jojo = jojo;
      }

      lambda_jo_t::lambda_jo_t (jojo_t jojo)
      {
          this->jojo = &jojo;
      }
      field_jo_t::field_jo_t (name_t name)
      {
          this->name = name;
      }
      void
      p1 (env_t *env)
      {
           cout << "- p1\n";
      }
      void
      p2 (env_t *env)
      {
           cout << "- p2\n";
      }
    int
    main ()
    {
        env_t *env = new env_t;

        field_map_t field_map = {
            {"f1", new str_obj_t ("fs1")},
            {"f2", new str_obj_t ("fs2")},
        };

        name_map_t env_name_map = {
            {"k1", new str_obj_t ("s1")},
            {"k2", new str_obj_t ("s2")},
            {"p1", new primitive_obj_t (p1)},
            {"p2", new primitive_obj_t (p2)},
            {"d1", new data_obj_t ("d-t", &field_map)},
        };
        env->name_map = &env_name_map;

        jojo_t jojo = {
            new call_jo_t ("p1"),
            new call_jo_t ("p2"),

            new call_jo_t ("k1"),
            new call_jo_t ("k2"),
            new let_jo_t ("v"),
            new call_jo_t ("v"),
            new lambda_jo_t ({
                new call_jo_t ("k1"),
                new call_jo_t ("k2"),
            }),
            new call_jo_t ("v"),

            new call_jo_t ("d1"),
            new call_jo_t ("d1"),
            new field_jo_t ("f1"),
        };
        frame_t *frame = new frame_t (&jojo, new local_map_t);

        env->frame_stack->push (frame);

        env->report ();
        env->run ();
        env->report ();
    }
