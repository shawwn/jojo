    // use std::rc::Rc;
    use std::sync::Arc;
    use std::collections::HashMap;
    use std::path::Path;
    use std::path::PathBuf;
    use std::fs;
    use std::env;
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

  pub type StringVec = Vec <String>;
  pub type NameVec = Vec <Name>;
  pub type TagVec = Vec <Tag>;
  pub type ObjVec = Vec <Ptr <Obj>>;
  pub type JoVec = Vec <Ptr <Jo>>;
      fn vec_peek <T> (vec: &Vec <T>, index: usize) -> &T {
            let back_index = vec.len () - index - 1;
            &vec [back_index]
      }
      fn str_vec_join (vec: Vec <&str>, c: char) -> String {
          let mut string = String::new ();
          for s in vec {
              string = format! ("{}{}", s, c);
          }
          string.pop ();
          string
      }
      pub fn obj_vec_eq (
          lhs: &ObjVec,
          rhs: &ObjVec,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter () .zip (rhs.iter ())
           .all (|p| obj_eq (&p.0, &p.1)))
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
      pub fn obj_stack_eq (
          lhs: &ObjVec,
          rhs: &ObjVec,
      ) -> bool {
          obj_vec_eq (lhs, rhs)
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
                      obj_eq (& (p.0).1, & (p.1).1))))
      }
      fn type_dic_eq (
          lhs: &TypeDic,
          rhs: &TypeDic,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter () .zip (rhs.iter ())
           .all (|p| ((p.0).0 == (p.1).0 &&
                      type_eq (& (p.0).1, & (p.1).1))))
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
          ( $( $x:expr ),* $(,)* ) => {{
              let jo_vec: JoVec = vec! [
                  $( Ptr::new ($x) ),*
              ];
              Ptr::new (jo_vec)
          }};
      }
      macro_rules! frame {
          ( $( $x:expr ),* $(,)* ) => {{
              let jo_vec: JoVec = vec! [
                  $( Ptr::new ($x) ),*
              ];
              Frame::make (jo_vec)
          }};
      }
      fn frame_stack_eq (
          lhs: &FrameStack,
          rhs: &FrameStack,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter () .zip (rhs.iter ())
           .all (|p| frame_eq (&p.0, &p.1)))
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
      pub const NONE_T      : Tag = 17;
      pub const SOME_T         : Tag = 18;
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
          preserve_tag (env, NONE_T         , "none-t");
          preserve_tag (env, SOME_T         , "some-t");
      }
      pub trait Dup {
         fn dup (&self) -> Self;
      }
      impl Dup for Ptr <Obj> {
          fn dup (&self) -> Self {
              Ptr::clone (self)
          }
      }
      impl Dup for Ptr <Type> {
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
        fn obj (x: T) -> Ptr <Self>;
    }
    pub struct Env {
        pub obj_dic: ObjDic,
        pub type_dic: TypeDic,
        pub obj_stack: ObjStack,
        pub frame_stack: FrameStack,
        pub current_dir: PathBuf,
        pub module_path: PathBuf,
    }

    impl Env {
        pub fn new () -> Env {
            let mut env = Env {
                obj_dic: ObjDic::new (),
                type_dic: TypeDic::new (),
                obj_stack: ObjStack::new (),
                frame_stack: FrameStack::new (),
                current_dir: env::current_dir () .unwrap (),
                module_path: PathBuf::new (),
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
    }
    impl Env {
        pub fn define (
            &mut self,
            name: &str,
            obj: Ptr <Obj>,
        ) -> Id {
            self.obj_dic.ins (name, Some (obj))
        }
    }
    // impl Env {
    //     pub fn find_type_from_prefix (
    //         &mut self,
    //         prefix: &str,
    //     ) -> Option <Ptr <Type>> {
    //         panic! ();
    //     }
    // }
    // impl Env {
    //     pub fn assign (
    //         &mut self,
    //         prefix: &str,
    //         name: &str,
    //         obj: Ptr <Obj>,
    //     ) -> Id {
    //         if prefix == "" {
    //             self.define (name, obj)
    //         } else {
    //             if let Some (typ) = self.find_type_from_prefix (prefix) {

    //             } else {

    //             }
    //         }
    //     }
    // }
    impl Env {
        pub fn define_type (
            &mut self,
            name: &str,
            typ: Ptr <Type>,
        ) -> Tag {
            self.type_dic.ins (name, Some (typ))
        }
    }

    fn env_eq (
        lhs: &Env,
        rhs: &Env,
    ) -> bool {
        (obj_dic_eq (&lhs.obj_dic, &rhs.obj_dic) &&
         type_dic_eq (&lhs.type_dic, &rhs.type_dic) &&
         obj_stack_eq (&lhs.obj_stack, &rhs.obj_stack) &&
         frame_stack_eq (&lhs.frame_stack, &rhs.frame_stack))
    }
    pub struct Frame {
        pub index: usize,
        pub jojo: Ptr <JoVec>,
        pub scope: Ptr <Scope>,
    }
    fn frame_eq (
        lhs: &Frame,
        rhs: &Frame,
    ) -> bool {
        (lhs.index == rhs.index &&
         jojo_eq (&lhs.jojo, &rhs.jojo) &&
         scope_eq (&lhs.scope, &rhs.scope))
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
    pub trait Obj {
        fn tag (&self) -> Tag;
        fn obj_dic (&self) -> Option <&ObjDic> { None }

        fn eq (&self, _other: Ptr <Obj>) -> bool { false }

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

        fn apply_to_arg_dict (&self, env: &mut Env) {
            eprintln! ("- Obj::apply_to_arg_dict");
            eprintln! ("  applying non applicable object");
            eprintln! ("  tag : {}", name_of_tag (&env, self.tag ()));
            eprintln! ("  obj : {}", self.repr (&env));
            panic! ("jojo fatal error!");
        }
    }
    pub fn obj_to <T: Obj> (obj: Ptr <Obj>) -> Ptr <T> {
        let obj_ptr = Ptr::into_raw (obj);
        unsafe {
            let obj_ptr = obj_ptr as *const Obj as *const T;
            Ptr::from_raw (obj_ptr)
        }
    }
    pub fn obj_eq (
        lhs: &Ptr <Obj>,
        rhs: &Ptr <Obj>,
    ) -> bool {
        lhs.eq (rhs.dup ())
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
    pub struct ApplyToArgDictJo;

    impl Jo for ApplyToArgDictJo {
        fn exe (&self, env: &mut Env, _: Ptr <Scope>) {
            let obj = env.obj_stack.pop () .unwrap ();
            obj.apply_to_arg_dict (env);
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
    pub struct LitJo {
        obj: Ptr <Obj>,
    }

    impl Jo for LitJo {
        fn exe (&self, env: &mut Env, _: Ptr <Scope>) {
            env.obj_stack.push (self.obj.dup ());
        }
    }
    pub struct Type {
        method_dic: ObjDic,
        tag_of_type: Tag,
        super_tag_vec: TagVec,
    }
    pub fn type_eq (
        lhs: &Ptr <Type>,
        rhs: &Ptr <Type>,
    ) -> bool {
        lhs.eq (rhs.dup ())
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
    pub type PrimFn = fn (
        env: &mut Env,
        arg_dic: &ObjDic,
    );
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
    fn car_as_sym (cons: Ptr <Obj>) -> Ptr <Sym> {
        assert! (cons_p (&cons));
        let head = car (cons);
        assert! (sym_p (&head));
        obj_to::<Sym> (head)
    }
    fn list_size (mut list: Ptr <Obj>) -> usize {
        assert! (list_p (&list));
        let mut size = 0;
        while ! null_p (&list) {
            size += 1;
            list = cdr (list);
        }
        size
    }
    fn list_rev (mut list: Ptr <Obj>) -> Ptr <Obj> {
        assert! (list_p (&list));
        let mut rev = null_c ();
        while ! null_p (&list) {
            let obj = car (list.dup ());
            rev = cons_c (obj, rev);
            list = cdr (list);
        }
        rev
    }
    pub fn unit_list (obj: Ptr <Obj>) -> Ptr <Obj> {
        cons_c (obj, null_c ())
    }
    pub fn none_c () -> Ptr <Obj> {
       Data::unit (NONE_T)
    }
    pub fn some_c (value: Ptr <Obj>) -> Ptr <Obj> {
        Data::make (SOME_T, vec! [
            ("value", value),
        ])
    }
    pub fn value_of_some (some: Ptr <Obj>) -> Ptr <Obj> {
        assert_eq! (SOME_T, some.tag ());
        some.get ("value") .unwrap ()
    }
    pub fn option_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (NONE_T == tag ||
         SOME_T == tag)
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
    fn list_to_vect (mut list: Ptr <Obj>) -> Ptr <Vect> {
        let mut obj_vec = ObjVec::new ();
        while cons_p (&list) {
            obj_vec.push (car (list.dup ()));
            list = cdr (list);
        }
        Vect::obj (&obj_vec)
    }
    struct CollectVectJo {
        counter: usize,
    }

    impl Jo for CollectVectJo {
        fn exe (&self, env: &mut Env, _: Ptr <Scope>) {
            let obj_vec = (0..self.counter)
                .into_iter ()
                .map (|_| env.obj_stack.pop () .unwrap ())
                .rev ()
                .collect::<ObjVec> ();
            env.obj_stack.push (Vect::obj (&obj_vec));
        }
    }
    fn vect_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        vect: Ptr <Vect>,
    ) -> Ptr <JoVec> {
        let sexp_list = vect_to_list (vect);
        let counter = list_size (sexp_list.dup ());
        let jojo = sexp_list_compile (
            env, static_scope, sexp_list);
        let ending_jojo = jojo! [
            CollectVectJo { counter },
        ];
        jojo_append (&jojo, &ending_jojo)
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
    pub fn dict_to_list_rev (dict: Ptr <Dict>) -> Ptr <Obj> {
        let mut list = null_c ();
        let obj_dic = &dict.obj_dic;
        for kv in obj_dic.iter () {
            let sym = Sym::obj (kv.0);
            let obj = kv.1;
            let pair = cons_c (sym, unit_list (obj.dup ()));
            list = cons_c (pair, list);
        }
        list
    }
    pub fn dict_to_list (dict: Ptr <Dict>) -> Ptr <Obj> {
        let list = dict_to_list_rev (dict);
        list_rev (list)
    }
    fn list_to_dict (mut list: Ptr <Obj>) -> Ptr <Dict> {
        let mut obj_dic = ObjDic::new ();
        while ! null_p (&list) {
            let pair = car (list.dup ());
            let key = car (pair.dup ());
            let rest = cdr (pair.dup ());
            assert! (sym_p (& key));
            let sym = obj_to::<Sym> (key);
            let name = &sym.sym;
            if cons_p (&rest) {
                let obj = car (rest);
                obj_dic.set (name, Some (obj));
            } else {
                obj_dic.set (name, None);
            }
            list = cdr (list);
        }
        Dict::obj (&obj_dic)
    }
    fn dict_to_flat_list_rev (dict: Ptr <Dict>) -> Ptr <Obj> {
        let mut list = null_c ();
        for kv in dict.obj_dic.iter () {
            let key = cons_c (
                Sym::obj ("quote"),
                unit_list (Sym::obj (kv.0)));
            let obj = kv.1.dup ();
            list = cons_c (obj, list);
            list = cons_c (key, list);
        }
        list
    }
    struct CollectDictJo {
        counter: usize,
    }

    impl Jo for CollectDictJo {
        fn exe (&self, env: &mut Env, _: Ptr <Scope>) {
            let mut obj_dic = ObjDic::new ();
            for _ in 0..self.counter {
                let key = env.obj_stack.pop () .unwrap ();
                let obj = env.obj_stack.pop () .unwrap ();
                assert! (sym_p (&key));
                let sym = obj_to::<Sym> (key);
                let name = sym.sym .as_str ();
                obj_dic.ins (name, Some (obj));
            }
            env.obj_stack.push (Dict::obj (&obj_dic));
        }
    }
    fn dict_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        dict: Ptr <Dict>,
    ) -> Ptr <JoVec> {
        let sexp_list = dict_to_flat_list_rev (dict);
        let counter = list_size (sexp_list.dup ());
        let counter = counter / 2;
        let jojo = sexp_list_compile (
            env, static_scope, sexp_list);
        let ending_jojo = jojo! [
            CollectDictJo { counter },
        ];
        jojo_append (&jojo, &ending_jojo)
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
            if sym_sexp_as_str_p (&head, "=") {
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
            let v = list_to_vect (l);
            let obj_vec = v.obj_vec
                .iter ()
                .map (|x| cons_c (Sym::obj ("="), x.dup ()))
                .collect ();
            let v = Vect::obj (&obj_vec);
            let l = vect_to_list (v);
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
    fn sym_sexp_as_str_p (sexp: &Ptr <Obj>, str: &str) -> bool {
        if ! sym_p (&sexp) {
            false
        } else {
            let sym = obj_to::<Sym> (sexp.dup ());
            (sym.sym .as_str () == str)
        }
    }
    pub type KeywordFn = fn (
        env: &mut Env,
        static_scope: &StaticScope,
        body: Ptr <Obj>,
    ) -> Ptr <JoVec>;
    pub fn keyword_fn_eq (
        lhs: &KeywordFn,
        rhs: &KeywordFn,
    ) -> bool {
        (*lhs) as usize == (*rhs) as usize
    }
    struct Keyword {
        fun: KeywordFn,
    }
    pub fn keyword_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (KEYWORD_T == tag)
    }
    impl Obj for Keyword {
        fn tag (&self) -> Tag { KEYWORD_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Keyword> (other);
                (keyword_fn_eq (&self.fun, &other.fun))
            }
        }
    }
    fn find_keyword (
        env: &Env,
        name: &str,
    ) -> Option <Ptr <Keyword>> {
        if let Some (obj) = env.obj_dic.get (name) {
            if keyword_p (obj) {
                let keyword = obj_to::<Keyword> (obj.dup ());
                Some (keyword)
            } else {
                None
            }
        } else {
            None
        }
    }
    fn keyword_sexp_p (env: &Env, sexp: &Ptr <Obj>) -> bool {
        if ! cons_p (&sexp) {
            return false;
        }
        let head = car (sexp.dup ());
        if ! sym_p (&head) {
            false
        } else {
            let sym = obj_to::<Sym> (head);
            let name = &sym.sym;
            if let Some (_) = find_keyword (env, name) {
                true
            } else {
                false
            }
        }
    }
    fn keyword_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        sexp: Ptr <Obj>,
    ) -> Ptr <JoVec> {
        let sym = car_as_sym (sexp.dup ());
        let name = &sym.sym;
        let keyword = find_keyword (env, name) .unwrap ();
        let body = cdr (sexp);
        (keyword.fun) (env, static_scope, body)
    }
    struct Macro {
        obj: Ptr <Obj>,
    }
    pub fn macro_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (MACRO_T == tag)
    }
    impl Obj for Macro {
        fn tag (&self) -> Tag { MACRO_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Macro> (other);
                (obj_eq (&self.obj, &other.obj))
            }
        }
    }
    fn find_macro (
        env: &Env,
        name: &str,
    ) -> Option <Ptr <Macro>> {
        if let Some (obj) = env.obj_dic.get (name) {
            if macro_p (obj) {
                let mac = obj_to::<Macro> (obj.dup ());
                Some (mac)
            } else {
                None
            }
        } else {
            None
        }
    }
    fn macro_sexp_p (env: &Env, sexp: &Ptr <Obj>) -> bool {
        if ! cons_p (&sexp) {
            return false;
        }
        let head = car (sexp.dup ());
        if ! sym_p (&head) {
            false
        } else {
            let sym = obj_to::<Sym> (head);
            let name = &sym.sym;
            if let Some (_) = find_macro (env, name) {
                true
            } else {
                false
            }
        }
    }
    fn macro_eval (
        env: &mut Env,
        sexp: Ptr <Obj>,
    ) -> Ptr <Obj> {
        let sym = car_as_sym (sexp.dup ());
        let name = &sym.sym;
        let mac = find_macro (env, name) .unwrap ();
        let body = cdr (sexp);
        env.obj_stack.push (body);
        let base = env.frame_stack.len ();
        mac.obj.apply (env, 1);
        env.run_with_base (base);
        env.obj_stack.pop () .unwrap ()
    }
    fn macro_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        sexp: Ptr <Obj>,
    ) -> Ptr <JoVec> {
        let new_sexp = macro_eval (env, sexp);
        sexp_compile (env, static_scope, new_sexp)
    }
    pub struct StaticRef {
        level: usize,
        index: usize,
    }
    fn static_ref_level_up (static_ref: &StaticRef) -> StaticRef {
        StaticRef {
          level: static_ref.level + 1,
          index: static_ref.index,
        }
    }
    pub type StaticScope = HashMap <Name, StaticRef>;
    fn static_scope_extend (
        old_static_scope: &StaticScope,
        name_vec: &NameVec,
    ) -> StaticScope {
        let mut static_scope: StaticScope = old_static_scope
            .iter ()
            .map (|kv| (kv.0.clone (), static_ref_level_up (kv.1)))
            .collect ();
        for (index, name) in name_vec .iter () .enumerate () {
            let static_ref = StaticRef { level: 0, index: index };
            static_scope.insert (name.clone (), static_ref);
        }
        static_scope
    }
    fn lit_compile (
        _env: &Env,
        _static_scope: &StaticScope,
        sexp: Ptr <Obj>,
    ) -> Ptr <JoVec> {
        jojo! [
            LitJo { obj: sexp },
        ]
    }
      fn dot_in_word_p (word: &str) -> bool {
          ((! token::str_word_p (word)) &&
           (word.find (".") != None))
      }
      fn dot_in_word_compile (
          env: &mut Env,
          static_scope: &StaticScope,
          word: &str,
      ) -> Ptr <JoVec> {
          let mut iter = word.split ('.');
          let name = iter.next () .unwrap ();
          let head_jojo = if name == "" {
              jojo! []
          } else {
              let sym = Sym::obj (name);
              sym_compile (env, static_scope, sym)
          };
          let mut jo_vec = JoVec::new ();
          for name in iter {
              let jo = Ptr::new (DotJo { name: String::from (name) });
              jo_vec.push (jo);
          }
          jojo_append (&head_jojo, &jo_vec)
      }
      fn ref_compile (
          env: &mut Env,
          static_scope: &StaticScope,
          name: &str,
      ) -> Ptr <JoVec> {
          if let Some (static_ref) = static_scope.get (name) {
              jojo! [
                  LocalRefJo {
                      level: static_ref.level,
                      index: static_ref.index,
                  }
              ]
          } else {
              if let Some (id) = env.obj_dic.get_index (name) {
                  jojo! [ RefJo { id } ]
              } else {
                  jojo! [
                      RefJo { id: env.obj_dic.ins (name, None) }
                  ]
              }
          }
      }
      fn sym_compile (
          env: &mut Env,
          static_scope: &StaticScope,
          sym: Ptr <Sym>,
      ) -> Ptr <JoVec> {
          let word = &sym.sym;
          if dot_in_word_p (word) {
              dot_in_word_compile (env, static_scope, word)
          } else {
              ref_compile (env, static_scope, word)
          }
      }
      fn apply_to_arg_dict_sexp_p (
          _env: &Env,
          sexp: &Ptr <Obj>,
      ) -> bool {
          if ! cons_p (sexp) {
              return false;
          }
          let mut body = sexp_list_prefix_assign (cdr (sexp.dup ()));
          while ! null_p (&body) {
              let head = car (body.dup ());
              if cons_p (&head) {
                  let head_car = car (head);
                  if sym_sexp_as_str_p (&head_car, "=") {
                      return true;
                  }
              } else {
                  return false;
              }
              body = cdr (body);
          }
          return false;
      }
      fn sexp_list_assign_to_pair (sexp_list: Ptr <Obj>) -> Ptr <Obj> {
          if null_p (& sexp_list) {
              sexp_list
          } else {
              cons_c (cdr (car (sexp_list.dup ())),
                      sexp_list_assign_to_pair (cdr (sexp_list)))
          }
      }
      fn sexp_list_to_dict (sexp_list: Ptr <Obj>) -> Ptr <Dict> {
          list_to_dict (
              sexp_list_assign_to_pair (
                  sexp_list_prefix_assign (sexp_list)))
      }
      pub fn apply_to_arg_dict_compile (
          env: &mut Env,
          static_scope: &StaticScope,
          sexp: Ptr <Obj>,
      ) -> Ptr <JoVec> {
          let head = car (sexp.dup ());
          let body = cdr (sexp);
          let jojo = jojo! [
              ApplyToArgDictJo {},
          ];
          let head_jojo = sexp_compile (env, static_scope, head);
          let dict = sexp_list_to_dict (body);
          let body_jojo = dict_compile (env, static_scope, dict);
          let jojo = jojo_append (&head_jojo, &jojo);
          let jojo = jojo_append (&body_jojo, &jojo);
          jojo
      }
      fn dot_word_p (word: &str) -> bool {
          (word.len () >= 1 &&
           word.starts_with ("."))
      }
      fn arity_of_body (mut body: Ptr <Obj>) -> usize {
          assert! (list_p (&body));
          let mut arity = 0;
          while ! null_p (&body) {
              let head = car (body.dup ());
              if ! sym_p (&head) {
                  arity += 1;
              } else {
                  let sym = obj_to::<Sym> (head.dup ());
                  let word = sym.sym .as_str ();
                  match word {
                      "drop" => arity -= 1,
                      "dup" | "over" | "tuck" => arity += 1,
                      "swap" => {}
                      _ if dot_word_p (word) => {}
                      _ => arity += 1,
                  }
              }
              body = cdr (body);
          }
          arity
      }
      fn apply_sexp_p (
          _env: &Env,
          sexp: &Ptr <Obj>,
      ) -> bool {
          cons_p (sexp)
      }
      pub fn apply_compile (
          env: &mut Env,
          static_scope: &StaticScope,
          sexp: Ptr <Obj>,
      ) -> Ptr <JoVec> {
          let head = car (sexp.dup ());
          let body = cdr (sexp);
          let arity = arity_of_body (body.dup ());
          let jojo = jojo! [
              ApplyJo { arity },
          ];
          let head_jojo = sexp_compile (env, static_scope, head);
          let body_jojo = sexp_list_compile (env, static_scope, body);
          let jojo = jojo_append (&head_jojo, &jojo);
          let jojo = jojo_append (&body_jojo, &jojo);
          jojo
      }
    pub fn sexp_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        sexp: Ptr <Obj>,
    ) -> Ptr <JoVec> {
        if str_p (&sexp) || num_p (&sexp) {
            lit_compile (env, static_scope, sexp)
        } else if sym_p (&sexp) {
            let sym = obj_to::<Sym> (sexp);
            sym_compile (env, static_scope, sym)
        } else if vect_p (&sexp) {
            let vect = obj_to::<Vect> (sexp);
            vect_compile (env, static_scope, vect)
        } else if dict_p (&sexp) {
            let dict = obj_to::<Dict> (sexp);
            dict_compile (env, static_scope, dict)
        } else if keyword_sexp_p (env, &sexp) {
            keyword_compile (env, static_scope, sexp)
        } else if macro_sexp_p (env, &sexp) {
            macro_compile (env, static_scope, sexp)
        } else if apply_to_arg_dict_sexp_p (env, &sexp) {
            apply_to_arg_dict_compile (env, static_scope, sexp)
        } else if apply_sexp_p (env, &sexp) {
            apply_compile (env, static_scope, sexp)
        } else {
            eprintln! ("- sexp_compile");
            eprintln! ("  unknown sexp : {}", sexp_repr (env, sexp));
            panic! ("jojo fatal error!");
        }
    }
    pub fn sexp_list_compile (
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
    struct Module {
        module_env: Env,
    }
    impl Obj for Module {
        fn tag (&self) -> Tag { MODULE_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Module> (other);
                (env_eq (&self.module_env, &other.module_env))
            }
        }
    }
    pub type TopKeywordFn = fn (
        env: &mut Env,
        body: Ptr <Obj>,
    );
    pub fn top_keyword_fn_eq (
        lhs: &TopKeywordFn,
        rhs: &TopKeywordFn,
    ) -> bool {
        (*lhs) as usize == (*rhs) as usize
    }
    struct TopKeyword {
        fun: TopKeywordFn,
    }
    pub fn top_keyword_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (TOP_KEYWORD_T == tag)
    }
    impl ObjFrom <TopKeywordFn> for TopKeyword {
        fn obj (fun: TopKeywordFn) -> Ptr <TopKeyword> {
            Ptr::new (TopKeyword {
                fun,
            })
        }
    }
    impl Obj for TopKeyword {
        fn tag (&self) -> Tag { TOP_KEYWORD_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<TopKeyword> (other);
                (top_keyword_fn_eq (&self.fun, &other.fun))
            }
        }
    }
    fn find_top_keyword (
        env: &Env,
        name: &str,
    ) -> Option <Ptr <TopKeyword>> {
        if let Some (obj) = env.obj_dic.get (name) {
            if top_keyword_p (obj) {
                let top_keyword = obj_to::<TopKeyword> (obj.dup ());
                Some (top_keyword)
            } else {
                None
            }
        } else {
            None
        }
    }
    fn top_keyword_sexp_p (env: &Env, sexp: &Ptr <Obj>) -> bool {
        if ! cons_p (&sexp) {
            return false;
        }
        let head = car (sexp.dup ());
        if ! sym_p (&head) {
            false
        } else {
            let sym = obj_to::<Sym> (head);
            let name = &sym.sym;
            if let Some (_) = find_top_keyword (env, name) {
                true
            } else {
                false
            }
        }
    }
    impl Env {
        pub fn define_top_keyword (
            &mut self,
            name: &str,
            fun: TopKeywordFn,
        ) -> Id {
            self.define (name, TopKeyword::obj (fun))
        }
    }
    fn jojo_run (
        env: &mut Env,
        scope: &Scope,
        jojo: Ptr <JoVec>,
    ) {
        let base = env.frame_stack.len ();
        let frame = Frame {
            index: 0,
            jojo,
            scope: Ptr::new (scope.clone ()),
        };
        env.frame_stack.push (Box::new (frame));
        env.run_with_base (base);
    }
    fn jojo_eval (
        env: &mut Env,
        scope: &Scope,
        jojo: Ptr <JoVec>,
    ) -> Ptr <Obj> {
        jojo_run (env, scope, jojo);
        env.obj_stack.pop () .unwrap ()
    }
    fn jojo_run_in_new_frame (
        env: &mut Env,
        jojo: Ptr <JoVec>,
    ) {
        let base = env.frame_stack.len ();
        let jo_vec = (*jojo).clone ();
        env.frame_stack.push (Frame::make (jo_vec));
        env.run_with_base (base);
    }
    fn jojo_eval_in_new_frame (
        env: &mut Env,
        jojo: Ptr <JoVec>,
    ) -> Ptr <Obj> {
        jojo_run_in_new_frame (env, jojo);
        env.obj_stack.pop () .unwrap ()
    }
    fn sexp_run (
        env: &mut Env,
        sexp: Ptr <Obj>,
    ) {
        if top_keyword_sexp_p (env, &sexp) {
            eprintln! ("- sexp_run");
            eprintln! ("  can not handle top_keyword_sexp");
            eprintln! ("  only `top_sexp_run` can handle top_keyword_sexp");
            eprintln! ("  sexp : {}", sexp_repr (env, sexp));
            panic! ("jojo fatal error!");
        } else {
            let static_scope = StaticScope::new ();
            let jojo = sexp_compile (env, &static_scope, sexp);
            jojo_run_in_new_frame (env, jojo);
        }
    }
    fn sexp_list_run (
        env: &mut Env,
        sexp_list: Ptr <Obj>,
    ) {
        if cons_p (&sexp_list) {
            sexp_run (env, car (sexp_list.dup ()));
            sexp_list_run (env, cdr (sexp_list));
        }
    }
    fn sexp_eval (
        env: &mut Env,
        sexp: Ptr <Obj>,
    ) -> Ptr <Obj> {
        let size_before = env.obj_stack.len ();
        sexp_run (env, sexp.dup ());
        let size_after = env.obj_stack.len ();
        if size_after - size_before == 1 {
            env.obj_stack.pop () .unwrap ()
        } else {
            eprintln! ("- sexp_eval mismatch");
            eprintln! ("  sexp must eval to one value");
            eprintln! ("  sexp : {}", sexp_repr (env, sexp));
            eprintln! ("  stack size before : {}", size_before);
            eprintln! ("  stack size after : {}", size_after);
            panic! ("jojo fatal error!");
        }
    }
    fn top_sexp_run (
        env: &mut Env,
        sexp: Ptr <Obj>,
    ) {
        if top_keyword_sexp_p (env, &sexp) {
            let sym = car_as_sym (sexp.dup ());
            let name = &sym.sym;
            let top_keyword = find_top_keyword (env, name) .unwrap ();
            let body = cdr (sexp);
            (top_keyword.fun) (env, body);
        } else {
            let static_scope = StaticScope::new ();
            let jojo = sexp_compile (env, &static_scope, sexp);
            jojo_run_in_new_frame (env, jojo);
            env.obj_stack.pop ();
        }
    }
    fn top_sexp_list_run_without_infix_assign (
        env: &mut Env,
        sexp_list: Ptr <Obj>,
    ) {
        if cons_p (&sexp_list) {
            top_sexp_run (env, car (sexp_list.dup ()));
            top_sexp_list_run_without_infix_assign (
                env, cdr (sexp_list));
        }
    }
    fn top_sexp_list_run (
        env: &mut Env,
        sexp_list: Ptr <Obj>,
    ) {
        top_sexp_list_run_without_infix_assign (
            env, sexp_list_prefix_assign (sexp_list));
    }
      fn prefix_of_word (word: &str) -> String {
          let mut vec = word.split ('.') .collect::<Vec <&str>> ();
          if vec.len () == 1 {
              String::new ()
          } else {
              vec.pop ();
              str_vec_join (vec, '.')
          }
      }
      fn name_of_word (word: &str) -> String {
          word.split ('.')
              .rev ()
              .next ()
              .unwrap ()
              .to_string ()
      }
      fn assign_data_p (body: &Ptr <Obj>) -> bool {
          (cons_p (&body) &&
           sym_p (&(car (body.dup ()))) &&
           cons_p (&(cdr (body.dup ()))) &&
           cons_p (&(car (cdr (body.dup ())))) &&
           sym_sexp_as_str_p (&(car (car (cdr (body.dup ())))), "data"))
      }
      fn name_t2c (name: &str) -> String {
          let mut name = name.to_string ();
          assert! (name.ends_with ("-t"));
          name.pop ();
          name.push ('c');
          name
      }
      fn tk_assign_data (
          env: &mut Env,
          body: Ptr <Obj>,
      ) {
          let sym = car_as_sym (body.dup ());
          let type_name = name_of_word (&sym.sym);
          let data_name = name_t2c (&type_name);
          let prefix = prefix_of_word (&sym.sym);
          let rest = cdr (body);
          let data_body = cdr (car (rest));
          let name_vect = list_to_vect (data_body);
          // let name_vector = name_vector_t ();
          // for (let obj: name_vect->obj_vector) {
          //     let sym = as_sym (obj);
          //     name_vector.push_back (sym->sym);
          // }
          // env.assign_type (env, &prefix, &type_name, tag_of_type, {});
          // env.assign_data (env, &prefix, &data_name, tag_of_type, name_vector);
      }
      fn assign_lambda_sugar_p (body: &Ptr <Obj>) -> bool {
          (cons_p (&body) &&
           cons_p (&(car (body.dup ()))))
      }
      fn assign_lambda_desugar (body: Ptr <Obj>) -> Ptr <Obj> {
          let head = car (body.dup ());
          let name = car (head.dup ());
          let arg_list = cdr (head);
          let rest = cdr (body);
          cons_c (name, unit_list (
              cons_c (Sym::obj ("lambda"),
                      cons_c (list_to_vect (arg_list),
                              rest))))
      }
      fn tk_assign_value (
          env: &mut Env,
          body: Ptr <Obj>,
      ) {
          let sym = car_as_sym (body.dup ());
          let name = name_of_word (&sym.sym);
          let prefix = prefix_of_word (&sym.sym);
          let rest = cdr (body);
          let rest_cdr = cdr (rest.dup ());
          assert! (null_p (&rest_cdr));
          let sexp = car (rest);
          let obj = sexp_eval (env, sexp);
          env.define (&name, obj);
          // env.assign (prefix, name, obj);
      }
      fn tk_assign (
          env: &mut Env,
          body: Ptr <Obj>,
      ) {
          if assign_data_p (&body) {
              tk_assign_data (env, body);
          } else if assign_lambda_sugar_p (&body) {
              tk_assign_value (env, assign_lambda_desugar (body));
          } else {
              tk_assign_value (env, body);
          }
      }
    fn expose_syntax (env: &mut Env) {
        env.define_top_keyword ("=", tk_assign);
        // env.define_keyword ("lambda", k_lambda);
        // env.define_keyword ("macro", k_macro);
        // env.define_keyword ("case", k_case);
        // env.define_keyword ("quote", k_quote);
        // env.define_keyword ("*", k_list);
        // env.define_keyword ("note", k_note);
        // env.define_keyword ("assert", k_assert);
        // env.define_keyword ("if", k_if);
        // env.define_keyword ("when", k_when);
        // env.define_keyword ("do", k_do);
        // env.define_prim_macro ("let", m_let);
        // env.define_prim_macro ("quasiquote", m_quasiquote);
        // env.define_prim_macro ("and", m_and);
        // env.define_prim_macro ("or", m_or);
        // env.define_prim_macro ("cond", m_cond);
    }
    fn expose_core (env: &mut Env) {
        // expose_bool (env);
        // expose_num (env);
        // expose_str (env);
        // expose_sym (env);
        // expose_list (env);
        // expose_vect (env);
        // expose_maybe (env);
        // expose_dict (env);
        // expose_sexp (env);
        // expose_top_keyword (env);
        // expose_keyword (env);
        // expose_system (env);
        // expose_module (env);
        expose_syntax (env);
        // expose_type (env);
        // expose_stack (env);
        // expose_misc (env);
    }
    fn code_run (
        env: &mut Env,
        code: &str,
    ) {
        let token_vec = token::scan (code);
        let sexp_list = parse_sexp_list (&token_vec);
        top_sexp_list_run (env, sexp_list);
    }
    fn code_from_module_path (
        module_path: &Path,
    ) -> String {
        let code = fs::read_to_string (module_path)
            .expect ("fail to read file");
        code
    }
    fn respect_current_dir (
        env: &Env,
        path: &Path,
    ) -> PathBuf {
        if path.is_absolute () {
            path.to_path_buf ()
        } else {
            let mut path_buf = env.current_dir.clone ();
            path_buf.push (path);
            path_buf
        }
    }
    fn respect_module_path (
        env: &Env,
        path: &Path,
    ) -> PathBuf {
        if path.is_absolute () {
            path.to_path_buf ()
        } else {
            let mut path_buf = env.module_path.clone ();
            path_buf.push (path);
            path_buf
        }
    }
    impl Env {
        pub fn from_module_path (module_path: &Path) -> Env {
            let mut env = Env::new ();
            env.module_path = respect_current_dir (&env, module_path);
            expose_core (&mut env);
            let code = code_from_module_path (module_path);
            code_run (&mut env, &code);
            env
        }
    }
    fn assert_pop (env: &mut Env, obj: Ptr <Obj>) {
        let pop = env.obj_stack.pop () .unwrap ();
        assert! (obj_eq (&obj, &pop));
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
            RefJo { id: world },
        ]);

        env.run ();

        assert_eq! (3, env.obj_stack.len ());
        assert_pop (&mut env, Str::obj ("world"));
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::obj ("bye"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::obj ("world"));
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
                           LocalRefJo { level: 0, index: 0 },
                       ] },
            ApplyJo { arity: 2 },
        ]);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::obj ("bye"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::obj ("world"));
        assert_eq! (0, env.obj_stack.len ());

        // curry

        env.frame_stack.push (frame! [
            RefJo { id: bye },
            RefJo { id: world },
            LambdaJo { arg_dic: Dic::from (vec! [ "x", "y" ]),
                       jojo: jojo! [
                           LocalRefJo { level: 0, index: 1 },
                           LocalRefJo { level: 0, index: 0 },
                       ] },
            ApplyJo { arity: 1 },
            ApplyJo { arity: 1 },
        ]);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::obj ("world"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::obj ("bye"));
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
            RefJo { id: last_cry },
        ]);

        env.run ();
        assert_eq! (3, env.obj_stack.len ());
        assert_pop (&mut env,
                    cons_c (Str::obj ("bye"),
                            Str::obj ("world")));
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::obj ("bye"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::obj ("world"));
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
            DotJo { name: String::from ("car") },
        ]);

        env.run ();
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::obj ("bye"));
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
        assert_pop (&mut env, Str::obj ("world"));
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
        assert_pop (&mut env, Str::obj ("bye"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::obj ("world"));
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
        assert_pop (&mut env, Str::obj ("world"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::obj ("bye"));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_sexp () {
        fn pr (code: &str) -> String {
            // parse and repr
            let env = Env::new ();
            let token_vec = token::scan (code);
            let sexp_list = parse_sexp_list (&token_vec);
            sexp_list_repr (&env, sexp_list)
        }

        fn pr_eq (lhs: &str, rhs: &str) -> bool {
            let lhs_pr = pr (lhs);
            rhs == &lhs_pr
        }

        fn pr_same (code: &str) -> bool {
            pr_eq (code, code)
        }

        assert! (pr_same ("()"));
        assert! (pr_same ("[]"));
        assert! (pr_same ("{}"));
        assert! (pr_same ("a b c"));
        assert! (pr_same ("a (b) c"));
        assert! (pr_same ("(a (b) c)"));
        assert! (pr_same ("[a b c]"));
        assert! (pr_same ("a [b] c"));
        assert! (pr_same ("[a [b] c]"));
        assert! (pr_same ("{(= a 1) (= b 2) (= c 3)}"));
        assert! (pr_eq ("{a = 1, b = 2, c = 3}",
                        "{(= a 1) (= b 2) (= c 3)}"));

        println! ("\n<test_sexp>"); {
            fn p (code: &str) {
                let env = Env::new ();
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
        }
        println! ("</test_sexp>");
    }
