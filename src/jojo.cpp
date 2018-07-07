    #include <iostream>
    #include <vector>
    #include <array>
    #include <map>
    #include <set>
    #include <stack>
    using namespace std;
    struct env_t;
    struct obj_t;
    struct cell_t;
    struct jo_t;
    using name_t = string;
    using local_map_t = map<name_t, obj_t *>;
    using jojo_t = vector<jo_t *>;
    struct jo_t
    {
        virtual void exe (env_t *env, local_map_t *local_map);
        virtual string repr (env_t *env);
    };
    using tag_t = string;
    struct obj_t
    {
        tag_t t;
        cell_t *cell;
        virtual ~obj_t ();
        virtual void apply (env_t *env);
        virtual void print (env_t *env);
        virtual void mark (env_t *env);
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
      enum cell_state_t
          { CELL_STATE_USED,
            CELL_STATE_FREE };
      struct cell_t
      {
          cell_state_t state;
          obj_t *obj;
      };
      const size_t cell_area_size = 1024;
      using cell_area_t = array<cell_t, cell_area_size>;
      struct gc_t
      {
          size_t index;
          size_t mark_counter;
          cell_area_t *cell_area;
          gc_t ();
      };
    struct env_t
    {
        name_map_t *name_map;
        obj_stack_t *obj_stack;
        frame_stack_t *frame_stack;
        gc_t *gc;
        env_t ();
        void step ();
        void run ();
        void report ();
    };
    void
    gc_for (env_t *env, obj_t *obj);
      obj_t::~obj_t ()
      {
      }
      void
      obj_t::mark (env_t *env)
      {
          this->cell->state = CELL_STATE_USED;
      }
      void
      obj_t::print (env_t *env)
      {
          cout << this->t;
      }
      void
      obj_t::apply (env_t *env)
      {
          env->obj_stack->push (this);
      }
      struct lambda_obj_t: obj_t
      {
          jojo_t *jojo;
          local_map_t *local_map;
          lambda_obj_t (env_t *env, jojo_t* jojo, local_map_t *local_map);
          virtual ~lambda_obj_t ();
          void apply (env_t *env);
          void mark (env_t *env);
      };
      lambda_obj_t::lambda_obj_t (env_t *env,
                                  jojo_t* jojo,
                                  local_map_t *local_map)
      {
          this->t = "lambda-t";
          this->jojo = jojo;
          this->local_map = local_map;
          gc_for (env, this);
      }
      lambda_obj_t::~lambda_obj_t ()
      {
          delete this->jojo;
          this->local_map->clear ();
          delete this->local_map;
      }
      void
      lambda_obj_t::mark (env_t *env)
      {
          this->cell->state = CELL_STATE_USED;
          for (auto &kv: *(this->local_map)) {
              obj_t *obj = kv.second;
              obj->mark (env);
          }
      }
      void
      lambda_obj_t::apply (env_t *env)
      {
          frame_t *frame = new frame_t (this->jojo, this->local_map);
          env->frame_stack->push (frame);
      }
      typedef void (*prim_fn) (env_t *);
      struct primitive_obj_t: obj_t
      {
          prim_fn fn;
          primitive_obj_t (env_t *env, prim_fn fn);
          void apply (env_t *env);
      };
      primitive_obj_t::primitive_obj_t (env_t *env, prim_fn fn)
      {
          this->t = "primitive-t";
          this->fn = fn;
          gc_for (env, this);
      }
      void
      primitive_obj_t::apply (env_t *env)
      {
          this->fn (env);
      }
      struct int_obj_t: obj_t
      {
          int i;
          int_obj_t (env_t *env, int i);
      };
      int_obj_t::int_obj_t (env_t *env, int i)
      {
          this->t = "int-t";
          this->i = i;
          gc_for (env, this);
      }
      struct string_obj_t: obj_t
      {
          string s;
          string_obj_t (env_t *env, string s);
      };
      string_obj_t::string_obj_t (env_t *env, string s)
      {
          this->t = "string-t";
          this->s = s;
          gc_for (env, this);
      }
      using map_t = map<string, obj_t *>;
      struct map_obj_t: obj_t
      {
          map_t *map;
          map_obj_t (env_t *env, map_t *map);
          virtual ~map_obj_t ();
          void mark (env_t *env);
      };
      map_obj_t::map_obj_t (env_t *env, map_t *map)
      {
          this->t = "map-t";
          this->map = map;
          gc_for (env, this);
      }
      map_obj_t::~map_obj_t ()
      {
          this->map->clear ();
          delete this->map;
      }
      void
      map_obj_t::mark (env_t *env)
      {
          this->cell->state = CELL_STATE_USED;
          for (auto &kv: *(this->map)) {
              obj_t *obj = kv.second;
              obj->mark (env);
          }
      }
      using field_map_t = map<name_t, obj_t *>;
      struct data_obj_t: obj_t
      {
          field_map_t *field_map;
          data_obj_t (env_t *env, tag_t t, field_map_t *field_map);
          virtual ~data_obj_t ();
          void mark (env_t *env);
      };
      data_obj_t::data_obj_t (env_t *env, tag_t t, field_map_t *field_map)
      {
          this->t = t;
          this->field_map = field_map;
          gc_for (env, this);
      }
      data_obj_t::~data_obj_t ()
      {
          this->field_map->clear ();
          delete this->field_map;
      }
      void
      data_obj_t::mark (env_t *env)
      {
          this->cell->state = CELL_STATE_USED;
          for (auto &kv: *(this->field_map)) {
              obj_t *obj = kv.second;
              obj->mark (env);
          }
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
              obj_t *obj = kv.second;
              obj->print (env);
              cout << "\n";
          }
      }
      void
      name_map_report (env_t *env)
      {
          cout << "- name_map # " << env->name_map->size () << "\n";
          for (auto &kv: *(env->name_map)) {
              cout << "  " << kv.first << " : ";
              obj_t *obj = kv.second;
              obj->print (env);
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
      gc_t::gc_t ()
      {
          this->index = 0;
          this->cell_area = new cell_area_t;
          for (auto &it: *(this->cell_area))
              it.state = CELL_STATE_FREE;
      }
      void
      gc_prepare (env_t *env)
      {
          gc_t *gc = env->gc;
          gc->index = 0;
          gc->mark_counter = 0;
          for (auto &it: *(gc->cell_area))
              it.state = CELL_STATE_FREE;
      }
      void
      gc_mark_one (env_t *env, obj_t *obj)
      {
          if (obj->cell->state == CELL_STATE_FREE) {
              env->gc->mark_counter++;
              obj->mark (env);
          }
      }
      void
      gc_mark (env_t *env)
      {
          for (auto &kv: *(env->name_map)) {
              obj_t *obj = kv.second;
              gc_mark_one (env, obj);
          }
          obj_stack_t obj_stack = *(env->obj_stack);
          while (!obj_stack.empty ()) {
              obj_t *obj = obj_stack.top ();
              gc_mark_one (env, obj);
              obj_stack.pop ();
          }
      }
      void
      gc_sweep (env_t *env)
      {
          for (auto &cell: *(env->gc->cell_area))
              if (cell.state == CELL_STATE_FREE)
                  delete cell.obj;
      }
      void
      gc_run (env_t *env)
      {
          gc_prepare (env);
          gc_mark (env);
          gc_sweep (env);
      }
      cell_t *
      gc_next_free_cell (env_t *env)
      {
           size_t index = env->gc->index;
           if (index >= cell_area_size) {
               gc_run (env);
               if (env->gc->mark_counter == cell_area_size) {
                   cout << "fatal error : cell_area fulled after gc" << "\n";
                   exit (1);
               }
               else {
                   return gc_next_free_cell (env);
               }
           }

           cell_t &cell = (*(env->gc->cell_area)) [index];
           if (cell.state == CELL_STATE_FREE) {
               env->gc->index++;
               return &cell;
           }
           else {
               env->gc->index++;
               return gc_next_free_cell (env);
           }
      }
      void
      gc_for (env_t *env, obj_t *obj)
      {
          cell_t *cell = gc_next_free_cell (env);
          cell->obj = obj;
          obj->cell = cell;
      }
    env_t::env_t ()
    {
        this->name_map = new name_map_t;
        this->obj_stack = new obj_stack_t;
        this->frame_stack = new frame_stack_t;
        this->gc = new gc_t;
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
          exit (1);
      }
      string
      jo_t::repr (env_t *env)
      {
          return "(unknown)";
      }
      struct call_jo_t: jo_t
      {
          name_t name;

          call_jo_t (name_t name);

          void exe (env_t *env, local_map_t *local_map);
          string repr (env_t *env);
      };
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
          exit (1);
      }
      string
      call_jo_t::repr (env_t *env)
      {
          return "(call " + this->name + ")";
      }
      call_jo_t::call_jo_t (name_t name)
      {
          this->name = name;
      }
      struct let_jo_t: jo_t
      {
          name_t name;
          let_jo_t (name_t name);
          void exe (env_t *env, local_map_t *local_map);
          string repr (env_t *env);
      };
      void
      let_jo_t::exe (env_t *env, local_map_t *local_map)
      {
           obj_t *obj = env->obj_stack->top ();
           env->obj_stack->pop ();
           local_map->insert (pair<name_t, obj_t *> (this->name, obj));
      }
      string
      let_jo_t::repr (env_t *env)
      {
          return "(let " + this->name + ")";
      }
      let_jo_t::let_jo_t (name_t name)
      {
          this->name = name;
      }
      struct lambda_jo_t: jo_t
      {
          jojo_t *jojo;
          lambda_jo_t (jojo_t *jojo);
          void exe (env_t *env, local_map_t *local_map);
          string repr (env_t *env);
      };
      void
      lambda_jo_t::exe (env_t *env, local_map_t *local_map)
      {
          // create lambda_obj_t by closure
          // and push it to obj_stack
          frame_t *frame = env->frame_stack->top ();
          lambda_obj_t *lambda_obj =
              new lambda_obj_t (env, this->jojo, frame->local_map);
          env->obj_stack->push (lambda_obj);
      }
      string
      lambda_jo_t::repr (env_t *env)
      {
          return "(lambda)";
      }
      lambda_jo_t::lambda_jo_t (jojo_t *jojo)
      {
          this->jojo = jojo;
      }
      struct field_jo_t: jo_t
      {
          name_t name;
          field_jo_t (name_t name);
          void exe (env_t *env, local_map_t *local_map);
          string repr (env_t *env);
      };
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
          exit (1);
      }
      string
      field_jo_t::repr (env_t *env)
      {
          return "(field " + this->name + ")";
      }
      field_jo_t::field_jo_t (name_t name)
      {
          this->name = name;
      }
      struct apply_jo_t: jo_t
      {
          void exe (env_t *env, local_map_t *local_map);
          string repr (env_t *env);
      };
      void
      apply_jo_t::exe (env_t *env, local_map_t *local_map)
      {
          obj_t *obj = env->obj_stack->top ();
          env->obj_stack->pop ();
          obj->apply (env);
      }
      string
      apply_jo_t::repr (env_t *env)
      {
          return "(apply)";
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
        field_map_t *field_map = new field_map_t;
        field_map->insert
            (pair<name_t, obj_t *> ("f1", new string_obj_t (env, "fs1")));
        field_map->insert
            (pair<name_t, obj_t *> ("f2", new string_obj_t (env, "fs2")));

        name_map_t *name_map = new name_map_t;
        name_map->insert (pair<name_t, obj_t *> ("k1", new string_obj_t (env, "s1")));
        name_map->insert (pair<name_t, obj_t *> ("k2", new string_obj_t (env, "s2")));
        name_map->insert (pair<name_t, obj_t *> ("p1", new primitive_obj_t (env, p1)));
        name_map->insert (pair<name_t, obj_t *> ("p2", new primitive_obj_t (env, p2)));
        name_map->insert (pair<name_t, obj_t *> ("d1", new data_obj_t (env, "d-t", field_map)));
        env->name_map = name_map;

        jojo_t *lambda_jojo = new jojo_t;
        lambda_jojo->push_back (new call_jo_t ("k1"));
        lambda_jojo->push_back (new call_jo_t ("k2"));
        lambda_jojo->push_back (new call_jo_t ("v"));
        jojo_t *jojo = new jojo_t;
        jojo->push_back (new call_jo_t ("p1"));
        jojo->push_back (new call_jo_t ("p2"));
        jojo->push_back (new call_jo_t ("k1"));
        jojo->push_back (new call_jo_t ("k2"));
        jojo->push_back (new let_jo_t ("v"));
        jojo->push_back (new call_jo_t ("v"));
        jojo->push_back (new lambda_jo_t (lambda_jojo));
        jojo->push_back (new apply_jo_t ());
        jojo->push_back (new call_jo_t ("v"));
        jojo->push_back (new call_jo_t ("d1"));
        jojo->push_back (new call_jo_t ("d1"));
        jojo->push_back (new field_jo_t ("f1"));

        frame_t *frame = new frame_t (jojo, new local_map_t);
        env->frame_stack->push (frame);
        env->report ();
        env->run ();

        size_t counter;

        counter = 0;
        while (counter < cell_area_size) {
            new string_obj_t (env, "s");
            counter++;
        }

        counter = 0;
        while (counter < cell_area_size) {
            new string_obj_t (env, "s");
            counter++;
        }

        counter = 0;
        while (counter < cell_area_size) {
            new string_obj_t (env, "s");
            counter++;
        }

        env->report ();
    }
