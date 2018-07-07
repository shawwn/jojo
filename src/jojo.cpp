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
      struct lambda_o: obj_t
      {
          jojo_t *jojo;
          local_map_t *local_map;
          lambda_o (env_t *env, jojo_t* jojo, local_map_t *local_map);
          virtual ~lambda_o ();
          void apply (env_t *env);
          void mark (env_t *env);
      };
      lambda_o::
      lambda_o (env_t *env,
                jojo_t* jojo,
                local_map_t *local_map)
      {
          this->t = "lambda-t";
          this->jojo = jojo;
          this->local_map = local_map;
          gc_for (env, this);
      }
      lambda_o::~lambda_o ()
      {
          delete this->jojo;
          this->local_map->clear ();
          delete this->local_map;
      }
      void
      lambda_o::mark (env_t *env)
      {
          this->cell->state = CELL_STATE_USED;
          for (auto &kv: *(this->local_map)) {
              obj_t *obj = kv.second;
              obj->mark (env);
          }
      }
      void
      lambda_o::apply (env_t *env)
      {
          frame_t *frame = new frame_t (this->jojo, this->local_map);
          env->frame_stack->push (frame);
      }
      typedef void (*prim_fn) (env_t *);
      struct primitive_o: obj_t
      {
          prim_fn fn;
          primitive_o (env_t *env, prim_fn fn);
          void apply (env_t *env);
      };
      primitive_o::primitive_o (env_t *env, prim_fn fn)
      {
          this->t = "primitive-t";
          this->fn = fn;
          gc_for (env, this);
      }
      void
      primitive_o::apply (env_t *env)
      {
          this->fn (env);
      }
      struct int_o: obj_t
      {
          int i;
          int_o (env_t *env, int i);
      };
      int_o::int_o (env_t *env, int i)
      {
          this->t = "int-t";
          this->i = i;
          gc_for (env, this);
      }
      struct string_o: obj_t
      {
          string s;
          string_o (env_t *env, string s);
      };
      string_o::string_o (env_t *env, string s)
      {
          this->t = "string-t";
          this->s = s;
          gc_for (env, this);
      }
      struct bool_o: obj_t
      {
          bool b;
          bool_o (env_t *env, bool b);
      };
      bool_o::bool_o (env_t *env, bool b)
      {
          this->t = "bool-t";
          this->b = b;
          gc_for (env, this);
      }
      using map_t = map<string, obj_t *>;
      struct map_o: obj_t
      {
          map_t *map;
          map_o (env_t *env, map_t *map);
          virtual ~map_o ();
          void mark (env_t *env);
      };
      map_o::map_o (env_t *env, map_t *map)
      {
          this->t = "map-t";
          this->map = map;
          gc_for (env, this);
      }
      map_o::~map_o ()
      {
          this->map->clear ();
          delete this->map;
      }
      void
      map_o::mark (env_t *env)
      {
          this->cell->state = CELL_STATE_USED;
          for (auto &kv: *(this->map)) {
              obj_t *obj = kv.second;
              obj->mark (env);
          }
      }
      using field_vector_t = vector<name_t>;
      struct type_o: obj_t
      {
          tag_t type_tag;
          field_vector_t *field_vector;
          type_o (env_t *env,
                  tag_t type_tag,
                  field_vector_t *field_vector);
          virtual ~type_o ();
      };
      type_o::
      type_o (env_t *env,
              tag_t type_tag,
              field_vector_t *field_vector)
      {
          this->t = "type-t";
          this->type_tag = type_tag;
          this->field_vector = field_vector;
      }
      type_o::~type_o ()
      {
          delete this->field_vector;
      }
      struct type_constructor_o: obj_t
      {

      };
      using field_map_t = map<name_t, obj_t *>;
      struct data_o: obj_t
      {
          field_map_t *field_map;
          data_o (env_t *env, tag_t t, field_map_t *field_map);
          virtual ~data_o ();
          void mark (env_t *env);
      };
      data_o::data_o (env_t *env, tag_t t, field_map_t *field_map)
      {
          this->t = t;
          this->field_map = field_map;
          gc_for (env, this);
      }
      data_o::~data_o ()
      {
          this->field_map->clear ();
          delete this->field_map;
      }
      void
      data_o::mark (env_t *env)
      {
          this->cell->state = CELL_STATE_USED;
          for (auto &kv: *(this->field_map)) {
              obj_t *obj = kv.second;
              obj->mark (env);
          }
      }
      struct data_constructor_o: obj_t
      {
          type_o *type;
          data_constructor_o (env_t *env, type_o *type);
          void apply (env_t *env);
      };
      data_constructor_o::
      data_constructor_o (env_t *env, type_o *type)
      {
          this->t = "data-constructor-t";
          this->type = type;
          gc_for (env, this);
      }
      void
      data_constructor_o::apply (env_t *env)
      {
          field_map_t *field_map = new field_map_t;
          field_vector_t *field_vector = this->type->field_vector;
          field_vector_t::reverse_iterator it;
          for (it = field_vector->rbegin();
               it != field_vector->rend();
               it++) {
              name_t name = *it;
              obj_t *obj = env->obj_stack->top ();
              env->obj_stack->pop ();
              field_map->insert (pair<name_t, obj_t *> (name, obj));
          }
          data_o* data =
              new data_o (env,
                          this->type->type_tag,
                          field_map);
          env->obj_stack->push (data);
      }
      struct data_creator_o: obj_t
      {
          type_o *type;
          data_creator_o (env_t *env, type_o *type);
          void apply (env_t *env);
      };
      data_creator_o::
      data_creator_o (env_t *env, type_o *type)
      {
          this->t = "data-creator-t";
          this->type = type;
          gc_for (env, this);
      }
      void
      data_creator_o::apply (env_t *env)
      {
          obj_t *obj = env->obj_stack->top ();
          env->obj_stack->pop ();
          map_o *map = static_cast<map_o *> (obj);
          data_o* data =
              new data_o (env,
                          this->type->type_tag,
                          map->map);
          env->obj_stack->push (data);
      }
      struct data_predicate_o: obj_t
      {
          type_o *type;
          data_predicate_o (env_t *env, type_o *type);
          void apply (env_t *env);
      };
      data_predicate_o::
      data_predicate_o (env_t *env, type_o *type)
      {
          this->t = "data-predicate-t";
          this->type = type;
          gc_for (env, this);
      }
      void
      data_predicate_o::apply (env_t *env)
      {
          tag_t tag = this->type->type_tag;
          obj_t *obj = env->obj_stack->top ();
          env->obj_stack->pop ();
          if (obj->t == tag)
              env->obj_stack->push (new bool_o (env, true));
          else
              env->obj_stack->push (new bool_o (env, false));
      }
      struct null_o: obj_t
      {
          null_o (env_t *env);
      };
      null_o::null_o (env_t *env)
      {
          gc_for (env, this);
      }
      struct cons_o: obj_t
      {
          obj_t *car;
          obj_t *cdr;
          cons_o (env_t *env, obj_t *car, obj_t *cdr);
          void mark (env_t *env);
      };
      cons_o::cons_o (env_t *env, obj_t *car, obj_t *cdr)
      {
          this->car = car;
          this->cdr = cdr;
          gc_for (env, this);
      }
      void
      cons_o::mark (env_t *env)
      {
          this->cell->state = CELL_STATE_USED;
          this->car->mark (env);
          this->cdr->mark (env);
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
      call_jo_t::call_jo_t (name_t name)
      {
          this->name = name;
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
          exit (1);
      }
      string
      call_jo_t::repr (env_t *env)
      {
          return "(call " + this->name + ")";
      }
      struct let_jo_t: jo_t
      {
          name_t name;
          let_jo_t (name_t name);
          void exe (env_t *env, local_map_t *local_map);
          string repr (env_t *env);
      };
      let_jo_t::let_jo_t (name_t name)
      {
          this->name = name;
      }
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
      struct lambda_jo_t: jo_t
      {
          jojo_t *jojo;
          lambda_jo_t (jojo_t *jojo);
          void exe (env_t *env, local_map_t *local_map);
          string repr (env_t *env);
      };
      lambda_jo_t::lambda_jo_t (jojo_t *jojo)
      {
          this->jojo = jojo;
      }
      void
      lambda_jo_t::exe (env_t *env, local_map_t *local_map)
      {
          // create lambda_o by closure
          // and push it to obj_stack
          frame_t *frame = env->frame_stack->top ();
          lambda_o *lambda =
              new lambda_o (env, this->jojo, frame->local_map);
          env->obj_stack->push (lambda);
      }
      string
      lambda_jo_t::repr (env_t *env)
      {
          return "(lambda)";
      }
      struct field_jo_t: jo_t
      {
          name_t name;
          field_jo_t (name_t name);
          void exe (env_t *env, local_map_t *local_map);
          string repr (env_t *env);
      };
      field_jo_t::field_jo_t (name_t name)
      {
          this->name = name;
      }
      void
      field_jo_t::exe (env_t *env, local_map_t *local_map)
      {
          obj_t *obj = env->obj_stack->top ();
          env->obj_stack->pop ();
          data_o *data = static_cast<data_o *> (obj);
          auto it = data->field_map->find (this->name);
          if (it != data->field_map->end ()) {
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
        field_map->insert (pair<name_t, obj_t *> ("f1", new string_o (env, "fs1")));
        field_map->insert (pair<name_t, obj_t *> ("f2", new string_o (env, "fs2")));

        name_map_t *name_map = new name_map_t;
        name_map->insert (pair<name_t, obj_t *> ("k1", new string_o (env, "s1")));
        name_map->insert (pair<name_t, obj_t *> ("k2", new string_o (env, "s2")));
        name_map->insert (pair<name_t, obj_t *> ("p1", new primitive_o (env, p1)));
        name_map->insert (pair<name_t, obj_t *> ("p2", new primitive_o (env, p2)));
        name_map->insert (pair<name_t, obj_t *> ("d1", new data_o (env, "d-t", field_map)));
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
            new string_o (env, "s");
            counter++;
        }

        counter = 0;
        while (counter < cell_area_size) {
            new string_o (env, "s");
            counter++;
        }

        counter = 0;
        while (counter < cell_area_size) {
            new string_o (env, "s");
            counter++;
        }

        env->report ();
    }
