  // use std::rc::Rc;
  use std::sync::Arc;
  use std::collections::HashMap;

  use dic::Dic;
  use token;
  // pub type Ptr <T> = Rc <T>;
  pub type Ptr <T> = Arc <T>;

  pub type Name = String;

  pub type Id = usize; // index in to ObjDic
  pub type ObjDic = Dic <Ptr <Obj>>;

  pub type Tag = usize; // index in to TypeDic
  pub type TypeDic = Dic <Ptr <Type>>;

  pub type ObjStack = Vec <Ptr <Obj>>;
  pub type FrameStack = Vec <Box <Frame>>;

  pub type Scope = Vec <ObjDic>; // index from end

  pub type TagVec = Vec <Tag>;
  pub type ObjVec = Vec <Ptr <Obj>>;
  pub type JoVec = Vec <Ptr <Jo>>;
      fn vec_peek <T> (vec: &Vec <T>, index: usize) -> &T {
            let back_index = vec.len () - index - 1;
            &vec [back_index]
      }
      pub fn obj_stack_pop_to_vec (
          env: &mut Env,
          len: usize,
      ) -> ObjVec {
          let mut obj_vec = ObjVec::new ();
          (0..len)
              .into_iter ()
              .for_each (|_| obj_vec.push (
                  env.obj_stack.pop () .unwrap ()));
          obj_vec
      }
      pub fn obj_dic_eat_obj_vec (
          obj_dic: &ObjDic,
          obj_vec: ObjVec,
      ) -> ObjDic {
          let mut obj_dic = obj_dic.clone ();
          obj_vec
              .into_iter ()
              .rev ()
              .for_each (|obj| obj_dic.eat (obj));
          obj_dic
      }
      pub fn obj_dic_pick_up (
          env: &mut Env,
          obj_dic: &ObjDic,
          arity: usize,
      ) -> ObjDic {
          obj_dic_eat_obj_vec (
              obj_dic,
              obj_stack_pop_to_vec (env, arity))
      }
      fn obj_dic_eq (
          lhs: &ObjDic,
          rhs: &ObjDic,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter () .zip (rhs.iter ())
           .all (|p| ((p.0).0 == (p.1).0 &&
                      obj_eq ((p.0).1.dup (),
                              (p.1).1.dup ()))))
      }
      pub fn obj_vec_eq (
          lhs: &ObjVec,
          rhs: &ObjVec,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter () .zip (rhs.iter ())
           .all (|p| obj_eq (p.0.dup (),
                             p.1.dup ())))
      }
      pub fn scope_extend (
          scope: &Scope,
          obj_dic: ObjDic,
      ) -> Ptr <Scope> {
          let mut obj_dic_vec = scope.clone ();
          obj_dic_vec.push (obj_dic);
          Ptr::new (obj_dic_vec)
      }
      pub fn scope_eq (
          lhs: &Scope,
          rhs: &Scope,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter () .zip (rhs.iter ())
           .all (|p| obj_dic_eq (p.0, p.1)))
      }
      fn new_jojo () -> Ptr <JoVec> {
          let jo_vec = JoVec::new ();
          Ptr::new (jo_vec)
      }
      fn jojo_append (
          ante: &JoVec,
          succ: &JoVec,
      ) -> Ptr <JoVec> {
          let mut jo_vec = ante.clone ();
          jo_vec.append (&mut succ.clone ());
          Ptr::new (jo_vec)
      }
      pub fn jojo_eq (
          lhs: &JoVec,
          rhs: &JoVec,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter () .zip (rhs.iter ())
           .all (|p| jo_eq (p.0.dup (),
                            p.1.dup ())))
      }
      macro_rules! jojo {
          ( $( $x:expr ),* ) => {{
              let jo_vec: JoVec = vec! [
                  $( Ptr::new ($x) ),*
              ];
              Ptr::new (jo_vec)
          }};
      }
      macro_rules! frame {
          ( $( $x:expr ),* ) => {{
              let jo_vec: JoVec = vec! [
                  $( Ptr::new ($x) ),*
              ];
              Frame::make (jo_vec)
          }};
      }
      pub fn name_of_tag (
          env: &Env,
          tag: Tag,
      ) -> Name {
          if tag >= env.type_dic.len () {
              format! ("#<unknown-tag:{}>", tag.to_string ())
          } else {
              let entry = env.type_dic.idx (tag);
              entry.name.clone ()
          }
      }
      fn preserve_tag (
          env: &mut Env,
          tag: Tag,
          name: &str,
      ) {
          let index = env.type_dic.ins (name, Some (Type::obj (tag)));
          assert_eq! (tag, index);
      }
      pub const CLOSURE_T      : Tag = 0;
      pub const TYPE_T         : Tag = 1;
      pub const TRUE_T         : Tag = 2;
      pub const FALSE_T        : Tag = 3;
      pub const DATA_CONS_T    : Tag = 4;
      pub const PRIM_T         : Tag = 5;
      pub const NUM_T          : Tag = 6;
      pub const STR_T          : Tag = 7;
      pub const SYM_T          : Tag = 8;
      pub const NULL_T         : Tag = 9;
      pub const CONS_T         : Tag = 10;
      pub const VECT_T         : Tag = 11;
      pub const DICT_T         : Tag = 12;
      pub const MODULE_T       : Tag = 13;
      pub const KEYWORD_T      : Tag = 14;
      pub const MACRO_T        : Tag = 15;
      pub const TOP_KEYWORD_T  : Tag = 16;
      pub const NOTHING_T      : Tag = 17;
      pub const JUST_T         : Tag = 18;
      fn init_type_dic (env: &mut Env) {
          preserve_tag (env, CLOSURE_T      , "closure-t");
          preserve_tag (env, TYPE_T         , "type-t");
          preserve_tag (env, TRUE_T         , "true-t");
          preserve_tag (env, FALSE_T        , "false-t");
          preserve_tag (env, DATA_CONS_T    , "data-cons-t");
          preserve_tag (env, PRIM_T         , "prim-t");
          preserve_tag (env, NUM_T          , "num-t");
          preserve_tag (env, STR_T          , "str-t");
          preserve_tag (env, SYM_T          , "sym-t");
          preserve_tag (env, NULL_T         , "null-t");
          preserve_tag (env, CONS_T         , "cons-t");
          preserve_tag (env, VECT_T         , "vect-t");
          preserve_tag (env, DICT_T         , "dict-t");
          preserve_tag (env, MODULE_T       , "module-t");
          preserve_tag (env, KEYWORD_T      , "keyword-t");
          preserve_tag (env, MACRO_T        , "macro-t");
          preserve_tag (env, TOP_KEYWORD_T  , "top-keyword-t");
          preserve_tag (env, NOTHING_T      , "nothing-t");
          preserve_tag (env, JUST_T         , "just-t");
      }
      pub trait Dup {
         fn dup (&self) -> Self;
      }
      impl Dup for Ptr <Obj> {
          fn dup (&self) -> Self {
              Ptr::clone (self)
          }
      }
      impl Dup for Ptr <Jo> {
          fn dup (&self) -> Self {
              Ptr::clone (self)
          }
      }
      impl Dup for Ptr <Scope> {
          fn dup (&self) -> Self {
              Ptr::clone (self)
          }
      }
      impl Dup for Ptr <JoVec> {
          fn dup (&self) -> Self {
              Ptr::clone (self)
          }
      }
    pub trait ObjFrom <T> {
        fn obj (T) -> Ptr <Self>;
    }
    pub trait Obj {
        fn tag (&self) -> Tag;
        fn obj_dic (&self) -> Option <&ObjDic> { None }

        fn eq (&self, other: Ptr <Obj>) -> bool;

        fn get (
            &self,
            name: &str,
        ) -> Option <Ptr <Obj>> {
            if let Some (obj_dic) = self.obj_dic () {
                if let Some (obj) = obj_dic.get (name) {
                    Some (obj.dup ())
                } else {
                    None
                }
            } else {
                None
            }
        }

        fn get_method (
            &self,
            env: &Env,
            name: &str,
        ) -> Option <Ptr <Obj>> {
            let tag = self.tag ();
            let entry = env.type_dic.idx (tag);
            if let Some (typ) = &entry.value {
                typ.get (name)
            } else {
                None
            }
        }

        fn dot (
            &self,
            env: &Env,
            name: &str,
        ) -> Option <Ptr <Obj>> {
            if let Some (obj) = self.get (name) {
                Some (obj)
            } else {
                self.get_method (env, name)
            }
        }

        fn repr (&self, env: &Env) -> String {
            format! ("#<{}>", name_of_tag (&env, self.tag ()))
        }

        fn print (&self, env: &Env) {
            println! ("{}", self.repr (&env));
        }

        fn apply (&self, env: &mut Env, arity: usize) {
            eprintln! ("- Obj::apply");
            eprintln! ("  applying non applicable object");
            eprintln! ("  tag : {}", name_of_tag (&env, self.tag ()));
            eprintln! ("  obj : {}", self.repr (env));
            eprintln! ("  arity : {}", arity);
            panic! ("jojo fatal error!");
        }

        // fn apply_to_arg_dict (&self, env: &mut Env) {
        //     eprintln! ("- Obj::apply_to_arg_dict");
        //     eprintln! ("  applying non applicable object");
        //     eprintln! ("  tag : {}", name_of_tag (&env, self.tag ()));
        //     eprintln! ("  obj : {}", self.repr (&env));
        //     panic! ("jojo fatal error!");
        // }
    }
    pub fn obj_to <T: Obj> (obj: Ptr <Obj>) -> Ptr <T> {
        let obj_ptr = Ptr::into_raw (obj);
        unsafe {
            let obj_ptr = obj_ptr as *const Obj as *const T;
            Ptr::from_raw (obj_ptr)
        }
    }
    pub fn obj_eq (
        lhs: Ptr <Obj>,
        rhs: Ptr <Obj>,
    ) -> bool {
        lhs.eq (rhs)
    }
    pub trait Jo {
        fn exe (&self, env: &mut Env, scope: Ptr <Scope>);

        fn repr (&self, _env: &Env) -> String {
            "#<unknown-jo>".to_string ()
        }
    }
    pub fn jo_eq (
        lhs: Ptr <Jo>,
        rhs: Ptr <Jo>,
    ) -> bool {
        let lhs_ptr = Ptr::into_raw (lhs);
        let rhs_ptr = Ptr::into_raw (rhs);
        lhs_ptr == rhs_ptr
    }
    pub struct RefJo {
        id: Id,
    }

    impl Jo for RefJo {
        fn exe (&self, env: &mut Env, _: Ptr <Scope>) {
            let entry = env.obj_dic.idx (self.id);
            if let Some (obj) = &entry.value {
                env.obj_stack.push (obj.dup ());
            } else {
                eprintln! ("- RefJo::exe");
                eprintln! ("  undefined name : {}", entry.name);
                eprintln! ("  id : {}", self.id);
                panic! ("jojo fatal error!");
            }
        }
    }

    pub struct LocalRefJo {
        level: usize,
        index: usize,
    }

    impl Jo for LocalRefJo {
        fn exe (&self, env: &mut Env, scope: Ptr <Scope>) {
            let obj_dic = vec_peek (&scope, self.level);
            let entry = obj_dic.idx (self.index);
            if let Some (obj) = &entry.value {
                env.obj_stack.push (obj.dup ());
            } else {
                eprintln! ("- LocalRefJo::exe");
                eprintln! ("  undefined name : {}", entry.name);
                eprintln! ("  level : {}", self.level);
                eprintln! ("  index : {}", self.index);
                panic! ("jojo fatal error!");
            }
        }
    }
    pub struct ApplyJo {
        arity: usize,
    }

    impl Jo for ApplyJo {
        fn exe (&self, env: &mut Env, _: Ptr <Scope>) {
            let obj = env.obj_stack.pop () .unwrap ();
            obj.apply (env, self.arity);
        }
    }
    pub struct DotJo {
        name: String,
    }

    impl Jo for DotJo {
        fn exe (&self, env: &mut Env, _: Ptr <Scope>) {
            let obj = env.obj_stack.pop () .unwrap ();
            let dot = obj.dot (env, &self.name) .unwrap ();
            env.obj_stack.push (dot);
        }
    }
    pub struct LambdaJo {
        arg_dic: ObjDic,
        jojo: Ptr <JoVec>,
    }

    impl Jo for LambdaJo {
        fn exe (&self, env: &mut Env, scope: Ptr <Scope>) {
            env.obj_stack.push (Ptr::new (Closure {
                arg_dic: self.arg_dic.clone (),
                jojo: self.jojo.dup (),
                scope: scope.dup (),
            }));
        }
    }
    pub struct Env {
        pub obj_dic: ObjDic,
        pub type_dic: TypeDic,
        pub obj_stack: ObjStack,
        pub frame_stack: FrameStack,
    }

    impl Env {
        pub fn new () -> Env {
            let mut env = Env {
                obj_dic: ObjDic::new (),
                type_dic: TypeDic::new (),
                obj_stack: ObjStack::new (),
                frame_stack: FrameStack::new (),
            };
            init_type_dic (&mut env);
            env
        }

        pub fn step (&mut self) {
            if let Some (mut frame) = self.frame_stack.pop () {
                let index = frame.index;
                let jo = frame.jojo [frame.index] .dup ();
                frame.index += 1;
                if index + 1 < frame.jojo.len () {
                    let scope = frame.scope.dup ();
                    self.frame_stack.push (frame);
                    jo.exe (self, scope);
                } else {
                    jo.exe (self, frame.scope);
                }
            }
        }

        pub fn run (&mut self) {
            while ! self.frame_stack.is_empty () {
                self.step ();
            }
        }

        pub fn run_with_base (&mut self, base: usize) {
            while self.frame_stack.len () > base {
                self.step ();
            }
        }

        pub fn define (
            &mut self,
            name: &str,
            obj: Ptr <Obj>,
        ) -> Id {
            self.obj_dic.ins (name, Some (obj))
        }

        pub fn define_type (
            &mut self,
            name: &str,
            typ: Ptr <Type>,
        ) -> Tag {
            self.type_dic.ins (name, Some (typ))
        }
    }
    pub struct Frame {
        pub index: usize,
        pub jojo: Ptr <JoVec>,
        pub scope: Ptr <Scope>,
    }
    impl Frame {
        pub fn make (jo_vec: JoVec) -> Box <Frame> {
            Box::new (Frame {
                index: 0,
                jojo: Ptr::new (jo_vec),
                scope: Ptr::new (Scope::new ()),
            })
        }
    }
    pub struct Type {
        method_dic: ObjDic,
        tag_of_type: Tag,
        super_tag_vec: TagVec,
    }
    impl ObjFrom <Tag> for Type {
        fn obj (tag: Tag) -> Ptr <Type> {
            Ptr::new (Type {
                method_dic: ObjDic::new (),
                tag_of_type: tag,
                super_tag_vec: TagVec::new (),
            })
        }
    }
    impl Obj for Type {
        fn tag (&self) -> Tag { TYPE_T }
        fn obj_dic (&self) -> Option <&ObjDic> { Some (&self.method_dic) }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Type> (other);
                (self.tag_of_type == other.tag_of_type &&
                 self.super_tag_vec == other.super_tag_vec)
            }
        }
    }
    pub struct Data {
        tag_of_type: Tag,
        field_dic: ObjDic,
    }
    impl Data {
        fn make (
            tag: Tag,
            vec: Vec <(&str, Ptr <Obj>)>,
        ) -> Ptr <Data> {
            Ptr::new (Data {
                tag_of_type: tag,
                field_dic: Dic::from (vec),
            })
        }
    }
    impl Data {
        fn unit (tag: Tag) -> Ptr <Data> {
            Ptr::new (Data {
                tag_of_type: tag,
                field_dic: ObjDic::new (),
            })
        }
    }
    impl Obj for Data {
        fn tag (&self) -> Tag { self.tag_of_type }
        fn obj_dic (&self) -> Option <&ObjDic> { Some (&self.field_dic) }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Data> (other);
                (self.tag_of_type == other.tag_of_type &&
                 obj_dic_eq (&self.field_dic, &other.field_dic))
            }
        }
    }
    pub struct DataCons {
        tag_of_type: Tag,
        field_dic: ObjDic,
    }
    impl DataCons {
        pub fn make (
            tag: Tag,
            vec: Vec <&str>,
        ) -> Ptr <DataCons> {
            Ptr::new (DataCons {
                tag_of_type: tag,
                field_dic: Dic::from (vec),
            })
        }
    }
    impl Obj for DataCons {
        fn tag (&self) -> Tag { DATA_CONS_T }
        fn obj_dic (&self) -> Option <&ObjDic> { Some (&self.field_dic) }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<DataCons> (other);
                (self.tag_of_type == other.tag_of_type &&
                 obj_dic_eq (&self.field_dic, &other.field_dic))
            }
        }

        fn apply (&self, env: &mut Env, arity: usize) {
            let lack = self.field_dic.lack ();
            if arity > lack {
                eprintln! ("- DataCons::apply");
                eprintln! ("  over-arity apply");
                eprintln! ("  arity > lack");
                eprintln! ("  arity : {}", arity);
                eprintln! ("  lack : {}", lack);
                panic! ("jojo fatal error!");
            }
            let tag_of_type = self.tag_of_type;
            let field_dic = obj_dic_pick_up (
                env, &self.field_dic, arity);
            if arity == lack {
                env.obj_stack.push (Ptr::new (Data {
                    tag_of_type,
                    field_dic,
                }));
            } else {
                env.obj_stack.push (Ptr::new (DataCons {
                    tag_of_type,
                    field_dic,
                }));
            }
        }
    }
    pub struct Closure {
        arg_dic: ObjDic,
        jojo: Ptr <JoVec>,
        scope: Ptr <Scope>,
    }
    impl Obj for Closure {
        fn tag (&self) -> Tag { CLOSURE_T }
        fn obj_dic (&self) -> Option <&ObjDic> { Some (&self.arg_dic) }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Closure> (other);
                (jojo_eq (&self.jojo, &other.jojo) &&
                 scope_eq (&self.scope, &other.scope) &&
                 obj_dic_eq (&self.arg_dic, &other.arg_dic))
            }
        }

        fn apply (&self, env: &mut Env, arity: usize) {
            let lack = self.arg_dic.lack ();
            if arity > lack {
                eprintln! ("- Closure::apply");
                eprintln! ("  over-arity apply");
                eprintln! ("  arity > lack");
                eprintln! ("  arity : {}", arity);
                eprintln! ("  lack : {}", lack);
                panic! ("jojo fatal error!");
            }
            let arg_dic = obj_dic_pick_up (env, &self.arg_dic, arity);
            if arity == lack {
                env.frame_stack.push (Box::new (Frame {
                    index: 0,
                    jojo: self.jojo.dup (),
                    scope: scope_extend (
                        &self.scope, arg_dic),
                }));
            } else {
                env.obj_stack.push (Ptr::new (Closure {
                    arg_dic,
                    jojo: self.jojo.dup (),
                    scope: self.scope.dup (),
                }));
            }
        }
    }
    pub type PrimFn = fn (env: &mut Env, arg_dic: &ObjDic);
    pub fn prim_fn_eq (
        lhs: &PrimFn,
        rhs: &PrimFn,
    ) -> bool {
        (*lhs) as usize == (*rhs) as usize
    }
    pub struct Prim {
        arg_dic: ObjDic,
        fun: PrimFn,
    }
    impl Obj for Prim {
        fn tag (&self) -> Tag { PRIM_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Prim> (other);
                (obj_dic_eq (&self.arg_dic, &other.arg_dic) &&
                 prim_fn_eq (&self.fun, &other.fun))
            }
        }

        fn apply (&self, env: &mut Env, arity: usize) {
            let lack = self.arg_dic.lack ();
            if arity > lack {
                eprintln! ("- Prim::apply");
                eprintln! ("  over-arity apply");
                eprintln! ("  arity > lack");
                eprintln! ("  arity : {}", arity);
                eprintln! ("  lack : {}", lack);
                panic! ("jojo fatal error!");
            }
            let fun = self.fun;
            let arg_dic = obj_dic_pick_up (env, &self.arg_dic, arity);
            if arity == lack {
                fun (env, &arg_dic);
            } else {
                env.obj_stack.push (Ptr::new (Prim {
                    arg_dic,
                    fun,
                }));
            }
        }
    }
    pub fn true_c () -> Ptr <Data> {
        Data::unit (TRUE_T)
    }
    pub fn false_c () -> Ptr <Data> {
        Data::unit (FALSE_T)
    }
    pub fn make_bool (b: bool) -> Ptr <Data> {
        if b {
            true_c ()
        }
        else {
            false_c ()
        }
    }
    pub struct Str { pub str: String }
    pub fn str_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (STR_T == tag)
    }
    impl <'a> ObjFrom <&'a str> for Str {
        fn obj (str: &'a str) -> Ptr <Str> {
            Ptr::new (Str { str: String::from (str) })
        }
    }
    impl Obj for Str {
        fn tag (&self) -> Tag { STR_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Str> (other);
                (self.str == other.str)
            }
        }
    }
    pub struct Sym { pub sym: String }
    pub fn sym_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (SYM_T == tag)
    }
    impl <'a> ObjFrom <&'a str> for Sym {
        fn obj (str: &'a str) -> Ptr <Sym> {
            Ptr::new (Sym { sym: String::from (str) })
        }
    }
    impl Obj for Sym {
        fn tag (&self) -> Tag { SYM_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Sym> (other);
                (self.sym == other.sym)
            }
        }
    }
    pub struct Num { pub num: f64 }
    pub fn num_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (NUM_T == tag)
    }
    impl ObjFrom <f64> for Num {
        fn obj (num: f64) -> Ptr <Num> {
            Ptr::new (Num { num })
        }
    }
    impl Obj for Num {
        fn tag (&self) -> Tag { NUM_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Num> (other);
                (self.num == other.num)
            }
        }

        fn repr (&self, _env: &Env) -> String {
            format! ("{}", self.num)
        }
    }
    pub fn null_c () -> Ptr <Obj> {
       Data::unit (NULL_T)
    }
    pub fn null_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (NULL_T == tag)
    }
    pub fn cons_c (car: Ptr <Obj>, cdr: Ptr <Obj>) -> Ptr <Obj> {
        Data::make (CONS_T, vec! [
            ("car", car),
            ("cdr", cdr),
        ])
    }
    pub fn cons_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (CONS_T == tag)
    }
    pub fn car (cons: Ptr <Obj>) -> Ptr <Obj> {
        assert_eq! (CONS_T, cons.tag ());
        cons.get ("car") .unwrap ()
    }
    pub fn cdr (cons: Ptr <Obj>) -> Ptr <Obj> {
        assert_eq! (CONS_T, cons.tag ());
        cons.get ("cdr") .unwrap ()
    }
    pub fn list_p (x: &Ptr <Obj>) -> bool {
        (null_p (x) ||
         cons_p (x))
    }
    pub fn unit_list (obj: Ptr <Obj>) -> Ptr <Obj> {
        cons_c (obj, null_c ())
    }
    pub fn nothing_c () -> Ptr <Obj> {
       Data::unit (NOTHING_T)
    }
    pub fn just_c (value: Ptr <Obj>) -> Ptr <Obj> {
        Data::make (JUST_T, vec! [
            ("value", value),
        ])
    }
    pub fn value_of_just (just: Ptr <Obj>) -> Ptr <Obj> {
        assert_eq! (JUST_T, just.tag ());
        just.get ("value") .unwrap ()
    }
    pub fn maybe_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (NOTHING_T == tag ||
         JUST_T == tag)
    }
    pub struct Vect { pub obj_vec: ObjVec }
    pub fn vect_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (VECT_T == tag)
    }
    impl <'a> ObjFrom <&'a ObjVec> for Vect {
        fn obj (obj_vec: &'a ObjVec) -> Ptr <Vect> {
            Ptr::new (Vect { obj_vec: obj_vec.clone () })
        }
    }
    impl Obj for Vect {
        fn tag (&self) -> Tag { VECT_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Vect> (other);
                (obj_vec_eq (&self.obj_vec, &other.obj_vec))
            }
        }
    }
    pub fn vect_to_list (vect: Ptr <Vect>) -> Ptr <Obj> {
        let obj_vec = &vect.obj_vec;
        let mut result = null_c ();
        for x in obj_vec .iter () .rev () {
            result = cons_c (x.dup (), result);
        }
        result
    }
    pub struct Dict { pub obj_dic: ObjDic }
    pub fn dict_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (DICT_T == tag)
    }
    impl <'a> ObjFrom <&'a ObjDic> for Dict {
        fn obj (obj_dic: &'a ObjDic) -> Ptr <Dict> {
            Ptr::new (Dict { obj_dic: obj_dic.clone () })
        }
    }
    impl Obj for Dict {
        fn tag (&self) -> Tag { DICT_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Dict> (other);
                (obj_dic_eq (&self.obj_dic, &other.obj_dic))
            }
        }
    }
    pub fn dict_to_list (dict: Ptr <Dict>) -> Ptr <Obj> {
        let mut result = null_c ();
        let obj_dic = &dict.obj_dic;
        for kv in obj_dic.iter () {
            let sym = Sym::obj (kv.0);
            let obj = kv.1;
            let pair = cons_c (sym, unit_list (obj.dup ()));
            result = cons_c (pair, result);
        }
        result
    }
    pub fn parse_sexp (token: &token::Token) -> Ptr <Obj> {
        match token {
            token::Token::List { token_vec, .. } => parse_sexp_list (token_vec),
            token::Token::Vect { token_vec, .. } => parse_sexp_vect (token_vec),
            token::Token::Dict { token_vec, .. } => parse_sexp_dict (token_vec),
            token::Token::QuotationMark { mark_name, token, .. } =>
                cons_c (Sym::obj (mark_name),
                        unit_list (parse_sexp (token))),
            token::Token::Num { num, .. } => Num::obj (*num),
            token::Token::Str { str, .. } => Str::obj (str),
            token::Token::Sym { sym, .. } => Sym::obj (sym),
        }
    }
    pub fn parse_sexp_list (token_vec: &token::TokenVec) -> Ptr <Obj> {
        let mut list = null_c ();
        token_vec
            .iter ()
            .rev ()
            .map (parse_sexp)
            .for_each (|obj| {
                list = cons_c (obj, list.dup ());
            });
        list
    }
    pub fn parse_sexp_vect (token_vec: &token::TokenVec) -> Ptr <Obj> {
        let obj_vec = token_vec
            .iter ()
            .map (parse_sexp)
            .collect::<ObjVec> ();
        Vect::obj (&obj_vec)
    }
    fn sexp_list_prefix_assign_with_last_sexp (
        sexp_list: Ptr <Obj>,
        last_sexp: Ptr <Obj>,
    ) -> Ptr <Obj> {
        if null_p (&sexp_list) {
            unit_list (last_sexp)
        } else {
            let head = car (sexp_list.dup ());
            if (sym_p (&head) &&
                obj_to::<Sym> (head.dup ())
                .sym .as_str () == "=")
            {
                let next = car (cdr (sexp_list.dup ()));
                let rest = cdr (cdr (sexp_list));
                let new_last_sexp = cons_c (
                    head, cons_c (
                        last_sexp,
                        unit_list (next)));
                cons_c (
                    new_last_sexp,
                    sexp_list_prefix_assign (rest))
            }
            else
            {
                let rest = cdr (sexp_list);
                cons_c (
                    last_sexp,
                    sexp_list_prefix_assign_with_last_sexp (rest, head))
            }
        }
    }
    pub fn sexp_list_prefix_assign (sexp_list: Ptr <Obj>) -> Ptr <Obj> {
        if null_p (&sexp_list) {
            sexp_list
        } else {
            sexp_list_prefix_assign_with_last_sexp (
                cdr (sexp_list.dup ()),
                car (sexp_list))
        }
    }
    pub fn parse_sexp_dict (token_vec: &token::TokenVec) -> Ptr <Obj> {
        let mut sexp_list = parse_sexp_list (token_vec);
        sexp_list = sexp_list_prefix_assign (sexp_list);
        let mut obj_dic = ObjDic::new ();
        while (cons_p (&sexp_list)) {
            let sexp = car (sexp_list.dup ());
            let name = car (cdr (sexp.dup ()));
            assert! (sym_p (&name));
            let name = obj_to::<Sym> (name);
            let value = car (cdr (cdr (sexp.dup ())));
            obj_dic.ins (&name.sym, Some (value));
            sexp_list = cdr (sexp_list.dup ())
        }
        Dict::obj (&obj_dic)
    }
    pub fn sexp_repr (env: &Env, sexp: Ptr <Obj>) -> String {
        if (null_p (&sexp)) {
            format! ("()")
        } else if (cons_p (&sexp)) {
            format! ("({})", sexp_list_repr (env, sexp))
        } else if (vect_p (&sexp)) {
            let v = obj_to::<Vect> (sexp);
            let l = vect_to_list (v);
            format! ("[{}]", sexp_list_repr (env, l))
        } else if (dict_p (&sexp)) {
            let d = obj_to::<Dict> (sexp);
            let l = dict_to_list (d);
            format! ("{{{}}}", sexp_list_repr (env, l))
        } else if (str_p (&sexp)) {
            let str = obj_to::<Str> (sexp);
            format! ("\"{}\"", str.str)
        } else if (sym_p (&sexp)) {
            let sym = obj_to::<Sym> (sexp);
            sym.sym.clone ()
        } else {
            sexp.repr (env)
        }
    }
    pub fn sexp_list_repr (env: &Env, sexp_list: Ptr <Obj>) -> String {
        if null_p (&sexp_list) {
            format! ("")
        } else if null_p (&cdr (sexp_list.dup ())) {
            sexp_repr (env, car (sexp_list))
        } else if (! cons_p (&cdr (sexp_list.dup ()))) {
            format! ("{} . {}",
                     sexp_repr (env, car (sexp_list.dup ())),
                     sexp_repr (env, cdr (sexp_list)))
        } else {
            format! ("{} {}",
                     sexp_repr (env, car (sexp_list.dup ())),
                     sexp_list_repr (env, cdr (sexp_list)))
        }
    }
    type StaticScope = HashMap <Name, (usize, usize)>;
    fn sexp_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        sexp: Ptr <Obj>,
    ) -> Ptr <JoVec> {
        unimplemented! ();
    //     if str_p (sexp) || num_p (sexp) {
    //         lit_compile (env, static_scope, sexp)
    //     } else if sym_p (sexp) {
    //         let sym = as_sym (sexp);
    //         symbol_compile (env, static_scope, sym->sym)
    //     } else if vect_p (sexp) {
    //         vect_compile (env, static_scope, as_vect (sexp))
    //     } else if dict_p (sexp) {
    //         dict_compile (env, static_scope, as_dict (sexp))
    //     } else if keyword_sexp_p (env, sexp) {
    //         keyword_compile (env, static_scope, sexp)
    //     } else if macro_sexp_p (env, sexp) {
    //         macro_compile (env, static_scope, sexp)
    //     } else if call_with_arg_dict_sexp_p (env, sexp) {
    //         call_with_arg_dict_compile (env, static_scope, sexp)
    //     } else {
    //         assert! (cons_p (&sexp));
    //         call_compile (env, static_scope, sexp)
    //     }
    }
    fn sexp_list_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        sexp_list: Ptr <Obj>,
    ) -> Ptr <JoVec> {
        let jojo = new_jojo ();
        if null_p (&sexp_list) {
            jojo
        } else {
            assert! (cons_p (&sexp_list));
            let head_jojo = sexp_compile (
                env, static_scope, car (sexp_list.dup ()));
            let body_jojo = sexp_list_compile (
                env, static_scope, cdr (sexp_list));
            jojo_append (&head_jojo, &body_jojo)
        }
    }
    #[test]
    fn test_step () {
        let mut env = Env::new ();

        let bye = env.define (
            "bye", Str::obj ("bye"));
        let world = env.define (
            "world", Str::obj ("world"));

        env.frame_stack.push (frame! [
            RefJo { id: world },
            RefJo { id: bye },
            RefJo { id: world }
        ]);

        env.run ();

        assert_eq! (3, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("world")));
        assert_eq! (2, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("bye")));
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("world")));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_apply () {
        let mut env = Env::new ();

        let bye = env.define (
            "bye", Str::obj ("bye"));
        let world = env.define (
            "world", Str::obj ("world"));

        env.frame_stack.push (frame! [
            RefJo { id: bye },
            RefJo { id: world },
            LambdaJo { arg_dic: Dic::from (vec! [ "x", "y" ]),
                       jojo: jojo! [
                           LocalRefJo { level: 0, index: 1 },
                           LocalRefJo { level: 0, index: 0 }
                       ] },
            ApplyJo { arity: 2 }
        ]);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("bye")));
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("world")));
        assert_eq! (0, env.obj_stack.len ());

        // curry

        env.frame_stack.push (frame! [
            RefJo { id: bye },
            RefJo { id: world },
            LambdaJo { arg_dic: Dic::from (vec! [ "x", "y" ]),
                       jojo: jojo! [
                           LocalRefJo { level: 0, index: 1 },
                           LocalRefJo { level: 0, index: 0 }
                       ] },
            ApplyJo { arity: 1 },
            ApplyJo { arity: 1 }
        ]);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("world")));
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("bye")));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_data () {
        let mut env = Env::new ();

        let last_cry = env.define (
            "last-cry",
            cons_c (Str::obj ("bye"),
                    Str::obj ("world")));

        env.frame_stack.push (frame! [
            RefJo { id: last_cry },
            DotJo { name: String::from ("cdr") },
            RefJo { id: last_cry },
            DotJo { name: String::from ("car") },
            RefJo { id: last_cry }
        ]);

        env.run ();
        assert_eq! (3, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            cons_c (Str::obj ("bye"),
                    Str::obj ("world"))));
        assert_eq! (2, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("bye")));
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("world")));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_data_cons () {
        let mut env = Env::new ();

        let bye = env.define (
            "bye", Str::obj ("bye"));
        let world = env.define (
            "world", Str::obj ("world"));
        let cons = env.define (
            "cons-c", DataCons::make (CONS_T, vec! ["car", "cdr"]));

        env.frame_stack.push (frame! [
            RefJo { id: bye },
            RefJo { id: world },
            RefJo { id: cons },
            ApplyJo { arity: 2 },
            DotJo { name: String::from ("car") }
        ]);

        env.run ();
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("bye")));
        assert_eq! (0, env.obj_stack.len ());

        // curry

        env.frame_stack.push (frame! [
            RefJo { id: bye },
            RefJo { id: world },
            RefJo { id: cons },
            ApplyJo { arity: 1 },
            ApplyJo { arity: 1 },
            DotJo { name: String::from ("car") }
        ]);

        env.run ();
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("world")));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_prim () {
        let mut env = Env::new ();

        let bye = env.define (
            "bye", Str::obj ("bye"));
        let world = env.define (
            "world", Str::obj ("world"));
        let swap = env.define (
            "swap", Ptr::new (Prim {
                arg_dic: Dic::from (vec! [ "x", "y" ]),
                fun: |env, arg_dic| {
                    let x = arg_dic.get ("x") .unwrap () .dup ();
                    let y = arg_dic.get ("y") .unwrap () .dup ();
                    env.obj_stack.push (y);
                    env.obj_stack.push (x);
                },
            }));

        env.frame_stack.push (frame! [
            RefJo { id: bye },
            RefJo { id: world },
            RefJo { id: swap },
            ApplyJo { arity: 2 }
        ]);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("bye")));
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("world")));
        assert_eq! (0, env.obj_stack.len ());

        // curry

        env.frame_stack.push (frame! [
            RefJo { id: bye },
            RefJo { id: world },
            RefJo { id: swap },
            ApplyJo { arity: 1 },
            ApplyJo { arity: 1 }
        ]);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("world")));
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::obj ("bye")));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_sexp () {
        println! ("\n<test_sexp>");
        let env = Env::new ();
        let p = |code| {
            let token_vec = token::scan (code);
            let sexp_list = parse_sexp_list (&token_vec);
            println! ("{} -> {}",
                      code,
                      sexp_list_repr (&env, sexp_list));
        };
        p ("()");
        p ("[]");
        p ("{}");
        p ("a b c");
        p ("a (b) c");
        p ("(a (b) c)");
        p ("[a b c]");
        p ("a [b] c");
        p ("[a [b] c]");
        p ("{(= a 1) (= b 2) (= c 3)}");
        println! ("</test_sexp>");
    }
