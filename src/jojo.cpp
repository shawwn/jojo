    #include <iostream>
    #include <vector>
    #include <map>
    #include <set>
    #include <stack>
    using namespace std;
    using name_t = string;
      struct env_t;
      struct obj_t;
      struct ins_t
      {
          virtual
          void exe (env_t &env, map<name_t, obj_t *> &local_map);
          virtual
          string repr (env_t &env);
      };
      struct call_ins_t: ins_t
      {
          name_t name;
          void exe (env_t &env, map<name_t, obj_t *> &local_map);
          string repr (env_t &env);
      };
      struct let_ins_t: ins_t
      {
          name_t name;
          void exe (env_t &env, map<name_t, obj_t *> &local_map);
          string repr (env_t &env);
      };
      struct lambda_ins_t: ins_t
      {
          vector<ins_t *> body;
          void exe (env_t &env, map<name_t, obj_t *> &local_map);
          string repr (env_t &env);
      };
      using tag_t = string;
      struct obj_t
      {
          tag_t t;
      };
      struct lambda_obj_t: obj_t
      {
          map<name_t, obj_t *> local_map;
          vector<ins_t *> body;
      };
      struct int_obj_t: obj_t
      {
          int i;
      };
      struct str_obj_t: obj_t
      {
          string s;
      };
    struct frame_t
    {
       size_t index;
       vector<ins_t *> body;
       map<name_t, obj_t *> local_map;
    };
    struct env_t
    {
        map<name_t, obj_t *> name_map;
        stack<obj_t *> obj_stack;
        stack<frame_t *> frame_stack;

        void step ();
        void run ();

        void report_name_map ();
        void report_frame_stack ();
        void report_obj_stack ();
        void report ();
    };
    void env_t::step ()
    {
        frame_t *frame = this->frame_stack.top ();
        size_t size = frame->body.size ();
        size_t index = frame->index;

        // handle empty function body
        if (index >= size) {
            this->frame_stack.pop ();
            return;
        }

        // get ins only for non empty function body
        ins_t *ins = frame->body[index];

        frame->index++;

        // handle proper tail call
        if (index+1 == size)
            frame_stack.pop ();

        // since the last frame might be drop,
        //   we pass local_map the last frame
        //   as an extra argument.
        ins->exe (*this, frame->local_map);
    }
    void env_t::run ()
    {
        while (!this->frame_stack.empty ())
            this->step ();
    }
      void obj_print (env_t &env, obj_t &obj)
      {
          if (obj.t == "lambda-t") {
              cout << "lambda-t";
          }
          else if (obj.t == "int-t") {
              cout << "int-t";
          }
          else if (obj.t == "string-t") {
              cout << "string-t";
          }
          else {
              cout << "<unknown-t>";
          }
      }
      void env_t::report_name_map ()
      {
          cout << "- name_map # " << this->name_map.size () << "\n";
          for (auto &kv: this->name_map) {
              cout << "  " << kv.first << " : ";
              obj_print (*this, *(kv.second));
              cout << "\n";
          }
      }
      void body_print (env_t &env, vector<ins_t *> &body)
      {
          for (auto &ins: body)
              cout << ins->repr (env) << " ";
      }
      void body_print_with_index (env_t &env, vector<ins_t *> &body,
                                  size_t index)
      {
          vector<ins_t *>::iterator it;
          for (it = body.begin ();
               it != body.end ();
               it++) {
              size_t it_index = it - body.begin();
              ins_t *ins = *it;
              if (index == it_index) {
                  cout << "->> " << ins->repr (env) << " ";
              }
              else {
                  cout << ins->repr (env) << " ";
              }
          }
      }
      void frame_report (env_t &env, frame_t &frame)
      {
          cout << "  - ["
               << frame.index+1
               << "/"
               << frame.body.size()
               << "] ";
          body_print_with_index (env, frame.body, frame.index);
          cout << "\n";

          cout << "  - local_map # " << frame.local_map.size () << "\n";
          for (auto &kv: frame.local_map) {
              cout << "    " << kv.first << " : ";
              obj_print (env, *(kv.second));
              cout << "\n";
          }
      }
      void env_t::report_frame_stack ()
      {
          cout << "- frame_stack # " << this->frame_stack.size () << "\n";
          stack<frame_t *> frame_stack = this->frame_stack;
          while (!frame_stack.empty ()) {
             frame_t *frame = frame_stack.top ();
             frame_report (*this, *frame);
             frame_stack.pop ();
          }
      }
      void env_t::report_obj_stack ()
      {
          cout << "- obj_stack # " << this->obj_stack.size () << "\n";
          cout << "  ";
          stack<obj_t *> obj_stack = this->obj_stack;
          while (!obj_stack.empty ()) {
              obj_t *obj = obj_stack.top ();
              obj_print (*this, *obj);
              cout << " ";
              obj_stack.pop ();
          }
          cout << "\n";
      }
      void env_t::report ()
      {
          this->report_name_map ();
          this->report_frame_stack ();
          this->report_obj_stack ();
          cout << "\n";
      }
      void ins_t::exe (env_t &env, map<name_t, obj_t *> &local_map)
      {
          cout << "fatal error : unknown ins" << "\n";
      }
      void obj_apply (env_t &env, obj_t *obj)
      {
          if (obj->t == "lambda-t") {
              // apply lambda by push new frame to frame_stack
              lambda_obj_t *obj = obj;
              frame_t *frame = new frame_t;
              frame->index = 0;
              frame->body = obj->body;
              frame->local_map = obj->local_map;
              env.frame_stack.push (frame);
          } else {
              // push non lambda into obj_stack
              env.obj_stack.push (obj);
          }
      }
      void call_ins_t::exe (env_t &env, map<name_t, obj_t *> &local_map)
      {
          // local_map first
          auto it = local_map.find (this->name);
          if (it != local_map.end ()) {
              obj_apply (env, it->second);
              return;
          }
          // name_map second
          it = env.name_map.find (this->name);
          if (it != env.name_map.end ()) {
              obj_apply (env, it->second);
              return;
          }
          cout << "fatal error ! unknown name : "
               << this->name
               << "\n";
      }
      void let_ins_t::exe (env_t &env, map<name_t, obj_t *> &local_map)
      {
           obj_t *obj = env.obj_stack.top ();
           env.obj_stack.pop ();
           local_map.insert (pair<name_t, obj_t *> (this->name, obj));
      }
      void lambda_ins_t::exe (env_t &env, map<name_t, obj_t *> &local_map)
      {
          // create lambda_obj_t by closure
          // and push it to obj_stack
          lambda_obj_t *lambda_obj = new lambda_obj_t;
          lambda_obj->t = "lambda-t";
          lambda_obj->body = this->body;
          frame_t *frame = env.frame_stack.top ();
          lambda_obj->local_map = frame->local_map;
          env.obj_stack.push (lambda_obj);
      }
      string ins_t::repr (env_t &env)
      {
          return "(unknown)";
      }
      string call_ins_t::repr (env_t &env)
      {
          return "(call " + this->name + ")";
      }
      string let_ins_t::repr (env_t &env)
      {
          return "(let " + this->name + ")";
      }
      string lambda_ins_t::repr (env_t &env)
      {
          return "(lambda)";
      }
    int main ()
    {
        env_t env;

        str_obj_t s1;
        s1.t = "string-t";
        s1.s = "s1";

        str_obj_t s2;
        s2.t = "string-t";
        s2.s = "s2";

        env.name_map = {
            {"k1", &s1},
            {"k2", &s2},
        };

        frame_t frame;
        frame.index = 0;

        call_ins_t call_k1;
        call_k1.name = "k1";

        call_ins_t call_k2;
        call_k2.name = "k2";

        let_ins_t let_v;
        let_v.name = "v";

        call_ins_t call_v;
        call_v.name = "v";

        lambda_ins_t lambda_f;
        lambda_f.body = {
            &call_k1,
            &call_k2,
        };

        frame.body = {
            &call_k1,
            &call_k2,
            &let_v,
            &call_v,
            &lambda_f,
            &call_v,
        };

        env.frame_stack.push (&frame);

        env.report ();
        env.run ();
        env.report ();
    }
