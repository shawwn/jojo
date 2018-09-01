    // use std::rc::Rc;
    use std::sync::Arc;
    use dic::Dic;
    use scan;
    // pub type Ptr <T> = Rc <T>;
    pub type Ptr <T> = Arc <T>;

    pub type Name = String;

    pub type Id = usize; // index in to ObjDic
    pub type ObjDic = Dic <Ptr <Obj>>;

    pub type Tag = usize; // index in to TypeDic
    pub type TypeDic = Dic <Ptr <Type>>;

    pub type ObjStack = Vec <Ptr <Obj>>;
    pub type FrameStack = Vec <Box <Frame>>;

    pub type LocalScope = Vec <ObjDic>; // index from end

    pub type TagVec = Vec <Tag>;
    pub type JoVec = Vec <Ptr <Jo>>;
    pub type ObjVec = Vec <Ptr <Obj>>;
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
      pub fn local_scope_extend (
          local_scope: &LocalScope,
          obj_dic: ObjDic,
      ) -> Ptr <LocalScope> {
          let mut obj_dic_vec = local_scope.clone ();
          obj_dic_vec.push (obj_dic);
          Ptr::new (obj_dic_vec)
      }
      pub fn local_scope_eq (
          lhs: &LocalScope,
          rhs: &LocalScope,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter () .zip (rhs.iter ())
           .all (|p| obj_dic_eq (p.0, p.1)))
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
      impl Dup for Ptr <LocalScope> {
          fn dup (&self) -> Self {
              Ptr::clone (self)
          }
      }
      impl Dup for Ptr <JoVec> {
          fn dup (&self) -> Self {
              Ptr::clone (self)
          }
      }
      pub trait Make <T> {
          fn make (T) -> Ptr <Self>;
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
        fn exe (&self, env: &mut Env, local_scope: Ptr <LocalScope>);

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
        fn exe (&self, env: &mut Env, _: Ptr <LocalScope>) {
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
        fn exe (&self, env: &mut Env, local_scope: Ptr <LocalScope>) {
            let obj_dic = vec_peek (&local_scope, self.level);
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
        fn exe (&self, env: &mut Env, _: Ptr <LocalScope>) {
            let obj = env.obj_stack.pop () .unwrap ();
            obj.apply (env, self.arity);
        }
    }
    pub struct DotJo {
        name: String,
    }

    impl Jo for DotJo {
        fn exe (&self, env: &mut Env, _: Ptr <LocalScope>) {
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
        fn exe (&self, env: &mut Env, local_scope: Ptr <LocalScope>) {
            env.obj_stack.push (Ptr::new (Closure {
                arg_dic: self.arg_dic.clone (),
                jojo: self.jojo.dup (),
                local_scope: local_scope.dup (),
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
                    let local_scope = frame.local_scope.dup ();
                    self.frame_stack.push (frame);
                    jo.exe (self, local_scope);
                } else {
                    jo.exe (self, frame.local_scope);
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
        pub local_scope: Ptr <LocalScope>,
    }
    impl Frame {
        pub fn make (jo_vec: JoVec) -> Box <Frame> {
            Box::new (Frame {
                index: 0,
                jojo: Ptr::new (jo_vec),
                local_scope: Ptr::new (LocalScope::new ()),
            })
        }
    }
    pub struct Type {
        method_dic: ObjDic,
        tag_of_type: Tag,
        super_tag_vec: TagVec,
    }
    impl Make <Tag> for Type {
        fn make (tag: Tag) -> Ptr <Type> {
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
        local_scope: Ptr <LocalScope>,
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
                 local_scope_eq (&self.local_scope, &other.local_scope) &&
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
                    local_scope: local_scope_extend (
                        &self.local_scope, arg_dic),
                }));
            } else {
                env.obj_stack.push (Ptr::new (Closure {
                    arg_dic,
                    jojo: self.jojo.dup (),
                    local_scope: self.local_scope.dup (),
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
    impl <'a> Make <&'a str> for Str {
        fn make (str: &'a str) -> Ptr <Str> {
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
    impl <'a> Make <&'a str> for Sym {
        fn make (str: &'a str) -> Ptr <Sym> {
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
    impl Make <f64> for Num {
        fn make (num: f64) -> Ptr <Num> {
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
    }
    pub fn null_c () -> Ptr <Data> {
       Data::unit (NULL_T)
    }
    pub fn cons_c (car: Ptr <Obj>, cdr: Ptr <Obj>) -> Ptr <Data> {
        Data::make (CONS_T, vec! [
            ("car", car),
            ("cdr", cdr),
        ])
    }
    pub fn car (cons: Ptr <Obj>) -> Ptr <Obj> {
        assert_eq! (CONS_T, cons.tag ());
        cons.get ("car") .unwrap ()
    }
    pub fn cdr (cons: Ptr <Obj>) -> Ptr <Obj> {
        assert_eq! (CONS_T, cons.tag ());
        cons.get ("cdr") .unwrap ()
    }
    pub fn list_p (x: Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (NULL_T == tag ||
         CONS_T == tag)
    }
    pub fn unit_list (obj: Ptr <Obj>) -> Ptr <Obj> {
        cons_c (obj, null_c ())
    }
    pub fn nothing_c () -> Ptr <Data> {
       Data::unit (NOTHING_T)
    }
    pub fn just_c (value: Ptr <Obj>) -> Ptr <Data> {
        Data::make (JUST_T, vec! [
            ("value", value),
        ])
    }
    pub fn value_of_just (just: Ptr <Obj>) -> Ptr <Obj> {
        assert_eq! (JUST_T, just.tag ());
        just.get ("value") .unwrap ()
    }
    pub fn maybe_p (x: Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (NOTHING_T == tag ||
         JUST_T == tag)
    }
    pub struct Vect { pub obj_vec: ObjVec }
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
    pub struct Dict { pub obj_dic: ObjDic }
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
    // fn scan_word_list (code: Ptr <Str>) -> Ptr <Obj> {
    //     scan::scan_word_vec (code.str)
    //         .filter ()
    //         .map ()
    // }
    // fn parse_sexp () -> Ptr <Obj> {
    //
    // }
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
            RefJo { id: world }
        ]);

        env.run ();

        assert_eq! (3, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::make ("world")));
        assert_eq! (2, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::make ("bye")));
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::make ("world")));
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
            Str::make ("bye")));
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::make ("world")));
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
            Str::make ("world")));
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::make ("bye")));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_data () {
        let mut env = Env::new ();

        let last_cry = env.define (
            "last-cry",
            cons_c (Str::make ("bye"),
                    Str::make ("world")));

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
            cons_c (Str::make ("bye"),
                    Str::make ("world"))));
        assert_eq! (2, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::make ("bye")));
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::make ("world")));
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
            Str::make ("bye")));
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
            Str::make ("world")));
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
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::make ("bye")));
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::make ("world")));
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
            Str::make ("world")));
        assert_eq! (1, env.obj_stack.len ());
        assert! (obj_eq (
            env.obj_stack.pop () .unwrap (),
            Str::make ("bye")));
        assert_eq! (0, env.obj_stack.len ());
    }
