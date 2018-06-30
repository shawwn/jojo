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
      };
      struct call_ins_t: public ins_t
      {
          name_t name;
          void exe (env_t &env, map<name_t, obj_t *> &local_map);
      };
      struct let_ins_t: public ins_t
      {
          name_t name;
          void exe (env_t &env, map<name_t, obj_t *> &local_map);
      };
      struct lambda_ins_t: public ins_t
      {
          vector<ins_t *> body;
          void exe (env_t &env, map<name_t, obj_t *> &local_map);
      };
      struct obj_t
      {
          string tagstr;
      };
      struct lambda_obj_t: public obj_t
      {
          map<name_t, obj_t *> local_map;
          vector<ins_t *> body;
      };
      struct int_obj_t: public obj_t
      {
          int i;
      };
      struct str_obj_t: public obj_t
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
        void report ();
        void eval ();
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
    void env_t::report ()
    {
        cout << "- name_map : "
             << this->name_map.size () << "\n";
        cout << "- obj_stack : "
             << this->obj_stack.size () << "\n";
        cout << "- frame_stack : "
             << this->frame_stack.size () << "\n";
        cout << "\n";
    }
    void env_t::eval ()
    {

    }
    void ins_t::exe (env_t &env, map<name_t, obj_t *> &local_map)
    {
        cout << "fatal error : unknown ins" << "\n";
    }
    void obj_apply (env_t &env, obj_t &obj)
    {
        // apply lambda
        // ><><><

        // push non lambda into obj_stack
        env.obj_stack.push (&obj);
    }
    void call_ins_t::exe (env_t &env, map<name_t, obj_t *> &local_map)
    {
        // local_map first
        auto it = local_map.find (this->name);
        if (it != local_map.end ()) {
            obj_apply (env, *(it->second));
            return;
        }
        // name_map second
        it = env.name_map.find (this->name);
        if (it != env.name_map.end ()) {
            obj_apply (env, *(it->second));
            return;
        }
        cout << "fatal error ! unknown name : "
             << this->name
             << "\n";
    }
    void let_ins_t::exe (env_t &env, map<name_t, obj_t *> &local_map)
    {

    }
    void lambda_ins_t::exe (env_t &env, map<name_t, obj_t *> &local_map)
    {

    }
    int main ()
    {
        env_t env;

        str_obj_t s1;
        s1.tagstr = "string-t";
        s1.s = "s1";

        str_obj_t s2;
        s2.tagstr = "string-t";
        s2.s = "s2";

        env.name_map.insert (pair<name_t, obj_t *> ("k1", &s1));
        env.name_map.insert (pair<name_t, obj_t *> ("k2", &s2));

        frame_t frame;
        frame.index = 0;

        call_ins_t ins1;
        ins1.name = "k1";

        call_ins_t ins2;
        ins2.name = "k2";

        frame.body.push_back (&ins1);
        frame.body.push_back (&ins2);

        env.frame_stack.push (&frame);

        env.report ();
        env.step ();
        env.report ();
        env.step ();
        env.report ();
    }
