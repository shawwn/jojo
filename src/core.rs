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
  pub type CharVec = Vec <char>;
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
      pub fn obj_vec_rev (obj_vec: &ObjVec) -> ObjVec {
          obj_vec
              .clone ()
              .into_iter ()
              .rev ()
              .collect::<ObjVec> ()
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
          let index = env.type_dic.ins (name, Some (Type::make (tag)));
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
      // impl Dup for Ptr <Type> {
      //     fn dup (&self) -> Self {
      //         Ptr::clone (self)
      //     }
      // }
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
      impl Dup for Ptr <ObjDic> {
          fn dup (&self) -> Self {
              Ptr::clone (self)
          }
      }
      macro_rules! impl_tag {
          ( $type:ty, $tag:expr ) => {
              impl $type {

                  pub fn tag () -> Tag {
                      $tag
                  }

                  pub fn cast (obj: Ptr <Obj>) -> Ptr <Self> {
                      assert! (Self::p (&obj));
                      unsafe {
                          obj_to::<Self> (obj)
                      }
                  }

                  pub fn p (x: &Ptr <Obj>) -> bool {
                      let tag = x.tag ();
                      (Self::tag () == tag)
                  }
              }};
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
            if self.obj_dic.has_name (name) {
                if let Some (old_obj) = self.obj_dic.get (name) {
                    eprintln! ("- Env::define");
                    eprintln! ("  re-defining a name is not allowed");
                    eprintln! ("  name : {}", name);
                    eprintln! ("  old obj : {}", old_obj.repr (self));
                    eprintln! ("  new obj : {}", obj.repr (self));
                    panic! ("jojo fatal error!");
                } else {
                    self.obj_dic.set (name, Some (obj));
                }
                self.obj_dic.get_index (name) .unwrap ()
            } else {
               self.obj_dic.ins (name, Some (obj))
            }
        }
    }
    impl Env {
        pub fn find_type (
            &mut self,
            name: &str,
        ) -> Option <Ptr <Type>> {
            if let Some (typ) = self.type_dic.get (name) {
                Some (typ.dup ())
            } else {
                None
            }
        }
    }
    fn method_dic_extend (
        obj_dic: &ObjDic,
        name: &str,
        obj: Ptr <Obj>,
    ) -> Ptr <ObjDic> {
        let mut obj_dic = obj_dic.clone ();
        if obj_dic.has_name (name) {
            obj_dic.set (name, Some (obj));
        } else {
            obj_dic.ins (name, Some (obj));
        }
        Ptr::new (obj_dic)
    }
    impl Env {
        pub fn assign (
            &mut self,
            type_name: &str,
            name: &str,
            obj: Ptr <Obj>,
        ) {
            if type_name == "" {
                self.define (name, obj);
            } else {
                if let Some (typ) = self.find_type (type_name) {
                    let new_typ = Ptr::new (Type  {
                        method_dic: method_dic_extend (
                            &typ.method_dic, name, obj),
                        tag_of_type: typ.tag_of_type,
                        super_tag_vec: typ.super_tag_vec.clone (),
                    });
                    self.type_dic.set (type_name, Some (new_typ));
                } else {
                    eprintln! ("- Env::assign");
                    eprintln! ("  unknown type_name : {}", type_name);
                    eprintln! ("  name : {}", name);
                    eprintln! ("  obj : {}", obj.repr (self));
                    panic! ("jojo fatal error!");
                }
            }
        }
    }
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

        fn obj_dic (&self) -> Option <Ptr <ObjDic>> { None }

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
            print! ("{}", self.repr (&env));
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
    /// Before cast an obj to T, caller must check that
    ///   the obj has the tag of T.
    unsafe fn obj_to <T: Obj> (obj: Ptr <Obj>) -> Ptr <T> {
        let obj_ptr = Ptr::into_raw (obj);
        let obj_ptr = obj_ptr as *const Obj as *const T;
        Ptr::from_raw (obj_ptr)
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
    pub struct TypeRefJo {
        tag: Tag,
    }

    impl Jo for TypeRefJo {
        fn exe (&self, env: &mut Env, _: Ptr <Scope>) {
            let entry = env.type_dic.idx (self.tag);
            if let Some (typ) = &entry.value {
                env.obj_stack.push (typ.dup ());
            } else {
                eprintln! ("- TypeRefJo::exe");
                eprintln! ("  undefined name : {}", entry.name);
                eprintln! ("  tag : {}", self.tag);
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
        arg_dic: Ptr <ObjDic>,
        jojo: Ptr <JoVec>,
    }

    impl Jo for LambdaJo {
        fn exe (&self, env: &mut Env, scope: Ptr <Scope>) {
            env.obj_stack.push (Ptr::new (Closure {
                arg_dic: self.arg_dic.dup (),
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
        method_dic: Ptr <ObjDic>,
        tag_of_type: Tag,
        super_tag_vec: TagVec,
    }

    impl_tag! (Type, TYPE_T);

    impl Obj for Type {
        fn tag (&self) -> Tag { TYPE_T }

        fn obj_dic (&self) -> Option <Ptr <ObjDic>> {
            Some (self.method_dic.dup ())
        }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Type::cast (other);
                (self.tag_of_type == other.tag_of_type &&
                 self.super_tag_vec == other.super_tag_vec)
            }
        }
    }
    pub fn type_eq (
        lhs: &Ptr <Type>,
        rhs: &Ptr <Type>,
    ) -> bool {
        lhs.eq (rhs.dup ())
    }
    impl Type {
        fn make (tag: Tag) -> Ptr <Type> {
            Ptr::new (Type {
                method_dic: Ptr::new (ObjDic::new ()),
                tag_of_type: tag,
                super_tag_vec: TagVec::new (),
            })
        }
    }
    fn type_of (env: &Env, obj: Ptr <Obj>) -> Ptr <Type> {
        let tag = obj.tag ();
        let entry = env.type_dic.idx (tag);
        if let Some (typ) = &entry.value {
            typ.dup ()
        } else {
            eprintln! ("- type_of");
            eprintln! ("  obj : {}", obj.repr (env));
            eprintln! ("  tag : {}", tag);
            panic! ("jojo fatal error!");
        }
    }
    pub struct Data {
        tag_of_type: Tag,
        field_dic: Ptr <ObjDic>,
    }

    impl Obj for Data {
        fn tag (&self) -> Tag { self.tag_of_type }

        fn obj_dic (&self) -> Option <Ptr <ObjDic>> {
            Some (self.field_dic.dup ())
        }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                unsafe {
                    let other = obj_to::<Data> (other);
                    (self.tag_of_type == other.tag_of_type &&
                     obj_dic_eq (&self.field_dic, &other.field_dic))
                }
            }
        }
    }
    impl Data {
        fn make (
            tag: Tag,
            vec: Vec <(&str, Ptr <Obj>)>,
        ) -> Ptr <Data> {
            Ptr::new (Data {
                tag_of_type: tag,
                field_dic: Ptr::new (Dic::from (vec)),
            })
        }
    }
    impl Data {
        fn unit (tag: Tag) -> Ptr <Data> {
            Ptr::new (Data {
                tag_of_type: tag,
                field_dic: Ptr::new (ObjDic::new ()),
            })
        }
    }
    pub struct DataCons {
        tag_of_type: Tag,
        field_dic: Ptr <ObjDic>,
    }

    impl_tag! (DataCons, DATA_CONS_T);

    impl Obj for DataCons {
        fn tag (&self) -> Tag { DATA_CONS_T }

        fn obj_dic (&self) -> Option <Ptr <ObjDic>> {
            Some (self.field_dic.dup ())
        }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = DataCons::cast (other);
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
                    field_dic: Ptr::new (field_dic),
                }));
            } else {
                env.obj_stack.push (Ptr::new (DataCons {
                    tag_of_type,
                    field_dic: Ptr::new (field_dic),
                }));
            }
        }
    }
    impl DataCons {
        pub fn make (
            tag: Tag,
            vec: Vec <String>,
        ) -> Ptr <DataCons> {
            Ptr::new (DataCons {
                tag_of_type: tag,
                field_dic: Ptr::new (Dic::from (vec)),
            })
        }
    }
    impl DataCons {
        pub fn unit (
            tag: Tag,
        ) -> Ptr <DataCons> {
            Ptr::new (DataCons {
                tag_of_type: tag,
                field_dic: Ptr::new (ObjDic::new ()),
            })
        }
    }
    pub struct Closure {
        arg_dic: Ptr <ObjDic>,
        jojo: Ptr <JoVec>,
        scope: Ptr <Scope>,
    }

    impl_tag! (Closure, CLOSURE_T);

    impl Obj for Closure {
        fn tag (&self) -> Tag { CLOSURE_T }

        fn obj_dic (&self) -> Option <Ptr <ObjDic>> {
            Some (self.arg_dic.dup ())
        }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Closure::cast (other);
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
                    arg_dic: Ptr::new (arg_dic),
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

    impl_tag! (Prim, PRIM_T);

    impl Obj for Prim {
        fn tag (&self) -> Tag { PRIM_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Prim::cast (other);
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
    impl Env {
        pub fn define_prim (
            &mut self,
            name: &str,
            name_vec: Vec <&str>,
            fun: PrimFn,
        ) -> Id {
            let arg_vec = name_vec. iter ()
                .map (|x| x.to_string ())
                .collect::<NameVec> ();
            self.define (name, Ptr::new (Prim {
                arg_dic: Dic::from (arg_vec),
                fun,
            }))
        }
    }
    macro_rules! define_prim {
        ( $env:expr, $name:expr,
          [$arg0:expr],
          $fun:expr ) => {
            ($env).define_prim (
                $name,
                vec! [$arg0],
                |env, arg| {
                    env.obj_stack.push ($fun (
                        arg_idx (arg, 0)));
                });
        };

        ( $env:expr, $name:expr,
          [$arg0:expr, $arg1:expr],
          $fun:expr ) => {
            ($env).define_prim (
                $name,
                vec! [$arg0, $arg1],
                |env, arg| {
                    env.obj_stack.push ($fun (
                        arg_idx (arg, 0),
                        arg_idx (arg, 1)));
                });
        };

        ( $env:expr, $name:expr,
          [$arg0:expr, $arg1:expr, $arg2:expr],
          $fun:expr ) => {
            ($env).define_prim (
                $name,
                vec! [$arg0, $arg1, $arg2],
                |env, arg| {
                    env.obj_stack.push ($fun (
                        arg_idx (arg, 0),
                        arg_idx (arg, 1),
                        arg_idx (arg, 2)));
                });
        };

        ( $env:expr, $name:expr,
          [$arg0:expr, $arg1:expr, $arg2:expr, $arg3:expr],
          $fun:expr ) => {
            ($env).define_prim (
                $name,
                vec! [$arg0, $arg1, $arg2, $arg3],
                |env, arg| {
                    env.obj_stack.push ($fun (
                        arg_idx (arg, 0),
                        arg_idx (arg, 1),
                        arg_idx (arg, 2),
                        arg_idx (arg, 3)));
                });
        };
    }
    pub struct True;

    impl_tag! (True, TRUE_T);

    impl Obj for True {
        fn tag (&self) -> Tag { TRUE_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                true
            }
        }
    }
    impl True {
        fn make () -> Ptr <True> {
            Ptr::new (True {})
        }
    }
    pub struct False;

    impl_tag! (False, FALSE_T);

    impl Obj for False {
        fn tag (&self) -> Tag { FALSE_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                true
            }
        }
    }
    impl False {
        fn make () -> Ptr <False> {
            Ptr::new (False {})
        }
    }
    pub fn true_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (TRUE_T == tag)
    }

    pub fn false_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (FALSE_T == tag)
    }
    pub fn not (x: Ptr <Obj>) -> Ptr <Obj> {
        make_bool (false_p (&x))
    }
    pub fn make_bool (b: bool) -> Ptr <Obj> {
        if b {
            True::make ()
        }
        else {
            False::make ()
        }
    }
    pub struct Str { pub str: String }

    impl_tag! (Str, STR_T);

    impl Obj for Str {
        fn tag (&self) -> Tag { STR_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Str::cast (other);
                (self.str == other.str)
            }
        }
    }
    impl Str {
        fn make (str: &str) -> Ptr <Str> {
            Ptr::new (Str { str: String::from (str) })
        }
    }
    fn str_length (str: Ptr <Obj>) -> Ptr <Num> {
        let str = Str::cast (str);
        Num::make (str.str.len () as f64)
    }
    fn str_append (
        ante: Ptr <Obj>,
        succ: Ptr <Obj>,
    ) -> Ptr <Str> {
        let ante = Str::cast (ante);
        let succ = Str::cast (succ);
        Str::make (&format! ("{}{}", ante.str, succ.str))
    }
    fn str_slice (
        str: Ptr <Obj>,
        begin: Ptr <Obj>,
        end: Ptr <Obj>,
    ) -> Ptr <Str> {
        let str = Str::cast (str);
        let begin = Num::cast (begin);
        let end = Num::cast (end);
        let char_vec = str.str.chars() .collect::<CharVec> ();
        let begin = begin.num as usize;
        let end = end.num as usize;
        let slice = &char_vec [begin..end];
        Str::make (&slice .iter () .collect::<String> ())
    }
    fn str_ref (
        str: Ptr <Obj>,
        index: Ptr <Obj>,
    ) -> Ptr <Str> {
        str_slice (str, index.dup (), inc (index))
    }
    fn str_head (str: Ptr <Obj>) -> Ptr <Str> {
        str_ref (str, Num::make (0.0))
    }
    fn str_rest (str: Ptr <Obj>) -> Ptr <Str> {
        let len = str_length (str.dup ());
        str_slice (str, Num::make (1.0), len)
    }
    pub struct Sym { pub sym: String }

    impl_tag! (Sym, SYM_T);

    impl Obj for Sym {
        fn tag (&self) -> Tag { SYM_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Sym::cast (other);
                (self.sym == other.sym)
            }
        }
    }
    impl Sym {
        fn make (str: &str) -> Ptr <Sym> {
            Ptr::new (Sym { sym: String::from (str) })
        }
    }
    fn sym_length (sym: Ptr <Obj>) -> Ptr <Num> {
        let sym = Sym::cast (sym);
        Num::make (sym.sym.len () as f64)
    }
    fn sym_append (
        ante: Ptr <Obj>,
        succ: Ptr <Obj>,
    ) -> Ptr <Sym> {
        let ante = Sym::cast (ante);
        let succ = Sym::cast (succ);
        Sym::make (&format! ("{}{}", ante.sym, succ.sym))
    }
    fn sym_slice (
        sym: Ptr <Obj>,
        begin: Ptr <Obj>,
        end: Ptr <Obj>,
    ) -> Ptr <Sym> {
        let sym = Sym::cast (sym);
        let begin = Num::cast (begin);
        let end = Num::cast (end);
        let char_vec = sym.sym.chars() .collect::<CharVec> ();
        let begin = begin.num as usize;
        let end = end.num as usize;
        let slice = &char_vec [begin..end];
        Sym::make (&slice .iter () .collect::<String> ())
    }
    fn sym_ref (
        sym: Ptr <Obj>,
        index: Ptr <Obj>,
    ) -> Ptr <Sym> {
        sym_slice (sym, index.dup (), inc (index))
    }
    fn sym_head (sym: Ptr <Obj>) -> Ptr <Sym> {
        sym_ref (sym, Num::make (0.0))
    }
    fn sym_rest (sym: Ptr <Obj>) -> Ptr <Sym> {
        let len = sym_length (sym.dup ());
        sym_slice (sym, Num::make (1.0), len)
    }
    pub struct Num { pub num: f64 }

    impl_tag! (Num, NUM_T);

    impl Obj for Num {
        fn tag (&self) -> Tag { NUM_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Num::cast (other);
                (self.num == other.num)
            }
        }

        fn repr (&self, _env: &Env) -> String {
            format! ("{}", self.num)
        }
    }
    impl Num {
        fn make (num: f64) -> Ptr <Num> {
            Ptr::new (Num { num })
        }
    }
    fn inc (x: Ptr <Obj>) -> Ptr <Num> {
        let x = Num::cast (x);
        Num::make (x.num + 1.0)
    }
    fn dec (x: Ptr <Obj>) -> Ptr <Num> {
        let x = Num::cast (x);
        Num::make (x.num - 1.0)
    }
    fn add (x: Ptr <Obj>, y: Ptr <Obj>) -> Ptr <Num> {
        let x = Num::cast (x);
        let y = Num::cast (y);
        Num::make (x.num + y.num)
    }
    fn sub (x: Ptr <Obj>, y: Ptr <Obj>) -> Ptr <Num> {
        let x = Num::cast (x);
        let y = Num::cast (y);
        Num::make (x.num - y.num)
    }
    fn mul (x: Ptr <Obj>, y: Ptr <Obj>) -> Ptr <Num> {
        let x = Num::cast (x);
        let y = Num::cast (y);
        Num::make (x.num * y.num)
    }
    fn div (x: Ptr <Obj>, y: Ptr <Obj>) -> Ptr <Num> {
        let x = Num::cast (x);
        let y = Num::cast (y);
        Num::make (x.num / y.num)
    }
    pub struct Null;

    impl_tag! (Null, NULL_T);

    impl Obj for Null {
        fn tag (&self) -> Tag { NULL_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                true
            }
        }
    }
    impl Null {
        fn make () -> Ptr <Null> {
            Ptr::new (Null {})
        }
    }
    pub fn null () -> Ptr <Obj> {
       Null::make ()
    }
    pub struct Cons {
        car: Ptr <Obj>,
        cdr: Ptr <Obj>,
    }

    impl_tag! (Cons, CONS_T);

    impl Obj for Cons {
        fn tag (&self) -> Tag { CONS_T }

        fn obj_dic (&self) -> Option <Ptr <ObjDic>> {
            let mut obj_dic = ObjDic::new ();
            obj_dic.ins ("car", Some (self.car.dup ()));
            obj_dic.ins ("cdr", Some (self.cdr.dup ()));
            Some (Ptr::new (obj_dic))
        }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Cons::cast (other);
                (obj_eq (&self.car, &other.car) &&
                 obj_eq (&self.cdr, &other.cdr))
            }
        }
    }
    impl Cons {
        fn make (car: Ptr <Obj>, cdr: Ptr <Obj>) -> Ptr <Cons> {
            Ptr::new (Cons { car, cdr })
        }
    }
    pub fn cons (car: Ptr <Obj>, cdr: Ptr <Obj>) -> Ptr <Obj> {
        Cons::make (car, cdr)
    }
    pub fn null_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (NULL_T == tag)
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
        Sym::cast (head)
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
    fn list_length (list: Ptr <Obj>) -> Ptr <Num> {
        assert! (list_p (&list));
        Num::make (list_size (list) as f64)
    }
    fn list_reverse (mut list: Ptr <Obj>) -> Ptr <Obj> {
        assert! (list_p (&list));
        let mut rev = null ();
        while ! null_p (&list) {
            let obj = car (list.dup ());
            rev = cons (obj, rev);
            list = cdr (list);
        }
        rev
    }
    fn list_reverse_append (
        ante: Ptr <Obj>,
        succ: Ptr <Obj>,
    ) -> Ptr <Obj> {
        let mut list = ante;
        let mut result = succ;
        while ! null_p (&list) {
            let obj = car (list.dup ());
            result = cons (obj, result);
            list = cdr (list);
        }
        result
    }
    fn list_append (
        ante: Ptr <Obj>,
        succ: Ptr <Obj>,
    ) -> Ptr <Obj> {
        list_reverse_append (list_reverse (ante), succ)
    }
    pub fn unit_list (obj: Ptr <Obj>) -> Ptr <Obj> {
        cons (obj, null ())
    }
    pub struct JNone;

    impl_tag! (JNone, NONE_T);

    impl Obj for JNone {
        fn tag (&self) -> Tag { NONE_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                true
            }
        }
    }
    impl JNone {
        fn make () -> Ptr <JNone> {
            Ptr::new (JNone {})
        }
    }
    pub struct JSome {
        value: Ptr <Obj>,
    }

    impl_tag! (JSome, SOME_T);

    impl Obj for JSome {
        fn tag (&self) -> Tag { SOME_T }

        fn obj_dic (&self) -> Option <Ptr <ObjDic>> {
            let mut obj_dic = ObjDic::new ();
            obj_dic.ins ("value", Some (self.value.dup ()));
            Some (Ptr::new (obj_dic))
        }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = JSome::cast (other);
                (obj_eq (&self.value, &other.value))
            }
        }
    }
    impl JSome {
        fn make (value: Ptr <Obj>) -> Ptr <JSome> {
            Ptr::new (JSome { value })
        }
    }
    pub fn some (value: Ptr <Obj>) -> Ptr <JSome> {
        JSome::make (value)
    }
    pub fn option_p (x: &Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (NONE_T == tag ||
         SOME_T == tag)
    }
    pub struct Vect { pub obj_vec: ObjVec }

    impl_tag! (Vect, VECT_T);

    impl Obj for Vect {
        fn tag (&self) -> Tag { VECT_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Vect::cast (other);
                (obj_vec_eq (&self.obj_vec, &other.obj_vec))
            }
        }
    }
    impl Vect {
        fn make (obj_vec: &ObjVec) -> Ptr <Vect> {
            Ptr::new (Vect { obj_vec: obj_vec.clone () })
        }
    }
    pub fn vect_to_list (vect: Ptr <Obj>) -> Ptr <Obj> {
        let vect = Vect::cast (vect);
        let obj_vec = &vect.obj_vec;
        let mut result = null ();
        for x in obj_vec .iter () .rev () {
            result = cons (x.dup (), result);
        }
        result
    }
    fn list_to_vect (mut list: Ptr <Obj>) -> Ptr <Vect> {
        let mut obj_vec = ObjVec::new ();
        while cons_p (&list) {
            obj_vec.push (car (list.dup ()));
            list = cdr (list);
        }
        Vect::make (&obj_vec)
    }
    struct CollectVectJo {
        counter: usize,
    }

    impl Jo for CollectVectJo {
        fn exe (&self, env: &mut Env, _: Ptr <Scope>) {
            let mut obj_vec = ObjVec::new ();
            for _ in 0..self.counter {
                let obj = env.obj_stack.pop () .unwrap ();
                obj_vec.push (obj);
            }
            let obj_vec = obj_vec_rev (&obj_vec);
            env.obj_stack.push (Vect::make (&obj_vec));
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
    fn name_vect_to_name_vec (name_vect: Ptr <Vect>) -> NameVec {
        name_vect.obj_vec .iter ()
            .map (|x| {
                let sym = Sym::cast (x.dup ());
                sym.sym.to_string ()
            })
            .collect::<NameVec> ()
    }
    fn vect_length (vect: Ptr <Obj>) -> Ptr <Obj> {
        let vect = Vect::cast (vect);
        Num::make (vect.obj_vec.len () as f64)
    }
    fn vect_append (
        ante: Ptr <Obj>,
        succ: Ptr <Obj>,
    ) -> Ptr <Obj> {
        let ante = Vect::cast (ante);
        let succ = Vect::cast (succ);
        let mut ante_obj_vec = ante.obj_vec.clone ();
        let mut succ_obj_vec = succ.obj_vec.clone ();
        ante_obj_vec.append (&mut succ_obj_vec);
        Vect::make (&ante_obj_vec)
    }
    fn vect_slice (
        vect: Ptr <Obj>,
        begin: Ptr <Obj>,
        end: Ptr <Obj>,
    ) -> Ptr <Obj> {
        let vect = Vect::cast (vect);
        let begin = Num::cast (begin);
        let end = Num::cast (end);
        let begin = begin.num as usize;
        let end = end.num as usize;
        let obj_vec = ObjVec::from (&vect.obj_vec [begin..end]);
        Vect::make (&obj_vec)
    }
    fn vect_ref (
        vect: Ptr <Obj>,
        index: Ptr <Obj>,
    ) -> Ptr <Obj> {
        let vect = Vect::cast (vect);
        let index = Num::cast (index);
        let index = index.num as usize;
        let obj = &vect.obj_vec[index];
        obj.dup ()
    }
    fn vect_head (
        vect: Ptr <Obj>,
    ) -> Ptr <Obj> {
        let index = Num::make (0.0);
        vect_ref (vect, index)
    }
    fn vect_rest (
        vect: Ptr <Obj>,
    ) -> Ptr <Obj> {
        let begin = Num::make (1.0);
        let end = vect_length (vect.dup ());
        vect_slice (vect, begin, end)
    }
    fn vect_reverse (
        vect: Ptr <Obj>,
    ) -> Ptr <Vect> {
        let vect = Vect::cast (vect);
        let obj_vec = obj_vec_rev (&vect.obj_vec);
        Vect::make (&obj_vec)
    }
    fn unit_vect (
        obj: Ptr <Obj>,
    ) -> Ptr <Obj> {
        let mut obj_vec = ObjVec::new ();
        obj_vec.push (obj);
        Vect::make (&obj_vec)
    }
    pub struct Dict { pub obj_dic: ObjDic }

    impl_tag! (Dict, DICT_T);

    impl Obj for Dict {
        fn tag (&self) -> Tag { DICT_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Dict::cast (other);
                (obj_dic_eq (&self.obj_dic, &other.obj_dic))
            }
        }
    }
    impl Dict {
        fn make (obj_dic: &ObjDic) -> Ptr <Dict> {
            Ptr::new (Dict { obj_dic: obj_dic.clone () })
        }
    }
    pub fn dict_to_list_reverse (dict: Ptr <Obj>) -> Ptr <Obj> {
        let dict = Dict::cast (dict);
        let mut list = null ();
        let obj_dic = &dict.obj_dic;
        for kv in obj_dic.iter () {
            let sym = Sym::make (kv.0);
            let obj = kv.1;
            let pair = cons (sym, unit_list (obj.dup ()));
            list = cons (pair, list);
        }
        list
    }
    pub fn dict_to_list (dict: Ptr <Obj>) -> Ptr <Obj> {
        let dict = Dict::cast (dict);
        let list = dict_to_list_reverse (dict);
        list_reverse (list)
    }
    fn list_to_dict (mut list: Ptr <Obj>) -> Ptr <Dict> {
        let mut obj_dic = ObjDic::new ();
        while ! null_p (&list) {
            let pair = car (list.dup ());
            let key = car (pair.dup ());
            let rest = cdr (pair.dup ());
            let sym = Sym::cast (key);
            let name = &sym.sym;
            if cons_p (&rest) {
                let obj = car (rest);
                obj_dic.set (name, Some (obj));
            } else {
                obj_dic.set (name, None);
            }
            list = cdr (list);
        }
        Dict::make (&obj_dic)
    }
    fn dict_to_flat_list_reverse (dict: Ptr <Obj>) -> Ptr <Obj> {
        let dict = Dict::cast (dict);
        let mut list = null ();
        for kv in dict.obj_dic.iter () {
            let key = cons (
                Sym::make ("quote"),
                unit_list (Sym::make (kv.0)));
            let obj = kv.1.dup ();
            list = cons (obj, list);
            list = cons (key, list);
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
                let sym = Sym::cast (key);
                let name = sym.sym .as_str ();
                obj_dic.ins (name, Some (obj));
            }
            env.obj_stack.push (Dict::make (&obj_dic));
        }
    }
    fn dict_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        dict: Ptr <Dict>,
    ) -> Ptr <JoVec> {
        let sexp_list = dict_to_flat_list_reverse (dict);
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
                cons (Sym::make (mark_name),
                        unit_list (parse_sexp (token))),
            token::Token::Num { num, .. } => Num::make (*num),
            token::Token::Str { str, .. } => Str::make (str),
            token::Token::Sym { sym, .. } => Sym::make (sym),
        }
    }
    pub fn parse_sexp_list (token_vec: &token::TokenVec) -> Ptr <Obj> {
        let mut list = null ();
        token_vec
            .iter ()
            .rev ()
            .map (parse_sexp)
            .for_each (|obj| {
                list = cons (obj, list.dup ());
            });
        list
    }
    pub fn parse_sexp_vect (token_vec: &token::TokenVec) -> Ptr <Obj> {
        let obj_vec = token_vec
            .iter ()
            .map (parse_sexp)
            .collect::<ObjVec> ();
        Vect::make (&obj_vec)
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
                let new_last_sexp = cons (
                    head, cons (
                        last_sexp,
                        unit_list (next)));
                cons (
                    new_last_sexp,
                    sexp_list_prefix_assign (rest))
            }
            else
            {
                let rest = cdr (sexp_list);
                cons (
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
            let name = Sym::cast (name);
            let value = car (cdr (cdr (sexp.dup ())));
            obj_dic.ins (&name.sym, Some (value));
            sexp_list = cdr (sexp_list.dup ())
        }
        Dict::make (&obj_dic)
    }
    pub fn sexp_repr (env: &Env, sexp: Ptr <Obj>) -> String {
        if (null_p (&sexp)) {
            format! ("()")
        } else if (cons_p (&sexp)) {
            format! ("({})", sexp_list_repr (env, sexp))
        } else if (Vect::p (&sexp)) {
            let v = Vect::cast (sexp);
            let l = vect_to_list (v);
            format! ("[{}]", sexp_list_repr (env, l))
        } else if (Dict::p (&sexp)) {
            let d = Dict::cast (sexp);
            let l = dict_to_list (d);
            let v = list_to_vect (l);
            let obj_vec = v.obj_vec
                .iter ()
                .map (|x| cons (Sym::make ("="), x.dup ()))
                .collect ();
            let v = Vect::make (&obj_vec);
            let l = vect_to_list (v);
            format! ("{{{}}}", sexp_list_repr (env, l))
        } else if (Str::p (&sexp)) {
            let str = Str::cast (sexp);
            format! ("\"{}\"", str.str)
        } else if (Sym::p (&sexp)) {
            let sym = Sym::cast (sexp);
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
        if ! Sym::p (&sexp) {
            false
        } else {
            let sym = Sym::cast (sexp.dup ());
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

    impl_tag! (Keyword, KEYWORD_T);

    impl Obj for Keyword {
        fn tag (&self) -> Tag { KEYWORD_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Keyword::cast (other);
                (keyword_fn_eq (&self.fun, &other.fun))
            }
        }
    }
    impl Keyword {
        fn make (fun: KeywordFn) -> Ptr <Keyword> {
            Ptr::new (Keyword {
                fun,
            })
        }
    }
    fn find_keyword (
        env: &Env,
        name: &str,
    ) -> Option <Ptr <Keyword>> {
        if let Some (obj) = env.obj_dic.get (name) {
            if Keyword::p (obj) {
                let keyword = Keyword::cast (obj.dup ());
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
        if ! Sym::p (&head) {
            false
        } else {
            let sym = Sym::cast (head);
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
    impl Env {
        pub fn define_keyword (
            &mut self,
            name: &str,
            fun: KeywordFn,
        ) -> Id {
            self.define (name, Keyword::make (fun))
        }
    }
    struct Macro {
        obj: Ptr <Obj>,
    }

    impl_tag! (Macro, MACRO_T);

    impl Obj for Macro {
        fn tag (&self) -> Tag { MACRO_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Macro::cast (other);
                (obj_eq (&self.obj, &other.obj))
            }
        }
    }
    fn find_macro (
        env: &Env,
        name: &str,
    ) -> Option <Ptr <Macro>> {
        if let Some (obj) = env.obj_dic.get (name) {
            if Macro::p (obj) {
                let mac = Macro::cast (obj.dup ());
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
        if ! Sym::p (&head) {
            false
        } else {
            let sym = Sym::cast (head);
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
    impl Env {
        pub fn define_prim_macro (
            &mut self,
            name: &str,
            fun: PrimFn,
        ) -> Id {
            let arg_dic = Dic::from (vec! [ "body" ]);
            let obj = Ptr::new (Prim { arg_dic, fun });
            let mac = Ptr::new (Macro { obj });
            self.define (name, mac)
        }
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
              let sym = Sym::make (name);
              sym_compile (env, static_scope, sym)
          };
          let mut jo_vec = JoVec::new ();
          for name in iter {
              let jo = Ptr::new (DotJo { name: String::from (name) });
              jo_vec.push (jo);
          }
          jojo_append (&head_jojo, &jo_vec)
      }
      fn type_word_p (word: &str) -> bool {
          word.ends_with ("-t")
      }
      fn type_ref_compile (
          env: &mut Env,
          _: &StaticScope,
          name: &str,
      ) -> Ptr <JoVec> {
          if let Some (tag) = env.type_dic.get_index (name) {
              jojo! [ TypeRefJo { tag } ]
          } else {
              jojo! [
                  TypeRefJo { tag: env.type_dic.ins (name, None) }
              ]
          }
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
          } else if type_word_p (word) {
              type_ref_compile (env, static_scope, word)
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
              cons (cdr (car (sexp_list.dup ())),
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
              if ! Sym::p (&head) {
                  arity += 1;
              } else {
                  let sym = Sym::cast (head.dup ());
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
        if Str::p (&sexp) || Num::p (&sexp) {
            lit_compile (env, static_scope, sexp)
        } else if Sym::p (&sexp) {
            let sym = Sym::cast (sexp);
            sym_compile (env, static_scope, sym)
        } else if Vect::p (&sexp) {
            let vect = Vect::cast (sexp);
            vect_compile (env, static_scope, vect)
        } else if Dict::p (&sexp) {
            let dict = Dict::cast (sexp);
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
        if null_p (&sexp_list) {
            new_jojo ()
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

    impl_tag! (Module, MODULE_T);

    impl Obj for Module {
        fn tag (&self) -> Tag { MODULE_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Module::cast (other);
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

    impl_tag! (TopKeyword, TOP_KEYWORD_T);

    impl Obj for TopKeyword {
        fn tag (&self) -> Tag { TOP_KEYWORD_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = TopKeyword::cast (other);
                (top_keyword_fn_eq (&self.fun, &other.fun))
            }
        }
    }
    impl TopKeyword {
        fn make (fun: TopKeywordFn) -> Ptr <TopKeyword> {
            Ptr::new (TopKeyword {
                fun,
            })
        }
    }
    fn find_top_keyword (
        env: &Env,
        name: &str,
    ) -> Option <Ptr <TopKeyword>> {
        if let Some (obj) = env.obj_dic.get (name) {
            if TopKeyword::p (obj) {
                let top_keyword = TopKeyword::cast (obj.dup ());
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
        if ! Sym::p (&head) {
            false
        } else {
            let sym = Sym::cast (head);
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
            self.define (name, TopKeyword::make (fun))
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
          let vec = word.split ('.') .collect::<Vec <&str>> ();
          if vec.len () == 1 {
              String::new ()
          } else {
              assert! (vec.len () == 2);
              vec [0] .to_string ()
          }
      }
      fn name_of_word (word: &str) -> String {
          let vec = word.split ('.') .collect::<Vec <&str>> ();
          vec [vec.len () - 1] .to_string ()
      }
      fn assign_data_p (body: &Ptr <Obj>) -> bool {
          (cons_p (&body) &&
           Sym::p (&(car (body.dup ()))) &&
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
          let type_name = sym.sym.clone ();
          let data_name = name_t2c (&type_name);
          let rest = cdr (body);
          let data_body = cdr (car (rest));
          let name_vect = list_to_vect (data_body);
          let name_vec = name_vect_to_name_vec (name_vect);
          let tag = env.type_dic.len ();
          env.define_type (&type_name, Type::make (tag));
          env.define (&data_name, DataCons::make (tag, name_vec));
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
          cons (name, unit_list (
              cons (Sym::make ("lambda"),
                      cons (list_to_vect (arg_list),
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
          env.assign (&prefix, &name, obj);
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
      fn do_body_trans (body: Ptr <Obj>) -> Ptr <Obj> {
          if null_p (&body) {
              return body;
          }
          let sexp = car (body.dup ());
          let rest = cdr (body.dup ());
          if null_p (&rest) {
              return body;
          } else {
              let drop = unit_list (Sym::make ("drop"));
              let body = do_body_trans (rest);
              let body = cons (drop, body);
              let body = cons (sexp, body);
              return body;
          }
      }
      fn k_do (
          env: &mut Env,
          static_scope: &StaticScope,
          mut body: Ptr <Obj>,
      ) -> Ptr <JoVec> {
          body = sexp_list_prefix_assign (body.dup ());
          body = do_body_trans (body.dup ());
          sexp_list_compile (env, static_scope, body)
      }
      fn k_lambda (
          env: &mut Env,
          old_static_scope: &StaticScope,
          body: Ptr <Obj>,
      ) -> Ptr <JoVec> {
          let head = car (body.dup ());
          assert! (Vect::p (&head));
          let name_vect = Vect::cast (head);
          let name_vec = name_vect_to_name_vec (name_vect);
          let rest = cdr (body);
          let static_scope = static_scope_extend (
              old_static_scope, &name_vec);
          let jojo = sexp_compile (
              env, &static_scope, cons (Sym::make ("do"), rest));
          jojo! [
              LambdaJo  {
                  arg_dic: Ptr::new (Dic::from (name_vec)),
                  jojo,
              }
          ]
      }


      fn sexp_quote_compile (
          _env: &mut Env,
          sexp: Ptr <Obj>,
      ) -> Ptr <JoVec> {
          jojo! [
              LitJo { obj: sexp }
          ]
      }
      fn k_quote (
          env: &mut Env,
          _static_scope: &StaticScope,
          body: Ptr <Obj>,
      ) -> Ptr <JoVec> {
          assert! (cons_p (&body));
          assert! (null_p (&(cdr (body.dup ()))));
          let sexp = car (body);
          sexp_quote_compile (env, sexp)
      }
      type JojoMap = HashMap <Tag, Ptr <JoVec>>;
      struct CaseJo {
          jojo_map: JojoMap,
          default_jojo: Option <Ptr <JoVec>>,
      }

      impl Jo for CaseJo {
          fn exe (&self, env: &mut Env, scope: Ptr <Scope>) {
              let obj = env.obj_stack.pop () .unwrap ();
              let tag = obj.tag ();
              if let Some (jojo) = self.jojo_map.get (&tag) {
                  env.frame_stack.push (Box::new (Frame {
                      index: 0,
                      jojo: jojo.dup (),
                      scope: scope.dup (),
                  }));
              } else if let Some (ref jojo) = self.default_jojo {
                  env.frame_stack.push (Box::new (Frame {
                      index: 0,
                      jojo: jojo.dup (),
                      scope: scope.dup (),
                  }));
              } else {
                  eprintln! ("- CaseJo::exe");
                  eprintln! ("  tag mismatch");
                  eprintln! ("  tag : {}", name_of_tag (env, tag));
                  eprintln! ("  obj : {}", obj.repr (env));
                  panic! ("jojo fatal error!");
              }
          }
      }
      pub fn case_compile (
          env: &mut Env,
          static_scope: &StaticScope,
          mut body: Ptr <Obj>,
      ) -> Ptr <JoVec> {
          let mut jojo_map = JojoMap::new ();
          let mut default_jojo: Option <Ptr <JoVec>> = None;
          while ! null_p (&body) {
              let clause = car (body.dup ());
              let sym = car_as_sym (clause.dup ());
              let rest = cdr (clause);
              let type_name = &sym.sym;
              if (type_name == "_") {
                  let jojo = sexp_list_compile (env, static_scope, rest);
                  default_jojo = Some (jojo);
                  body = cdr (body);
              } else {
                  if let Some (typ) = env.find_type (type_name) {
                      let tag = typ.tag_of_type;
                      let jojo = sexp_list_compile (env, static_scope, rest);
                      jojo_map.insert (tag, jojo);
                      body = cdr (body);
                  } else {
                      eprintln! ("- case_compile");
                      eprintln! ("  unknown type_name : {}", type_name);
                      panic! ("jojo fatal error!");
                  }
              }
          }
          jojo! [
              CaseJo { jojo_map, default_jojo }
          ]
      }
      fn k_case (
          env: &mut Env,
          static_scope: &StaticScope,
          body: Ptr <Obj>,
      ) -> Ptr <JoVec> {
          let head = car (body.dup ());
          let rest = cdr (body);
          let head_jojo = sexp_compile (env, static_scope, head);
          let rest_jojo = case_compile (env, static_scope, rest);
          jojo_append (&head_jojo, &rest_jojo)
      }
      struct CollectListJo {
          counter: usize,
      }

      impl Jo for CollectListJo {
          fn exe (&self, env: &mut Env, _: Ptr <Scope>) {
              let mut list = null ();
              for _ in 0..self.counter {
                  let obj = env.obj_stack.pop () .unwrap ();
                  list = cons (obj, list)
              }
              env.obj_stack.push (list);
          }
      }
      fn k_list (
          env: &mut Env,
          static_scope: &StaticScope,
          body: Ptr <Obj>,
      ) -> Ptr <JoVec> {
          let sexp_list = body;
          let counter = list_size (sexp_list.dup ());
          let jojo = sexp_list_compile (
              env, static_scope, sexp_list);
          let ending_jojo = jojo! [
              CollectListJo { counter },
          ];
          jojo_append (&jojo, &ending_jojo)
      }
      fn k_note (
          _env: &mut Env,
          _static_scope: &StaticScope,
          body: Ptr <Obj>,
      ) -> Ptr <JoVec> {
          jojo! [
              LitJo { obj: cons (Sym::make ("note"), body) },
          ]
      }
      pub struct AssertJo {
          body: Ptr <Obj>,
          jojo: Ptr <JoVec>,
      }

      impl Jo for AssertJo {
          fn exe (&self, env: &mut Env, scope: Ptr <Scope>) {
              let base = env.frame_stack.len ();
              env.frame_stack.push (Box::new (Frame {
                  index: 0,
                  jojo: self.jojo.dup (),
                  scope: scope.dup (),
              }));
              env.run_with_base (base);
              let result = env.obj_stack.pop () .unwrap ();
              if true_p (&result) {
                  return;
              } else {
                  // env.frame_stack_report ();
                  // env.obj_stack_report ();
                  eprintln! ("- assert fail : ");
                  eprintln! ("  {} : ", sexp_list_repr (env, self.body.dup ()));
                  panic! ("jojo fatal error!");
              }
          }
      }
      fn k_assert (
          env: &mut Env,
          static_scope: &StaticScope,
          body: Ptr <Obj>,
      ) -> Ptr <JoVec> {
          let jojo = sexp_list_compile (env, &static_scope, body.dup ());
          jojo! [
              AssertJo { body, jojo }
          ]
      }
      struct IfJo {
          pred_jojo: Ptr <JoVec>,
          then_jojo: Ptr <JoVec>,
          else_jojo: Ptr <JoVec>,
      }

      impl Jo for IfJo {
          fn exe (&self, env: &mut Env, scope: Ptr <Scope>) {
              let result = jojo_eval (env, &scope, self.pred_jojo.dup ());
              if true_p (&result) {
                  env.frame_stack.push (Box::new (Frame {
                      index: 0,
                      jojo: self.then_jojo.dup (),
                      scope,
                  }));
              } else if false_p (&result) {
                  env.frame_stack.push (Box::new (Frame {
                      index: 0,
                      jojo: self.else_jojo.dup (),
                      scope,
                  }));
              } else {
                  eprintln! ("- IfJo::exe");
                  eprintln! ("  pred_jojo run to non bool value");
                  panic! ("jojo fatal error!");
              }
          }
      }
      fn k_if (
          env: &mut Env,
          static_scope: &StaticScope,
          body: Ptr <Obj>,
      ) -> Ptr <JoVec> {
          let size = list_size (body.dup ());
          assert! (size == 3);
          let pred_sexp = car (body.dup ());
          let then_sexp = car (cdr (body.dup ()));
          let else_sexp = car (cdr (cdr (body)));
          let pred_jojo = sexp_compile (env, static_scope, pred_sexp);
          let then_jojo = sexp_compile (env, static_scope, then_sexp);
          let else_jojo = sexp_compile (env, static_scope, else_sexp);
          jojo! [
              IfJo {
                  pred_jojo,
                  then_jojo,
                  else_jojo,
              }
          ]
      }
      struct WhenJo {
          pred_jojo: Ptr <JoVec>,
          then_jojo: Ptr <JoVec>,
      }

      impl Jo for WhenJo {
          fn exe (&self, env: &mut Env, scope: Ptr <Scope>) {
              let result = jojo_eval (env, &scope, self.pred_jojo.dup ());
              if true_p (&result) {
                  env.frame_stack.push (Box::new (Frame {
                      index: 0,
                      jojo: self.then_jojo.dup (),
                      scope,
                  }));
              } else if false_p (&result) {
                  env.obj_stack.push (result);
              } else {
                  eprintln! ("- WhenJo::exe");
                  eprintln! ("  pred_jojo run to non bool value");
                  panic! ("jojo fatal error!");
              }
          }
      }
      fn k_when (
          env: &mut Env,
          static_scope: &StaticScope,
          body: Ptr <Obj>,
      ) -> Ptr <JoVec> {
          let size = list_size (body.dup ());
          assert! (size == 2);
          let pred_sexp = car (body.dup ());
          let then_sexp = car (cdr (body.dup ()));
          let pred_jojo = sexp_compile (env, static_scope, pred_sexp);
          let then_jojo = sexp_compile (env, static_scope, then_sexp);
          jojo! [
              WhenJo {
                  pred_jojo,
                  then_jojo,
              }
          ]
      }
      fn m_let (
          env: &mut Env,
          arg: &ObjDic,
      ) {
          let body = arg_idx (arg, 0);
          let head = car (body.dup ());
          let rest = cdr (body);
          let binding_vect = vect_reverse (head);
          let mut sexp = cons (Sym::make ("do"), rest);
          for binding in &binding_vect.obj_vec {
              let name = car (binding.dup ());
              let obj = car (cdr (binding.dup ()));
              sexp = cons (
                  cons (Sym::make ("lambda"),
                        cons (unit_vect (name),
                              unit_list (sexp))),
                  unit_list (obj));
          }
          println! ("sexp : {}", sexp_repr (env, sexp.dup ()));
          env.obj_stack.push (sexp);
      }
      fn sexp_quote_and_unquote (
          env: &Env,
          sexp: Ptr <Obj>,
      ) -> Ptr <Obj> {
          if Str::p (&sexp) || Num::p (&sexp) {
              sexp
          } else if Sym::p (&sexp) {
              cons (Sym::make ("quote"),
                    unit_list (sexp))
          } else if null_p (&sexp) {
              cons (Sym::make ("quote"),
                    unit_list (sexp))
          } else if Vect::p (&sexp) {
              let list = vect_to_list (sexp);
              cons (Sym::make ("list-to-vect"),
                    unit_list (sexp_list_quote_and_unquote (env, list)))
          } else if Dict::p (&sexp) {
              let list = dict_to_list (sexp);
              cons (Sym::make ("list-to-dict"),
                    unit_list (sexp_list_quote_and_unquote (env, list)))
          } else {
              assert! (cons_p (&sexp));
              let head = car (sexp.dup ());
              if sym_sexp_as_str_p (&head, "unquote") {
                  let rest = cdr (sexp.dup ());
                  assert! (cons_p (&rest));
                  assert! (null_p (&(cdr (rest.dup ()))));
                  car (rest)
              } else {
                  sexp_list_quote_and_unquote (
                      env,
                      sexp)
              }
          }
      }
      fn sexp_list_quote_and_unquote (
          env: &Env,
          sexp_list: Ptr <Obj>,
      ) -> Ptr <Obj> {
          if null_p (&sexp_list) {
              unit_list (Sym::make ("*"))
          } else {
              assert! (cons_p (&sexp_list));
              let mut sexp = car (sexp_list.dup ());
              if cons_p (&sexp)
                  && sym_sexp_as_str_p (&(car (sexp.dup ())),
                                        "unquote-splicing")
              {
                  let rest = cdr (sexp);
                  assert! (cons_p (&rest));
                  assert! (null_p (&(cdr (rest.dup ()))));
                  sexp = car (rest);
              } else {
                  sexp = cons (
                      Sym::make ("*"),
                      unit_list (sexp_quote_and_unquote (env, sexp)));
              }
              cons (Sym::make ("list-append"),
                    cons (
                        sexp,
                        unit_list (
                            sexp_list_quote_and_unquote (
                                env, cdr (sexp_list)))))
          }
      }
      fn m_quasiquote (
          env: &mut Env,
          arg: &ObjDic,
      ) {
          let body = arg_idx (arg, 0);
          assert! (cons_p (&body));
          assert! (null_p (&(cdr (body.dup ()))));
          let sexp = car (body);
          let new_sexp = sexp_quote_and_unquote (env, sexp);
          env.obj_stack.push (new_sexp);
      }
      fn sexp_list_and (
          env: &mut Env,
          sexp_list: Ptr <Obj>,
      ) -> Ptr <Obj> {
          if null_p (&sexp_list) {
              Sym::make ("true")
          } else if null_p (&(cdr (sexp_list.dup ()))) {
              car (sexp_list)
          } else {
              let head = car (sexp_list.dup ());
              let rest = cdr (sexp_list);
              cons (
                  Sym::make ("if"),
                  cons (
                      cons (Sym::make ("not"), unit_list (head)),
                      cons (
                          Sym::make ("false"),
                          unit_list (sexp_list_and (env, rest)))))
          }
      }
      fn m_and (
          env: &mut Env,
          arg: &ObjDic,
      ) {
          let body = arg_idx (arg, 0);
          let sexp = sexp_list_and (env, body);
          env.obj_stack.push (sexp);
      }
      fn sexp_list_or (
          env: &mut Env,
          sexp_list: Ptr <Obj>,
      ) -> Ptr <Obj> {
          if null_p (&sexp_list) {
              Sym::make ("false")
          } else if null_p (&(cdr (sexp_list.dup ()))) {
              car (sexp_list)
          } else {
              let head = car (sexp_list.dup ());
              let rest = cdr (sexp_list);
              cons (
                  Sym::make ("if"),
                  cons (
                      head,
                      cons (
                          Sym::make ("true"),
                          unit_list (sexp_list_or (env, rest)))))
          }
      }
      fn m_or (
          env: &mut Env,
          arg: &ObjDic,
      ) {
          let body = arg_idx (arg, 0);
          let sexp = sexp_list_or (env, body);
          env.obj_stack.push (sexp);
      }
      fn vect_list_cond (
          env: &mut Env,
          vect_list: Ptr <Obj>,
      ) -> Ptr <Obj> {
          assert! (! null_p (&vect_list));
          let head = car (vect_list.dup ());
          let rest = cdr (vect_list);
          let list = vect_to_list (head);
          let question = car (list.dup ());
          let answer = cons (Sym::make ("do"), cdr (list));
          if (null_p (&rest)) {
              if (sym_sexp_as_str_p (&question, "else")) {
                  answer
              } else {
                  let result = null ();
                  let result = cons (answer, result);
                  let result = cons (question, result);
                  let result = cons (Sym::make ("when"), result);
                  result
              }
          } else {
              let result = unit_list (vect_list_cond (env, rest));
              let result = cons (answer, result);
              let result = cons (question, result);
              let result = cons (Sym::make ("if"), result);
              result
          }
      }
      fn m_cond (
          env: &mut Env,
          arg: &ObjDic,
      ) {
          let body = arg_idx (arg, 0);
          let sexp = vect_list_cond (env, body);
          env.obj_stack.push (sexp);
      }
    fn arg_idx (arg_dic: &ObjDic, index: usize) -> Ptr <Obj> {
        let entry = arg_dic.idx (index);
        if let Some (value) = &entry.value {
            value.dup ()
        } else {
            eprintln! ("- arg_idx");
            eprintln! ("  unknown index : {}", index);
            panic! ("jojo fatal error!");
        }
    }
    fn expose_type (env: &mut Env) {
        env.define_prim ("type-of", vec! ["obj"], |env, arg| {
            env.obj_stack.push (type_of (env, arg_idx (arg, 0)));
        });
    }
    fn expose_bool (env: &mut Env) {
        env.define ("true", True::make ());
        env.define ("false", False::make ());;
        define_prim! (env, "not", ["x"], not);;
    }
    fn expose_num (env: &mut Env) {
        define_prim! (env, "inc", ["x"], inc);
        define_prim! (env, "dec", ["x"], dec);
        define_prim! (env, "add", ["x", "y"], add);
        define_prim! (env, "sub", ["x", "y"], sub);
        define_prim! (env, "mul", ["x", "y"], mul);
        define_prim! (env, "div", ["x", "y"], div);
    }
    fn expose_str (env: &mut Env) {
        define_prim! (env, "str-length", ["str"], str_length);
        define_prim! (env, "str-append", ["ante", "succ"], str_append);
        define_prim! (env, "str-slice", ["str", "begin", "end"], str_slice);
        define_prim! (env, "str-ref", ["str", "index"], str_ref);
        define_prim! (env, "str-head", ["str"], str_head);
        define_prim! (env, "str-rest", ["str"], str_rest);
    }
    fn expose_sym (env: &mut Env) {
        define_prim! (env, "sym-length", ["sym"], sym_length);
        define_prim! (env, "sym-append", ["ante", "succ"], sym_append);
        define_prim! (env, "sym-slice", ["sym", "begin", "end"], sym_slice);
        define_prim! (env, "sym-ref", ["sym", "index"], sym_ref);
        define_prim! (env, "sym-head", ["sym"], sym_head);
        define_prim! (env, "sym-rest", ["sym"], sym_rest);
    }
    fn expose_list (env: &mut Env) {
        env.define ("null", null ());
        define_prim! (env, "cons", ["car", "cdr"], cons);
        define_prim! (env, "car", ["pair"], car);
        define_prim! (env, "cdr", ["pair"], cdr);
        define_prim! (env, "list-length", ["list"], list_length);
        define_prim! (env, "list-reverse", ["list"], list_reverse);
        define_prim! (env, "list-reverse-append", ["ante", "succ"], list_reverse_append);
        define_prim! (env, "list-append", ["ante", "succ"], list_append);
        define_prim! (env, "unit-list", ["obj"], unit_list);
    }
    fn expose_vect (env: &mut Env) {
        define_prim! (env, "list-to-vect", ["list"], list_to_vect);
        define_prim! (env, "vect-to-list", ["vect"], vect_to_list);
        define_prim! (env, "vect-length", ["vect"], vect_length);
        define_prim! (env, "vect-append", ["ante", "succ"], vect_append);
        define_prim! (env, "vect-slice", ["vect", "begin", "end"], vect_slice);
        define_prim! (env, "vect-ref", ["vect", "index"], vect_ref);
        define_prim! (env, "vect-head", ["vect"], vect_head);
        define_prim! (env, "vect-rest", ["vect"], vect_rest);
        define_prim! (env, "vect-reverse", ["vect"], vect_reverse);
        define_prim! (env, "unit-vect", ["obj"], unit_vect);
    }
    fn expose_option (env: &mut Env) {
        env.define ("none", JNone::make ());
        define_prim! (env, "some", ["value"], some);
    }
    fn expose_stack (env: &mut Env) {
        env.define_prim ("drop", vec! [], |env, _| {
            env.obj_stack.pop ();
        });
    }
    fn expose_syntax (env: &mut Env) {
        env.define_top_keyword ("=", tk_assign);
        env.define_keyword ("do", k_do);
        env.define_keyword ("lambda", k_lambda);
        // env.define_keyword ("macro", k_macro);
        env.define_keyword ("case", k_case);
        env.define_keyword ("quote", k_quote);
        env.define_keyword ("*", k_list);
        env.define_keyword ("note", k_note);
        env.define_keyword ("assert", k_assert);
        env.define_keyword ("if", k_if);
        env.define_keyword ("when", k_when);
        env.define_prim_macro ("let", m_let);
        env.define_prim_macro ("quasiquote", m_quasiquote);
        env.define_prim_macro ("and", m_and);
        env.define_prim_macro ("or", m_or);
        env.define_prim_macro ("cond", m_cond);
    }
    fn expose_misc (env: &mut Env) {
        env.define_prim ("repr", vec! ["obj"], |env, arg| {
            let obj = arg_idx (arg, 0);
            obj.print (env);
            env.obj_stack.push (obj);
        });
        env.define_prim ("print", vec! ["obj"], |env, arg| {
            let obj = arg_idx (arg, 0);
            obj.print (env);
            env.obj_stack.push (obj);
        });
        env.define_prim ("println", vec! ["obj"], |env, arg| {
            let obj = arg_idx (arg, 0);
            obj.print (env);
            println! ("");
            env.obj_stack.push (obj);
        });
        env.define_prim ("eq", vec! ["lhs", "rhs"], |env, arg| {
            let lhs = arg_idx (arg, 0);
            let rhs = arg_idx (arg, 1);
            env.obj_stack.push (make_bool (obj_eq (&lhs, &rhs)));
        });
    }
    fn expose_core (env: &mut Env) {
        expose_type (env);
        expose_bool (env);
        expose_num (env);
        expose_str (env);
        expose_sym (env);
        expose_list (env);
        expose_vect (env);
        expose_option (env);
        // expose_dict (env);
        // expose_sexp (env);
        // expose_top_keyword (env);
        // expose_keyword (env);
        // expose_system (env);
        // expose_module (env);
        expose_syntax (env);
        expose_stack (env);
        expose_misc (env);
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
            "bye", Str::make ("bye"));
        let world = env.define (
            "world", Str::make ("world"));

        env.frame_stack.push (frame! [
            RefJo { id: world },
            RefJo { id: bye },
            RefJo { id: world },
        ]);

        env.run ();

        assert_eq! (3, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("bye"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_apply () {
        let mut env = Env::new ();

        let bye = env.define (
            "bye", Str::make ("bye"));
        let world = env.define (
            "world", Str::make ("world"));

        env.frame_stack.push (frame! [
            RefJo { id: bye },
            RefJo { id: world },
            LambdaJo { arg_dic: Ptr::new (Dic::from (vec! [ "x", "y" ])),
                       jojo: jojo! [
                           LocalRefJo { level: 0, index: 1 },
                           LocalRefJo { level: 0, index: 0 },
                       ] },
            ApplyJo { arity: 2 },
        ]);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("bye"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (0, env.obj_stack.len ());

        // curry

        env.frame_stack.push (frame! [
            RefJo { id: bye },
            RefJo { id: world },
            LambdaJo { arg_dic: Ptr::new (Dic::from (vec! [ "x", "y" ])),
                       jojo: jojo! [
                           LocalRefJo { level: 0, index: 1 },
                           LocalRefJo { level: 0, index: 0 },
                       ] },
            ApplyJo { arity: 1 },
            ApplyJo { arity: 1 },
        ]);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("bye"));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_data () {
        let mut env = Env::new ();

        let last_cry = env.define (
            "last-cry",
            cons (Str::make ("bye"),
                    Str::make ("world")));

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
                    cons (Str::make ("bye"),
                          Str::make ("world")));
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("bye"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_data_cons () {
        let mut env = Env::new ();

        let bye = env.define (
            "bye", Str::make ("bye"));
        let world = env.define (
            "world", Str::make ("world"));
        let cons = env.define (
            "cons-c", DataCons::make (CONS_T, vec! [
                String::from ("car"),
                String::from ("cdr"),
            ]));

        env.frame_stack.push (frame! [
            RefJo { id: bye },
            RefJo { id: world },
            RefJo { id: cons },
            ApplyJo { arity: 2 },
            DotJo { name: String::from ("car") },
        ]);

        env.run ();
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("bye"));
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
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_prim () {
        let mut env = Env::new ();

        let bye = env.define (
            "bye", Str::make ("bye"));
        let world = env.define (
            "world", Str::make ("world"));
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
        assert_pop (&mut env, Str::make ("bye"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
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
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("bye"));
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
