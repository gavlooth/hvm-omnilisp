#pragma once

// OmniLisp Nick-encoded names
// Constructor names are encoded as base64-like 24-bit values
// Each character maps to 0-63, allowing 4 characters max

// hvm4.c is already included by main.c before this file
// #include "../../../hvm4/clang/hvm4.c"

// =============================================================================
// Nick Encoding Utilities
// =============================================================================

fn u32 omni_nick(const char *name) {
  u32 k = 0;
  for (u32 i = 0; name[i] != '\0' && i < 4; i++) {
    k = ((k << 6) + nick_letter_to_b64(name[i])) & EXT_MASK;
  }
  return k;
}

// =============================================================================
// OmniLisp AST Constructor Names
// =============================================================================

static int OMNI_NAMES_READY = 0;

// Core expressions
static u32 OMNI_NAM_LIT;   // Literal (integer/float wrapped)
static u32 OMNI_NAM_SYM;   // Symbol
static u32 OMNI_NAM_VAR;   // Variable (de Bruijn index)
static u32 OMNI_NAM_LAM;   // Lambda
static u32 OMNI_NAM_LAMR;  // Recursive lambda
static u32 OMNI_NAM_APP;   // Application
static u32 OMNI_NAM_LET;   // Let binding (lazy by default)
static u32 OMNI_NAM_LETS;  // Strict let binding (^:strict)
static u32 OMNI_NAM_LETP;  // Parallel let binding (^:parallel - force parallel)
static u32 OMNI_NAM_IF;    // Conditional
static u32 OMNI_NAM_DO;    // Sequencing

// Arithmetic operations
static u32 OMNI_NAM_ADD;   // +
static u32 OMNI_NAM_SUB;   // -
static u32 OMNI_NAM_MUL;   // *
static u32 OMNI_NAM_DIV;   // /
static u32 OMNI_NAM_MOD;   // mod
static u32 OMNI_NAM_EQL;   // =
static u32 OMNI_NAM_NEQ;   // !=
static u32 OMNI_NAM_LT;    // <
static u32 OMNI_NAM_GT;    // >
static u32 OMNI_NAM_LE;    // <=
static u32 OMNI_NAM_GE;    // >=
static u32 OMNI_NAM_AND;   // and
static u32 OMNI_NAM_OR;    // or
static u32 OMNI_NAM_NOT;   // not

// Type predicates
static u32 OMNI_NAM_INTP;  // int?
static u32 OMNI_NAM_LSTP;  // list?
static u32 OMNI_NAM_NILP;  // nil?
static u32 OMNI_NAM_NUMP;  // number?

// Data structures
static u32 OMNI_NAM_CON;   // Cons cell
static u32 OMNI_NAM_NIL;   // Empty list
static u32 OMNI_NAM_CHR;   // Character
static u32 OMNI_NAM_ARR;   // Array
static u32 OMNI_NAM_DICT;  // Dictionary
static u32 OMNI_NAM_FST;   // First
static u32 OMNI_NAM_SND;   // Second

// Numeric types (HVM4 representation)
static u32 OMNI_NAM_CST;   // Constant (wrapped integer)
static u32 OMNI_NAM_FIX;   // Fixed-point: #Fix{hi, lo, scale}

// Pattern matching
static u32 OMNI_NAM_MAT;   // Match expression
static u32 OMNI_NAM_MATS;  // Speculative match (^:speculate) - evaluates all branches in parallel
static u32 OMNI_NAM_NMAT;  // Native match
static u32 OMNI_NAM_CASE;  // Case clause: #Case{pattern, guard, body}
static u32 OMNI_NAM_PCTR;  // Constructor pattern: #PCtr{tag, args}
static u32 OMNI_NAM_PLIT;  // Literal pattern: #PLit{value}
static u32 OMNI_NAM_PWLD;  // Wildcard pattern: #PWld
static u32 OMNI_NAM_PVAR;  // Variable pattern: #PVar{index}
static u32 OMNI_NAM_PAS;   // As pattern: #PAs{name, pattern}
static u32 OMNI_NAM_PRST;  // Rest pattern: #PRst{name}
static u32 OMNI_NAM_PGRD;  // Guard: #PGrd{pattern, condition}
static u32 OMNI_NAM_GUAR;  // Guard expression: #Guar{condition}
static u32 OMNI_NAM_SPRD;  // Spread pattern: #Sprd{name}
static u32 OMNI_NAM_DLET;  // Destructuring let: #DLet{pattern, value, body}
static u32 OMNI_NAM_PARR;  // Array pattern: #PArr{elements}
static u32 OMNI_NAM_DLAM;  // Destructuring lambda: #DLam{pattern, body}
static u32 OMNI_NAM_POR;   // Or pattern: #POr{patterns} - (or pat1 pat2 ...)
static u32 OMNI_NAM_PLST;  // List pattern: #PLst{elements} - (h .. t) or (a b c)

// Named let (Scheme-style loop)
static u32 OMNI_NAM_NLET;  // Named let (parallel): #NLet{name, init_args, body}
                           // Default: bindings evaluate in parallel (HVM4 native)
static u32 OMNI_NAM_NLETS; // Named let (sequential): #NLetS{name, init_args, body}
                           // With ^:seq: bindings evaluate left-to-right

// Pipe and function utilities
static u32 OMNI_NAM_PIPE;  // Pipe: #Pipe{value, fns} (NOTE: |> is desugared at parse time)
static u32 OMNI_NAM_CURY;  // Curry: #Cury{fn, arity} - convert multi-arg to curried form
static u32 OMNI_NAM_FLIP;  // Flip: #Flip{fn} - swap first two arguments
static u32 OMNI_NAM_ROTR;  // Rotate: #Rotr{fn} - cycle args left (first to end)
static u32 OMNI_NAM_COMP;  // Compose: #Comp{fns}
static u32 OMNI_NAM_APPL;  // Apply: #Appl{fn, args}

// Conditional forms
static u32 OMNI_NAM_WHEN;  // When: #When{cond, body}
static u32 OMNI_NAM_UNLS;  // Unless: #Unls{cond, body}
static u32 OMNI_NAM_COND;  // Cond: #Cond{clauses}
static u32 OMNI_NAM_CCLS;  // Cond clause: #CCls{test, body}

// Path access (functional)
static u32 OMNI_NAM_GET;   // Get: #Get{coll, key}
static u32 OMNI_NAM_PUT;   // Put: #Put{coll, key, val} - functional update
static u32 OMNI_NAM_UPDT;  // Update: #Updt{coll, key, fn}
static u32 OMNI_NAM_GTIN;  // GetIn: #GtIn{coll, path}
static u32 OMNI_NAM_ASIN;  // AssocIn: #AsIn{coll, path, val}
static u32 OMNI_NAM_UPIN;  // UpdateIn: #UpIn{coll, path, fn}

// Mutation primitives (imperative, side-effecting)
static u32 OMNI_NAM_SETB;  // set!: #SetB{var, val} - mutate variable binding
static u32 OMNI_NAM_PUTB;  // put!: #PutB{coll, key, val} - mutate collection in place
static u32 OMNI_NAM_UPDTB; // update!: #UpdtB{coll, key, fn} - mutate via function

// Iterator Lazy Wrappers (internal HVM4 nodes)
// These represent SUSPENDED lazy computation over iterators.
// User-facing functions like (map f iter) dispatch here when the collection
// is an Iterator type. For List/Array, dispatch goes to eager implementations.
// See "Collection operations" section for generic dispatch nodes.
static u32 OMNI_NAM_ITER;  // Iterator: #Iter{state, next_fn}
static u32 OMNI_NAM_RANG;  // Range: #Rang{start, end, step}
static u32 OMNI_NAM_IMAP;  // Lazy map wrapper: #IMap{iter, fn}
static u32 OMNI_NAM_IFLT;  // Lazy filter wrapper: #IFlt{iter, pred}
static u32 OMNI_NAM_ITKN;  // Lazy take wrapper: #ITkn{iter, n}
static u32 OMNI_NAM_IDRP;  // Lazy drop wrapper: #IDrp{iter, n}
static u32 OMNI_NAM_DONE;  // Iterator done marker: #Done
static u32 OMNI_NAM_IZIP;  // Lazy zip wrapper: #IZip{iters}
static u32 OMNI_NAM_ICHN;  // Lazy chain wrapper: #IChn{iters}
static u32 OMNI_NAM_IENM;  // Lazy enumerate wrapper: #IEnm{iter}
static u32 OMNI_NAM_ITKW;  // Lazy take-while wrapper: #ITkW{iter, pred}
static u32 OMNI_NAM_IDRW;  // Lazy drop-while wrapper: #IDrW{iter, pred}
static u32 OMNI_NAM_IFLD;  // Iterator fold (consumes): #IFld{iter, init, fn}
static u32 OMNI_NAM_IFND;  // Iterator find (consumes): #IFnd{iter, pred}
static u32 OMNI_NAM_IANY;  // Iterator any? (consumes): #IAny{iter, pred}
static u32 OMNI_NAM_IALL;  // Iterator all? (consumes): #IAll{iter, pred}
static u32 OMNI_NAM_INTH;  // Iterator nth (consumes): #INth{iter, n}
static u32 OMNI_NAM_IFMP;  // Lazy flat-map wrapper: #IFMp{iter, fn}
static u32 OMNI_NAM_ISTP;  // Lazy step-by wrapper: #IStp{iter, n}
static u32 OMNI_NAM_ICHK;  // Lazy chunks wrapper: #IChk{iter, n}
static u32 OMNI_NAM_IWIN;  // Lazy windows wrapper: #IWin{iter, n}

// Math functions (FFI-backed)
static u32 OMNI_NAM_SQRT;  // sqrt: #Sqrt{x}
static u32 OMNI_NAM_POW;   // pow: #Pow{base, exp}
static u32 OMNI_NAM_MEXP;  // exp: #MExp{x}
static u32 OMNI_NAM_MLOG;  // log: #MLog{x}
static u32 OMNI_NAM_MSIN;  // sin: #MSin{x}
static u32 OMNI_NAM_MCOS;  // cos: #MCos{x}
static u32 OMNI_NAM_MTAN;  // tan: #MTan{x}
static u32 OMNI_NAM_MASN;  // asin: #MAsn{x}
static u32 OMNI_NAM_MACS;  // acos: #MAcs{x}
static u32 OMNI_NAM_MATN;  // atan: #MAtn{x}
static u32 OMNI_NAM_MABS;  // abs: #MAbs{x}
static u32 OMNI_NAM_FLOR;  // floor: #Flor{x}
static u32 OMNI_NAM_MCEI;  // ceil: #MCei{x}
static u32 OMNI_NAM_ROND;  // round: #Rond{x}
static u32 OMNI_NAM_RAND;  // random: #Rand{}

// I/O operations (FFI-backed)
static u32 OMNI_NAM_RDFL;  // read-file: #RdFl{path}
static u32 OMNI_NAM_WRFL;  // write-file: #WrFl{path, content}
static u32 OMNI_NAM_APFL;  // append-file: #ApFl{path, content}
static u32 OMNI_NAM_RDLN;  // read-lines: #RdLn{path}
static u32 OMNI_NAM_PRNT;  // print: #Prnt{val}
static u32 OMNI_NAM_PRNL;  // println: #PrnL{val}
static u32 OMNI_NAM_RDLN2; // read-line (stdin): #RdL2{}
static u32 OMNI_NAM_GTEV;  // getenv: #GtEv{name}
static u32 OMNI_NAM_STEV;  // setenv: #StEv{name, val}
static u32 OMNI_NAM_EXST;  // file-exists?: #Exst{path}
static u32 OMNI_NAM_ISDR;  // dir?: #IsDr{path}
static u32 OMNI_NAM_MKDR;  // mkdir: #MkDr{path}
static u32 OMNI_NAM_LSDR;  // list-dir: #LsDr{path}
static u32 OMNI_NAM_DLFL;  // delete-file: #DlFl{path}
static u32 OMNI_NAM_RNFL;  // rename-file: #RnFl{from, to}
static u32 OMNI_NAM_CPFL;  // copy-file: #CpFl{from, to}

// JSON operations (FFI-backed)
static u32 OMNI_NAM_JPRS;  // json-parse: #JPrs{str}
static u32 OMNI_NAM_JSTR;  // json-stringify: #JStr{val}
static u32 OMNI_NAM_JARR;  // JSON array marker: #JArr (for type distinction)
static u32 OMNI_NAM_JOBJ;  // JSON object marker: #JObj (for type distinction)
static u32 OMNI_NAM_JNUL;  // JSON null: #JNul

// DateTime operations (FFI-backed)
static u32 OMNI_NAM_DTNW;  // datetime-now: #DtNw{}
static u32 OMNI_NAM_DTPR;  // datetime-parse: #DtPr{str, fmt}
static u32 OMNI_NAM_DTFM;  // datetime-format: #DtFm{dt, fmt}
static u32 OMNI_NAM_DTAD;  // datetime-add: #DtAd{dt, duration}
static u32 OMNI_NAM_DTSB;  // datetime-sub: #DtSb{dt, duration}
static u32 OMNI_NAM_DTDF;  // datetime-diff: #DtDf{dt1, dt2}
static u32 OMNI_NAM_DT;    // DateTime value: #Dt{year, month, day, hour, min, sec, nsec}
static u32 OMNI_NAM_DUR;   // Duration value: #Dur{secs, nsecs}
static u32 OMNI_NAM_DTYR;  // datetime-year: #DtYr{dt}
static u32 OMNI_NAM_DTMO;  // datetime-month: #DtMo{dt}
static u32 OMNI_NAM_DTDY;  // datetime-day: #DtDy{dt}
static u32 OMNI_NAM_DTHR;  // datetime-hour: #DtHr{dt}
static u32 OMNI_NAM_DTMI;  // datetime-minute: #DtMi{dt}
static u32 OMNI_NAM_DTSC;  // datetime-second: #DtSc{dt}
static u32 OMNI_NAM_DTTS;  // datetime-timestamp: #DtTs{dt}
static u32 OMNI_NAM_DTFT;  // datetime-from-timestamp: #DtFt{ts}

// Tower/Meta-programming (multi-stage evaluation)
static u32 OMNI_NAM_LIFT;  // lift: #Lift{expr} - move to meta-level
static u32 OMNI_NAM_RUN;   // run: #Run{expr} - execute meta-level code
static u32 OMNI_NAM_EM;    // EM (Eval Meta): #EM{expr} - jump to parent level
static u32 OMNI_NAM_CLAM;  // clambda: #CLam{env, body} - compiled lambda
static u32 OMNI_NAM_STAG;  // stage: #Stag{level, expr} - staged expression
static u32 OMNI_NAM_SPLI;  // splice: #Spli{expr} - splice into code
static u32 OMNI_NAM_REFL;  // reflect: #Refl{val} - reflect value as code
static u32 OMNI_NAM_REIF;  // reify: #Reif{code} - reify code as value
static u32 OMNI_NAM_MLVL;  // meta-level: #MLvl{level} - current meta-level
static u32 OMNI_NAM_LPAR;  // lazy-parent: #LPar{thunk} - lazy parent reference

// Networking (FFI-backed)
static u32 OMNI_NAM_SOCK;  // socket: #Sock{handle} - socket handle
static u32 OMNI_NAM_TCPC;  // tcp-connect: #TcpC{host, port}
static u32 OMNI_NAM_TCPL;  // tcp-listen: #TcpL{port}
static u32 OMNI_NAM_TCPA;  // tcp-accept: #TcpA{server_sock}
static u32 OMNI_NAM_TCPS;  // tcp-send: #TcpS{sock, data}
static u32 OMNI_NAM_TCPR;  // tcp-recv: #TcpR{sock, max_len}
static u32 OMNI_NAM_UDPC;  // udp-socket: #UdpC{}
static u32 OMNI_NAM_UDPB;  // udp-bind: #UdpB{sock, port}
static u32 OMNI_NAM_UDPS;  // udp-send-to: #UdpS{sock, host, port, data}
static u32 OMNI_NAM_UDPR;  // udp-recv-from: #UdpR{sock, max_len}
static u32 OMNI_NAM_SCLS;  // socket-close: #SCls{sock}
static u32 OMNI_NAM_HTTP;  // http-request: #Http{method, url, headers, body}
static u32 OMNI_NAM_HGET;  // http-get: #HGet{url}
static u32 OMNI_NAM_HPOS;  // http-post: #HPos{url, body}
static u32 OMNI_NAM_HRES;  // http-response: #HRes{status, headers, body}

// Developer Tools
static u32 OMNI_NAM_INSP;  // inspect: #Insp{val, depth} - examine value structure
static u32 OMNI_NAM_TYOF;  // type-of: #TyOf{val} - get runtime type
static u32 OMNI_NAM_DOC;   // doc: #Doc{sym} - get documentation
static u32 OMNI_NAM_TRCE;  // trace: #Trce{label, val} - debug logging
static u32 OMNI_NAM_TIME;  // time: #Time{expr} - measure execution time
static u32 OMNI_NAM_EXPD;  // expand: #Expd{expr} - macro expansion
static u32 OMNI_NAM_DBUG;  // debug: #Dbug{expr} - debug breakpoint
static u32 OMNI_NAM_PRTY;  // pprint: #Prty{val} - pretty print
static u32 OMNI_NAM_SRCE;  // source: #Srce{fn} - get source code
static u32 OMNI_NAM_PROF;  // profile: #Prof{expr} - profiling wrapper

// Type annotations (erased at runtime, used for dispatch)
static u32 OMNI_NAM_TANN;  // Type annotation: #TAnn{expr, type}
static u32 OMNI_NAM_TDSC;  // Type descriptor: #TDsc{name, parent, fields}
static u32 OMNI_NAM_TVAR;  // Type variable: #TVar{name}
static u32 OMNI_NAM_TFUN;  // Function type: #TFun{args, ret}
static u32 OMNI_NAM_TFUNE; // Function type with effects: #TFunE{args, ret, effects}
static u32 OMNI_NAM_TSUP;  // Superposition type: #TSup{elem}
static u32 OMNI_NAM_TWSUP; // Weighted superposition: #TWSup{elem}
static u32 OMNI_NAM_TAPP;  // Type application: #TApp{base, args}
static u32 OMNI_NAM_VTYP;  // Value type (singleton): #VTyp{value} - {3} = #val 3

// Type definitions
static u32 OMNI_NAM_TABS;  // Abstract type: #TAbs{name, parent}
static u32 OMNI_NAM_TSTR;  // Struct type: #TStr{name, parent, fields, mutable}
static u32 OMNI_NAM_TENM;  // Enum type: #TEnm{name, variants}
static u32 OMNI_NAM_TUNI;  // Union type: #TUni{name, types}
static u32 OMNI_NAM_TFLD;  // Field descriptor: #TFld{name, type}
static u32 OMNI_NAM_TVRN;  // Enum variant: #TVrn{name, fields}
static u32 OMNI_NAM_TEFF;  // Effect type: #TEff{name, operations}
                           // e.g., (define {effect Error} ...) → #TEff{Error, ops}
static u32 OMNI_NAM_TEOP;  // Effect operation: #TEOp{name, params, ret_type}
                           // e.g., [raise [msg {String}] {bottom}] → #TEOp{raise, params, bottom}

// Metadata
static u32 OMNI_NAM_META;  // Metadata: #Meta{key, value, target}
static u32 OMNI_NAM_PURE;  // Purity marker: #Pure{fn} - function has no side effects
static u32 OMNI_NAM_ASSC;  // Associative marker: #Assc{fn} - function is associative (can use tree reduction)
static u32 OMNI_NAM_COVR;  // Covariance marker: #Covr{tvar}
                           // e.g., ^:covar T → type parameter T is covariant
static u32 OMNI_NAM_CNVR;  // Contravariance marker: #Cnvr{tvar}
                           // e.g., ^:contravar T → type parameter T is contravariant

// FFI
static u32 OMNI_NAM_FFI;   // FFI call: #FFI{name, args}
static u32 OMNI_NAM_HNDL;  // Handle: #Hndl{idx, gen}
static u32 OMNI_NAM_PTR;   // Raw pointer: #Ptr{hi, lo}
static u32 OMNI_NAM_PEND;  // Pending future: #Pend{id}

// Algebraic effects
static u32 OMNI_NAM_PERF;  // Perform: #Perf{tag, payload}
static u32 OMNI_NAM_HDLE;  // Handle: #Hdle{handlers, body}
static u32 OMNI_NAM_HDLR;  // Handler instance: #Hdlr{tag, handler_fn}
static u32 OMNI_NAM_HDEF;  // Handler definition: #HdlrDef{tag, fn_expr}
static u32 OMNI_NAM_EFF;   // Effect declaration
static u32 OMNI_NAM_ERWS;  // Effect row signature: #ERws{effects}
                           // e.g., ^:effects [{Error} {Ask}] → #ERws{[Error, Ask]}
static u32 OMNI_NAM_EFFR;  // Effect-free check: #Effr{fn} - returns true if fn has empty effect row
static u32 OMNI_NAM_STPR;  // Staged-pure check: #StPr{code} - compile-time purity analysis of AST
static u32 OMNI_NAM_MPCH;  // Map-chunks: #MpCh{f, xs, size} - chunked parallel map
static u32 OMNI_NAM_CPMF;  // Compile-parallel-map factory: #CpMf{f} - generate parallel map code

// Proof-as-Effect system (contract verification)
// These use the algebraic effects system for proof obligations
static u32 OMNI_NAM_REQR;  // Require effect (precondition): #Reqr{predicate}
                           // ^:require P or (require P) → (perform require P)
static u32 OMNI_NAM_ENSR;  // Ensure effect (postcondition): #Ensr{predicate}
                           // ^:ensure Q or (ensure Q) → (perform ensure Q)
static u32 OMNI_NAM_PROV;  // Prove effect: #Prov{goal} - request proof from handler

// Proof terms (Curry-Howard correspondence)
// Types ↔ Propositions, Terms ↔ Proofs
static u32 OMNI_NAM_PRRF;  // Proof reflexivity: #PrRf{a} - proof that a = a
static u32 OMNI_NAM_PRSM;  // Proof symmetry: #PrSm{pf} - if pf : a = b then b = a
static u32 OMNI_NAM_PRTR;  // Proof transitivity: #PrTr{pf1, pf2} - a=b, b=c → a=c
static u32 OMNI_NAM_PRCG;  // Proof congruence: #PrCg{f, pf} - a=b → f(a)=f(b)
static u32 OMNI_NAM_PRJL;  // J eliminator: #PrJl{motive, base, pf} - equality elim
static u32 OMNI_NAM_PRQD;  // QED marker: #PrQd{proof_term} - completed proof
static u32 OMNI_NAM_PRBY;  // Proof by: #PrBy{tactic} - proof via tactic application
static u32 OMNI_NAM_PRSK;  // Proof sketch: #PrSk{hint} - incomplete proof (hole)

// Proof search (SUP-powered parallel exploration)
static u32 OMNI_NAM_PRSP;  // Proof superposition: #PrSp{branches} - parallel search
static u32 OMNI_NAM_PRFL;  // Proof failure: #PrFl{reason} - search branch failed
static u32 OMNI_NAM_PRSC;  // Proof success: #PrSc{proof} - search found proof

// Concurrency
static u32 OMNI_NAM_FIBR;  // Fiber: #Fibr{state, cont, mailbox}
static u32 OMNI_NAM_FORK;  // Fork (HVM4 superposition): #Fork{a, b}
static u32 OMNI_NAM_AMB;   // Nondeterminism
static u32 OMNI_NAM_CHOI;  // Choice (nested superposition): #Choi{list}
static u32 OMNI_NAM_REQT;  // Require test: #Reqt{cond} - reject if cond is false (exploration)
static u32 OMNI_NAM_EXFR;  // Explore-first: #ExFr{choices, pred} - first matching choice
static u32 OMNI_NAM_EXAL;  // Explore-all: #ExAl{choices, body} - collect all valid results
static u32 OMNI_NAM_EXRG;  // Explore-range: #ExRg{lo, hi} - explore integer range

// Speculative Transactions (A3)
static u32 OMNI_NAM_ROLL;  // Rollback: #Roll{reason} - abort transaction
static u32 OMNI_NAM_COMT;  // Commit: #Comt{value} - commit transaction successfully
static u32 OMNI_NAM_SPTX;  // Speculative transaction: #SpTx{strategies} - race strategies
static u32 OMNI_NAM_WROL;  // With-rollback: #WRol{body, cleanup} - transaction with cleanup

// Ambient Parallelism (A5)
static u32 OMNI_NAM_PCTX;  // Parallel context: #PCtx{} - get current parallel context
static u32 OMNI_NAM_FJOI;  // Fork-join: #FJoi{tasks} - parallel task execution
static u32 OMNI_NAM_WPAR;  // With-parallelism: #WPar{workers, body} - set parallel context

// Probabilistic Effects (A7)
static u32 OMNI_NAM_BERN;  // Bernoulli distribution: #Bern{prob} - weighted coin
static u32 OMNI_NAM_CATG;  // Categorical distribution: #Catg{probs} - discrete distribution
static u32 OMNI_NAM_UNIF;  // Uniform distribution: #Unif{lo, hi} - uniform range
static u32 OMNI_NAM_BETA;  // Beta distribution: #Beta{alpha, beta} - beta distribution
static u32 OMNI_NAM_SMPL;  // Sample: #Smpl{dist} - sample from distribution
static u32 OMNI_NAM_OBSV;  // Observe: #Obsv{cond} - condition on observation
static u32 OMNI_NAM_FCTR;  // Factor: #Fctr{weight} - weight current execution path
static u32 OMNI_NAM_FLIP;  // Flip: #Flip{prob} - weighted coin flip (returns bool)
static u32 OMNI_NAM_ENMR;  // Enumerate-infer: #EnmI{model} - exact probabilistic inference
static u32 OMNI_NAM_IMPS;  // Importance-sample: #ImpS{model, n} - approximate inference
static u32 OMNI_NAM_WGTS;  // Weighted superposition: #WgtS{val, weight} - value with probability weight
static u32 OMNI_NAM_DMIX;  // Mixture distribution: #DMix{dists, weights} - weighted combination
static u32 OMNI_NAM_DPRD;  // Product distribution: #DPrd{dists} - independent joint distribution
static u32 OMNI_NAM_DMAP;  // Mapped distribution: #DMap{dist, fn} - transform samples

// Definitions
static u32 OMNI_NAM_DEF;   // Define: #Def{name, params, body}
static u32 OMNI_NAM_SLOT;  // Parameter slot: #Slot{name, type}

// Multiple dispatch
static u32 OMNI_NAM_METH;  // Method: #Meth{name, sig, impl, constraints, effects}
                           // constraints: list of #TWhr{tvar, bound} from ^:where
                           // effects: #ERws{effect_types} from ^:effects or NIL
static u32 OMNI_NAM_GFUN;  // Generic function: #GFun{name, methods}
static u32 OMNI_NAM_GPRT;  // Generic partial application: #GPrt{name, methods, args, remaining}
static u32 OMNI_NAM_DISP;  // Dispatch call: #Disp{name, args}
static u32 OMNI_NAM_SIG;   // Signature: #Sig{types}
static u32 OMNI_NAM_TWHR;  // Type constraint (^:where): #TWhr{tvar, bound}
                           // e.g., ^:where [T {Number}] → #TWhr{T, Number}
static u32 OMNI_NAM_TYCK;  // Type check: #Tyck{value, type}

// Macro system
static u32 OMNI_NAM_MSYN;  // Syntax macro definition: #MSyn{name, patterns}
static u32 OMNI_NAM_MPAT;  // Macro pattern: #MPat{pattern, template}
static u32 OMNI_NAM_MVAR;  // Macro pattern variable: #MVar{name}
static u32 OMNI_NAM_MRST;  // Macro rest pattern: #MRst{name} for ... patterns
static u32 OMNI_NAM_MLIT;  // Macro literal: #MLit{value} (matches exactly)
static u32 OMNI_NAM_MEXP;  // Macro expansion call: #MExp{name, args}

// Module system
static u32 OMNI_NAM_MODL;  // Module definition: #Modl{name, exports, body}
static u32 OMNI_NAM_IMPT;  // Import: #Impt{module_name, bindings}
static u32 OMNI_NAM_EXPT;  // Export list: #Expt{names}
static u32 OMNI_NAM_QUAL;  // Qualified access: #Qual{module, name}
static u32 OMNI_NAM_MODA;  // Module alias: #ModAlias{env} - for (import M :as Alias)

// Code/quasiquote
static u32 OMNI_NAM_COD;   // Code: #Cod{expr}
static u32 OMNI_NAM_QUOT;  // Quote symbol for (quote expr)
static u32 OMNI_NAM_QQ;    // Quasiquote: #QQ{expr}
static u32 OMNI_NAM_UQ;    // Unquote: #UQ{expr}
static u32 OMNI_NAM_UQS;   // Unquote-splicing: #UQS{expr}

// Runtime support
static u32 OMNI_NAM_MENV;  // Meta-environment
static u32 OMNI_NAM_CLO;   // Closure
static u32 OMNI_NAM_CLOR;  // Recursive closure
static u32 OMNI_NAM_ERR;   // Error
static u32 OMNI_NAM_NOTH;  // Nothing

// Delimited continuations
static u32 OMNI_NAM_PRMT;  // Reset/prompt: #Prmt{body}
static u32 OMNI_NAM_CTRL;  // Control/shift: #Ctrl{k_idx, body}
static u32 OMNI_NAM_CLOC;  // CPS closure: #CloC{env, body}
static u32 OMNI_NAM_CLOK;  // Recursive CPS closure: #CloK{env, body}
static u32 OMNI_NAM_KONT;  // Captured continuation: #Kont{k}

// Fiber support
static u32 OMNI_NAM_FYLD;  // Fiber yield marker: #FYld{val, k}
static u32 OMNI_NAM_FTHK;  // Fiber thunk: #FThk{body}
static u32 OMNI_NAM_FBRR;  // Fiber running: #FbrR
static u32 OMNI_NAM_FBRS;  // Fiber suspended: #FbrS
static u32 OMNI_NAM_FBRD;  // Fiber done: #FbrD
static u32 OMNI_NAM_YLD;   // Yield expression: #Yld{val}

// Fiber operations (exposed to user)
static u32 OMNI_NAM_FSPN;  // Fiber spawn: #FSpn{body} - create and start fiber
static u32 OMNI_NAM_FRSM;  // Fiber resume: #FRsm{fiber, value} - resume suspended fiber
static u32 OMNI_NAM_FDNP;  // Fiber done?: #FDn?{fiber} - check if fiber completed
static u32 OMNI_NAM_FRST;  // Fiber result: #FRst{fiber} - get final result
static u32 OMNI_NAM_FMBX;  // Fiber mailbox: #FMbx{fiber} - get yielded values

// Boolean values
static u32 OMNI_NAM_TRUE;  // true
static u32 OMNI_NAM_FALS;  // false

// Collection operations (gradual multiple dispatch)
// These are the user-facing generic functions using gradual dispatch:
//   - Typed args: compile-time method selection
//   - Untyped args: runtime dispatch based on collection type
// Behavior by type: List/Array → eager, Iterator → lazy wrapper
static u32 OMNI_NAM_SET;   // Set: #Set{elements}
static u32 OMNI_NAM_SORT;  // Sort: #Sort{list, cmp}
static u32 OMNI_NAM_GRPB;  // Group-by: #GrpB{list, key_fn}
static u32 OMNI_NAM_ZIP;   // Zip: #Zip{lists} - generic, dispatches to #IZip for iterators
static u32 OMNI_NAM_FLAT;  // Flatten: #Flat{nested}
static u32 OMNI_NAM_CONC;  // Concat: #Conc{lists}
static u32 OMNI_NAM_MAP;   // Map: #Map{fn, coll} - generic dispatch
static u32 OMNI_NAM_FILT;  // Filter: #Filt{pred, coll} - generic dispatch
static u32 OMNI_NAM_FOLD;  // Fold: #Fold{fn, init, coll} - generic dispatch
static u32 OMNI_NAM_TAKE;  // Take: #Take{n, coll} - generic dispatch
static u32 OMNI_NAM_DROP;  // Drop: #Drop{n, coll} - generic dispatch
static u32 OMNI_NAM_REV;   // Reverse: #Rev{coll}
static u32 OMNI_NAM_FIND;  // Find: #Find{pred, coll} - generic dispatch
static u32 OMNI_NAM_ANY;   // Any?: #Any{pred, coll} - generic dispatch
static u32 OMNI_NAM_ALL;   // All?: #All{pred, coll} - generic dispatch
static u32 OMNI_NAM_CONJ;  // Add element: #Conj{coll, val} - generic dispatch
static u32 OMNI_NAM_REMV;  // Remove element: #Remv{coll, val} - generic dispatch
static u32 OMNI_NAM_INTR;  // Intersection: #Intr{s1, s2} - generic dispatch
static u32 OMNI_NAM_UNIN;  // Union: #Unin{s1, s2} - generic dispatch
static u32 OMNI_NAM_DIFF;  // Difference: #Diff{s1, s2} - generic dispatch
static u32 OMNI_NAM_KEYS;  // Keys: #Keys{coll} - generic dispatch
static u32 OMNI_NAM_VALS;  // Values: #Vals{coll} - generic dispatch
static u32 OMNI_NAM_ENTS;  // Entries: #Ents{coll} - generic dispatch
static u32 OMNI_NAM_MERG;  // Merge: #Merg{colls} - generic dispatch
static u32 OMNI_NAM_ASOC;  // Assoc: #Asoc{coll, key, val} (alias for put)
static u32 OMNI_NAM_DISS;  // Dissoc: #Diss{coll, key} (remove by key)
static u32 OMNI_NAM_GET;   // Get: #Get{coll, key} - generic dispatch
static u32 OMNI_NAM_LEN;   // Length: #Len{coll} - generic dispatch
static u32 OMNI_NAM_EMPT;  // Empty?: #Empt{coll} - generic dispatch
static u32 OMNI_NAM_CONT;  // Contains?: #Cont{coll, val} - generic dispatch
static u32 OMNI_NAM_INDX;  // Index-of: #Indx{coll, val} - generic dispatch
static u32 OMNI_NAM_NTH;   // Nth: #Nth{coll, n} - generic dispatch
static u32 OMNI_NAM_SLCE;  // Slice: #Slce{coll, start, end} - generic dispatch
static u32 OMNI_NAM_FRNG;  // From range: #FRng{start, end, step}

// String operations
static u32 OMNI_NAM_STR;   // String: #Str{chars}
static u32 OMNI_NAM_SUPR;  // String upper: #SUpR{str}
static u32 OMNI_NAM_SLWR;  // String lower: #SLwR{str}
static u32 OMNI_NAM_STRM;  // String trim: #STrm{str}
static u32 OMNI_NAM_SSPL;  // String split: #SSpl{str, delim}
static u32 OMNI_NAM_SJOI;  // String join: #SJoi{strs, delim}
static u32 OMNI_NAM_SRPL;  // String replace: #SRpl{str, old, new}
static u32 OMNI_NAM_SSUB;  // String substring: #SSub{str, start, len}
static u32 OMNI_NAM_SIND;  // String index-of: #SInd{str, needle}
static u32 OMNI_NAM_SSTA;  // String starts-with: #SSta{str, prefix}
static u32 OMNI_NAM_SEND;  // String ends-with: #SEnd{str, suffix}
static u32 OMNI_NAM_SCNT;  // String contains: #SCnt{str, needle}
static u32 OMNI_NAM_SREV;  // String reverse: #SRev{str}
static u32 OMNI_NAM_SPAD;  // String pad: #SPad{str, len, char, side}
static u32 OMNI_NAM_SCAP;  // String capitalize: #SCap{str}
static u32 OMNI_NAM_SCHC;  // String char-at: #SChc{str, idx}
static u32 OMNI_NAM_SLEN;  // String length: #SLen{str}
static u32 OMNI_NAM_SEMP;  // String empty?: #SEmp{str}
static u32 OMNI_NAM_SCMP;  // String compare: #SCmp{str1, str2}
static u32 OMNI_NAM_SREP;  // String repeat: #SRep{str, n}
static u32 OMNI_NAM_FMTS;  // Format string: #Fmts{parts} - interpolated string
                           // parts is list of #Flit{str} and #Fexp{expr} alternating
static u32 OMNI_NAM_FLIT;  // Format literal part: #Flit{chars}
static u32 OMNI_NAM_FEXP;  // Format expression part: #Fexp{expr}

// Regex operations
static u32 OMNI_NAM_REGX;  // Regex literal: #Regx{pattern, flags}
static u32 OMNI_NAM_RMAT;  // Regex match: #RMat{regex, str}
static u32 OMNI_NAM_RFND;  // Regex find: #RFnd{regex, str}
static u32 OMNI_NAM_RFNA;  // Regex find-all: #RFnA{regex, str}
static u32 OMNI_NAM_RRPL;  // Regex replace: #RRpl{regex, str, replacement}
static u32 OMNI_NAM_RSPL;  // Regex split: #RSpl{regex, str}
static u32 OMNI_NAM_RGRP;  // Regex group: #RGrp{match, index}
static u32 OMNI_NAM_RMRS;  // Regex match result: #RMRs{matched, groups, start, end}

// Grammar DSL (Pika)
static u32 OMNI_NAM_GRAM;  // Grammar definition: #Gram{name, rules}
static u32 OMNI_NAM_RULE;  // Grammar rule: #Rule{name, pattern}
static u32 OMNI_NAM_GSEQ;  // Sequence: #GSeq{patterns}
static u32 OMNI_NAM_GALT;  // Alternative: #GAlt{patterns}
static u32 OMNI_NAM_GSTR;  // String literal: #GStr{chars}
static u32 OMNI_NAM_GCHR;  // Character class: #GChr{chars, negated}
static u32 OMNI_NAM_GREF;  // Rule reference: #GRef{name}
static u32 OMNI_NAM_GOPT;  // Optional (?): #GOpt{pattern}
static u32 OMNI_NAM_GSTA;  // Star (*): #GSta{pattern}
static u32 OMNI_NAM_GPLS;  // Plus (+): #GPls{pattern}
static u32 OMNI_NAM_GNOT;  // Not predicate (!): #GNot{pattern}
static u32 OMNI_NAM_GAND;  // And predicate (&): #GAnd{pattern}
static u32 OMNI_NAM_GCAP;  // Capture: #GCap{name, pattern}
static u32 OMNI_NAM_GACT;  // Action: #GAct{pattern, action}
static u32 OMNI_NAM_GANY;  // Any character (.): #GAny{}
static u32 OMNI_NAM_PRSR;  // Parser state: #Prsr{input, pos, captures}

// List comprehensions (parallel by default)
static u32 OMNI_NAM_CMPR;  // Comprehension: #Cmpr{clauses, yield_expr}
static u32 OMNI_NAM_CFOR;  // For clause: #CFor{var_nick, collection}
static u32 OMNI_NAM_CWHN;  // When clause: #CWhn{predicate}
static u32 OMNI_NAM_CYLD;  // Yield marker: #CYld{expr}
static u32 OMNI_NAM_PRES;  // Parse result: #PRes{success, value, pos}

fn void omni_names_init(void) {
  if (OMNI_NAMES_READY) return;

  // Core expressions
  OMNI_NAM_LIT  = omni_nick("Lit");
  OMNI_NAM_SYM  = omni_nick("Sym");
  OMNI_NAM_VAR  = omni_nick("Var");
  OMNI_NAM_LAM  = omni_nick("Lam");
  OMNI_NAM_LAMR = omni_nick("LamR");
  OMNI_NAM_APP  = omni_nick("App");
  OMNI_NAM_LET  = omni_nick("Let");
  OMNI_NAM_LETS = omni_nick("LetS");
  OMNI_NAM_LETP = omni_nick("LetP");
  OMNI_NAM_IF   = omni_nick("If");
  OMNI_NAM_DO   = omni_nick("Do");

  // Arithmetic
  OMNI_NAM_ADD = omni_nick("Add");
  OMNI_NAM_SUB = omni_nick("Sub");
  OMNI_NAM_MUL = omni_nick("Mul");
  OMNI_NAM_DIV = omni_nick("Div");
  OMNI_NAM_MOD = omni_nick("Mod");
  OMNI_NAM_EQL = omni_nick("Eql");
  OMNI_NAM_NEQ = omni_nick("Neq");
  OMNI_NAM_LT  = omni_nick("Lt");
  OMNI_NAM_GT  = omni_nick("Gt");
  OMNI_NAM_LE  = omni_nick("Le");
  OMNI_NAM_GE  = omni_nick("Ge");
  OMNI_NAM_AND = omni_nick("And");
  OMNI_NAM_OR  = omni_nick("Or");
  OMNI_NAM_NOT = omni_nick("Not");

  // Type predicates
  OMNI_NAM_INTP = omni_nick("IntP");
  OMNI_NAM_LSTP = omni_nick("LstP");
  OMNI_NAM_NILP = omni_nick("NilP");
  OMNI_NAM_NUMP = omni_nick("NumP");

  // Data structures
  OMNI_NAM_CON  = NAM_CON;  // Use HVM4's CON
  OMNI_NAM_NIL  = NAM_NIL;  // Use HVM4's NIL
  OMNI_NAM_CHR  = NAM_CHR;  // Use HVM4's CHR
  OMNI_NAM_ARR  = omni_nick("Arr");
  OMNI_NAM_DICT = omni_nick("Dict");
  OMNI_NAM_FST  = omni_nick("Fst");
  OMNI_NAM_SND  = omni_nick("Snd");

  // Numeric
  OMNI_NAM_CST = omni_nick("Cst");
  OMNI_NAM_FIX = omni_nick("Fix");

  // Pattern matching
  OMNI_NAM_MAT  = omni_nick("Mat");
  OMNI_NAM_MATS = omni_nick("MatS");  // Speculative match (^:speculate)
  OMNI_NAM_NMAT = omni_nick("NMat");
  OMNI_NAM_CASE = omni_nick("Case");
  OMNI_NAM_PCTR = omni_nick("PCtr");
  OMNI_NAM_PLIT = omni_nick("PLit");
  OMNI_NAM_PWLD = omni_nick("PWld");
  OMNI_NAM_PVAR = omni_nick("PVar");
  OMNI_NAM_PAS  = omni_nick("PAs");
  OMNI_NAM_PRST = omni_nick("PRst");
  OMNI_NAM_PGRD = omni_nick("PGrd");
  OMNI_NAM_GUAR = omni_nick("Guar");
  OMNI_NAM_SPRD = omni_nick("Sprd");
  OMNI_NAM_DLET = omni_nick("DLet");
  OMNI_NAM_PARR = omni_nick("PArr");
  OMNI_NAM_DLAM = omni_nick("DLam");
  OMNI_NAM_POR  = omni_nick("POr");
  OMNI_NAM_PLST = omni_nick("PLst");

  // Named let (Scheme-style loop)
  OMNI_NAM_NLET = omni_nick("NLet");   // Parallel (default)
  OMNI_NAM_NLETS = omni_nick("NLeS");  // Sequential (^:seq)

  // Pipe and function utilities
  OMNI_NAM_PIPE = omni_nick("Pipe");
  OMNI_NAM_CURY = omni_nick("Cury");
  OMNI_NAM_FLIP = omni_nick("Flip");
  OMNI_NAM_ROTR = omni_nick("Rotr");
  OMNI_NAM_COMP = omni_nick("Comp");
  OMNI_NAM_APPL = omni_nick("Appl");

  // Conditional forms
  OMNI_NAM_WHEN = omni_nick("When");
  OMNI_NAM_UNLS = omni_nick("Unls");
  OMNI_NAM_COND = omni_nick("Cond");
  OMNI_NAM_CCLS = omni_nick("CCls");

  // Path access (functional)
  OMNI_NAM_GET  = omni_nick("Get");
  OMNI_NAM_PUT  = omni_nick("Put");
  OMNI_NAM_UPDT = omni_nick("Updt");
  OMNI_NAM_GTIN = omni_nick("GtIn");
  OMNI_NAM_ASIN = omni_nick("AsIn");
  OMNI_NAM_UPIN = omni_nick("UpIn");

  // Mutation primitives
  OMNI_NAM_SETB  = omni_nick("SetB");
  OMNI_NAM_PUTB  = omni_nick("PutB");
  OMNI_NAM_UPDTB = omni_nick("UpdB");

  // Iterators
  OMNI_NAM_ITER = omni_nick("Iter");
  OMNI_NAM_RANG = omni_nick("Rang");
  OMNI_NAM_IMAP = omni_nick("IMap");
  OMNI_NAM_IFLT = omni_nick("IFlt");
  OMNI_NAM_ITKN = omni_nick("ITkn");
  OMNI_NAM_IDRP = omni_nick("IDrp");
  OMNI_NAM_DONE = omni_nick("Done");
  OMNI_NAM_IZIP = omni_nick("IZip");
  OMNI_NAM_ICHN = omni_nick("IChn");
  OMNI_NAM_IENM = omni_nick("IEnm");
  OMNI_NAM_ITKW = omni_nick("ITkW");
  OMNI_NAM_IDRW = omni_nick("IDrW");
  OMNI_NAM_IFLD = omni_nick("IFld");
  OMNI_NAM_IFND = omni_nick("IFnd");
  OMNI_NAM_IANY = omni_nick("IAny");
  OMNI_NAM_IALL = omni_nick("IAll");
  OMNI_NAM_INTH = omni_nick("INth");
  OMNI_NAM_IFMP = omni_nick("IFMp");
  OMNI_NAM_ISTP = omni_nick("IStp");
  OMNI_NAM_ICHK = omni_nick("IChk");
  OMNI_NAM_IWIN = omni_nick("IWin");

  // Math functions
  OMNI_NAM_SQRT = omni_nick("Sqrt");
  OMNI_NAM_POW  = omni_nick("Pow");
  OMNI_NAM_MEXP = omni_nick("MExp");
  OMNI_NAM_MLOG = omni_nick("MLog");
  OMNI_NAM_MSIN = omni_nick("MSin");
  OMNI_NAM_MCOS = omni_nick("MCos");
  OMNI_NAM_MTAN = omni_nick("MTan");
  OMNI_NAM_MASN = omni_nick("MAsn");
  OMNI_NAM_MACS = omni_nick("MAcs");
  OMNI_NAM_MATN = omni_nick("MAtn");
  OMNI_NAM_MABS = omni_nick("MAbs");
  OMNI_NAM_FLOR = omni_nick("Flor");
  OMNI_NAM_MCEI = omni_nick("MCei");
  OMNI_NAM_ROND = omni_nick("Rond");
  OMNI_NAM_RAND = omni_nick("Rand");

  // I/O operations
  OMNI_NAM_RDFL = omni_nick("RdFl");
  OMNI_NAM_WRFL = omni_nick("WrFl");
  OMNI_NAM_APFL = omni_nick("ApFl");
  OMNI_NAM_RDLN = omni_nick("RdLn");
  OMNI_NAM_PRNT = omni_nick("Prnt");
  OMNI_NAM_PRNL = omni_nick("PrnL");
  OMNI_NAM_RDLN2 = omni_nick("RdL2");
  OMNI_NAM_GTEV = omni_nick("GtEv");
  OMNI_NAM_STEV = omni_nick("StEv");
  OMNI_NAM_EXST = omni_nick("Exst");
  OMNI_NAM_ISDR = omni_nick("IsDr");
  OMNI_NAM_MKDR = omni_nick("MkDr");
  OMNI_NAM_LSDR = omni_nick("LsDr");
  OMNI_NAM_DLFL = omni_nick("DlFl");
  OMNI_NAM_RNFL = omni_nick("RnFl");
  OMNI_NAM_CPFL = omni_nick("CpFl");

  // JSON operations
  OMNI_NAM_JPRS = omni_nick("JPrs");
  OMNI_NAM_JSTR = omni_nick("JStr");
  OMNI_NAM_JARR = omni_nick("JArr");
  OMNI_NAM_JOBJ = omni_nick("JObj");
  OMNI_NAM_JNUL = omni_nick("JNul");

  // DateTime operations
  OMNI_NAM_DTNW = omni_nick("DtNw");
  OMNI_NAM_DTPR = omni_nick("DtPr");
  OMNI_NAM_DTFM = omni_nick("DtFm");
  OMNI_NAM_DTAD = omni_nick("DtAd");
  OMNI_NAM_DTSB = omni_nick("DtSb");
  OMNI_NAM_DTDF = omni_nick("DtDf");
  OMNI_NAM_DT   = omni_nick("Dt");
  OMNI_NAM_DUR  = omni_nick("Dur");
  OMNI_NAM_DTYR = omni_nick("DtYr");
  OMNI_NAM_DTMO = omni_nick("DtMo");
  OMNI_NAM_DTDY = omni_nick("DtDy");
  OMNI_NAM_DTHR = omni_nick("DtHr");
  OMNI_NAM_DTMI = omni_nick("DtMi");
  OMNI_NAM_DTSC = omni_nick("DtSc");
  OMNI_NAM_DTTS = omni_nick("DtTs");
  OMNI_NAM_DTFT = omni_nick("DtFt");

  // Tower/Meta-programming
  OMNI_NAM_LIFT = omni_nick("Lift");
  OMNI_NAM_RUN  = omni_nick("Run");
  OMNI_NAM_EM   = omni_nick("EM");
  OMNI_NAM_CLAM = omni_nick("CLam");
  OMNI_NAM_STAG = omni_nick("Stag");
  OMNI_NAM_SPLI = omni_nick("Spli");
  OMNI_NAM_REFL = omni_nick("Refl");
  OMNI_NAM_REIF = omni_nick("Reif");
  OMNI_NAM_MLVL = omni_nick("MLvl");
  OMNI_NAM_LPAR = omni_nick("LPar");

  // Networking (FFI-backed)
  OMNI_NAM_SOCK = omni_nick("Sock");
  OMNI_NAM_TCPC = omni_nick("TcpC");
  OMNI_NAM_TCPL = omni_nick("TcpL");
  OMNI_NAM_TCPA = omni_nick("TcpA");
  OMNI_NAM_TCPS = omni_nick("TcpS");
  OMNI_NAM_TCPR = omni_nick("TcpR");
  OMNI_NAM_UDPC = omni_nick("UdpC");
  OMNI_NAM_UDPB = omni_nick("UdpB");
  OMNI_NAM_UDPS = omni_nick("UdpS");
  OMNI_NAM_UDPR = omni_nick("UdpR");
  OMNI_NAM_SCLS = omni_nick("SCls");
  OMNI_NAM_HTTP = omni_nick("Http");
  OMNI_NAM_HGET = omni_nick("HGet");
  OMNI_NAM_HPOS = omni_nick("HPos");
  OMNI_NAM_HRES = omni_nick("HRes");

  // Developer Tools
  OMNI_NAM_INSP = omni_nick("Insp");
  OMNI_NAM_TYOF = omni_nick("TyOf");
  OMNI_NAM_DOC  = omni_nick("Doc");
  OMNI_NAM_TRCE = omni_nick("Trce");
  OMNI_NAM_TIME = omni_nick("Time");
  OMNI_NAM_EXPD = omni_nick("Expd");
  OMNI_NAM_DBUG = omni_nick("Dbug");
  OMNI_NAM_PRTY = omni_nick("Prty");
  OMNI_NAM_SRCE = omni_nick("Srce");
  OMNI_NAM_PROF = omni_nick("Prof");

  // Type annotations
  OMNI_NAM_TANN = omni_nick("TAnn");
  OMNI_NAM_TDSC = omni_nick("TDsc");
  OMNI_NAM_TVAR = omni_nick("TVar");
  OMNI_NAM_TFUN = omni_nick("TFun");
  OMNI_NAM_TFUNE = omni_nick("TFunE");
  OMNI_NAM_TSUP = omni_nick("TSup");
  OMNI_NAM_TWSUP = omni_nick("TWSup");
  OMNI_NAM_TAPP = omni_nick("TApp");
  OMNI_NAM_VTYP = omni_nick("VTyp");

  // Type definitions
  OMNI_NAM_TABS = omni_nick("TAbs");
  OMNI_NAM_TSTR = omni_nick("TStr");
  OMNI_NAM_TENM = omni_nick("TEnm");
  OMNI_NAM_TUNI = omni_nick("TUni");
  OMNI_NAM_TFLD = omni_nick("TFld");
  OMNI_NAM_TVRN = omni_nick("TVrn");
  OMNI_NAM_TEFF = omni_nick("TEff");  // Effect type definition
  OMNI_NAM_TEOP = omni_nick("TEOp");  // Effect operation

  // Metadata
  OMNI_NAM_META = omni_nick("Meta");
  OMNI_NAM_PURE = omni_nick("Pure");  // Purity marker
  OMNI_NAM_ASSC = omni_nick("Assc");  // Associative marker
  OMNI_NAM_COVR = omni_nick("Covr");  // Covariance marker
  OMNI_NAM_CNVR = omni_nick("Cnvr");  // Contravariance marker

  // FFI
  OMNI_NAM_FFI  = omni_nick("FFI");
  OMNI_NAM_HNDL = omni_nick("Hndl");
  OMNI_NAM_PTR  = omni_nick("Ptr");
  OMNI_NAM_PEND = omni_nick("Pend");

  // Effects
  OMNI_NAM_PERF = omni_nick("Perf");
  OMNI_NAM_HDLE = omni_nick("Hdle");
  OMNI_NAM_HDLR = omni_nick("Hdlr");
  OMNI_NAM_HDEF = omni_nick("HDef");
  OMNI_NAM_EFF  = omni_nick("Eff");
  OMNI_NAM_ERWS = omni_nick("ERws");  // Effect row signature
  OMNI_NAM_EFFR = omni_nick("Effr");  // Effect-free check
  OMNI_NAM_STPR = omni_nick("StPr");  // Staged-pure check
  OMNI_NAM_MPCH = omni_nick("MpCh");  // Map-chunks
  OMNI_NAM_CPMF = omni_nick("CpMf");  // Compile-parallel-map factory

  // Proof-as-Effect system
  OMNI_NAM_REQR = omni_nick("Reqr");  // Require (precondition)
  OMNI_NAM_ENSR = omni_nick("Ensr");  // Ensure (postcondition)
  OMNI_NAM_PROV = omni_nick("Prov");  // Prove (request proof)

  // Proof terms
  OMNI_NAM_PRRF = omni_nick("PrRf");  // Reflexivity
  OMNI_NAM_PRSM = omni_nick("PrSm");  // Symmetry
  OMNI_NAM_PRTR = omni_nick("PrTr");  // Transitivity
  OMNI_NAM_PRCG = omni_nick("PrCg");  // Congruence
  OMNI_NAM_PRJL = omni_nick("PrJl");  // J eliminator
  OMNI_NAM_PRQD = omni_nick("PrQd");  // QED marker
  OMNI_NAM_PRBY = omni_nick("PrBy");  // Proof by tactic
  OMNI_NAM_PRSK = omni_nick("PrSk");  // Proof sketch (hole)

  // Proof search
  OMNI_NAM_PRSP = omni_nick("PrSp");  // Superposition (parallel)
  OMNI_NAM_PRFL = omni_nick("PrFl");  // Failure
  OMNI_NAM_PRSC = omni_nick("PrSc");  // Success

  // Concurrency
  OMNI_NAM_FIBR = omni_nick("Fibr");
  OMNI_NAM_FORK = omni_nick("Fork");
  OMNI_NAM_AMB  = omni_nick("Amb");
  OMNI_NAM_CHOI = omni_nick("Choi");  // Choice (nested superposition)
  OMNI_NAM_REQT = omni_nick("Reqt");  // Require test (exploration)
  OMNI_NAM_EXFR = omni_nick("ExFr");  // Explore-first
  OMNI_NAM_EXAL = omni_nick("ExAl");  // Explore-all
  OMNI_NAM_EXRG = omni_nick("ExRg");  // Explore-range

  // Speculative Transactions (A3)
  OMNI_NAM_ROLL = omni_nick("Roll");  // Rollback
  OMNI_NAM_COMT = omni_nick("Comt");  // Commit
  OMNI_NAM_SPTX = omni_nick("SpTx");  // Speculative transaction
  OMNI_NAM_WROL = omni_nick("WRol");  // With-rollback

  // Ambient Parallelism (A5)
  OMNI_NAM_PCTX = omni_nick("PCtx");  // Parallel context
  OMNI_NAM_FJOI = omni_nick("FJoi");  // Fork-join
  OMNI_NAM_WPAR = omni_nick("WPar");  // With-parallelism

  // Probabilistic Effects (A7)
  OMNI_NAM_BERN = omni_nick("Bern");  // Bernoulli distribution
  OMNI_NAM_CATG = omni_nick("Catg");  // Categorical distribution
  OMNI_NAM_UNIF = omni_nick("Unif");  // Uniform distribution
  OMNI_NAM_BETA = omni_nick("Beta");  // Beta distribution
  OMNI_NAM_SMPL = omni_nick("Smpl");  // Sample
  OMNI_NAM_OBSV = omni_nick("Obsv");  // Observe
  OMNI_NAM_FCTR = omni_nick("Fctr");  // Factor
  OMNI_NAM_FLIP = omni_nick("Flip");  // Flip (weighted coin)
  OMNI_NAM_ENMR = omni_nick("EnmI");  // Enumerate-infer
  OMNI_NAM_IMPS = omni_nick("ImpS");  // Importance-sample
  OMNI_NAM_WGTS = omni_nick("WgtS");  // Weighted superposition
  OMNI_NAM_DMIX = omni_nick("DMix");  // Mixture distribution
  OMNI_NAM_DPRD = omni_nick("DPrd");  // Product distribution
  OMNI_NAM_DMAP = omni_nick("DMap");  // Mapped distribution

  // Definitions
  OMNI_NAM_DEF  = omni_nick("Def");
  OMNI_NAM_SLOT = omni_nick("Slot");

  // Multiple dispatch
  OMNI_NAM_METH = omni_nick("Meth");
  OMNI_NAM_GFUN = omni_nick("GFun");
  OMNI_NAM_GPRT = omni_nick("GPrt");  // Generic partial application
  OMNI_NAM_DISP = omni_nick("Disp");
  OMNI_NAM_SIG  = omni_nick("Sig");
  OMNI_NAM_TWHR = omni_nick("TWhr");  // Type where constraint
  OMNI_NAM_TYCK = omni_nick("Tyck");  // Type check

  // Macro system
  OMNI_NAM_MSYN = omni_nick("MSyn");
  OMNI_NAM_MPAT = omni_nick("MPat");
  OMNI_NAM_MVAR = omni_nick("MVar");
  OMNI_NAM_MRST = omni_nick("MRst");
  OMNI_NAM_MLIT = omni_nick("MLit");
  OMNI_NAM_MEXP = omni_nick("MExp");

  // Module system
  OMNI_NAM_MODL = omni_nick("Modl");
  OMNI_NAM_IMPT = omni_nick("Impt");
  OMNI_NAM_EXPT = omni_nick("Expt");
  OMNI_NAM_QUAL = omni_nick("Qual");
  OMNI_NAM_MODA = omni_nick("ModA");

  // Code/quasiquote
  OMNI_NAM_COD  = omni_nick("Cod");
  OMNI_NAM_QUOT = omni_nick("quot");
  OMNI_NAM_QQ   = omni_nick("QQ");
  OMNI_NAM_UQ  = omni_nick("UQ");
  OMNI_NAM_UQS = omni_nick("UQS");

  // Runtime
  OMNI_NAM_MENV = omni_nick("MEnv");
  OMNI_NAM_CLO  = omni_nick("Clo");
  OMNI_NAM_CLOR = omni_nick("CloR");
  OMNI_NAM_ERR  = omni_nick("Err");
  OMNI_NAM_NOTH = omni_nick("Noth");

  // Delimited continuations
  OMNI_NAM_PRMT = omni_nick("Prmt");
  OMNI_NAM_CTRL = omni_nick("Ctrl");
  OMNI_NAM_CLOC = omni_nick("CloC");
  OMNI_NAM_CLOK = omni_nick("CloK");
  OMNI_NAM_KONT = omni_nick("Kont");

  // Fiber support
  OMNI_NAM_FYLD = omni_nick("FYld");
  OMNI_NAM_FTHK = omni_nick("FThk");
  OMNI_NAM_FBRR = omni_nick("FbrR");
  OMNI_NAM_FBRS = omni_nick("FbrS");
  OMNI_NAM_FBRD = omni_nick("FbrD");
  OMNI_NAM_YLD  = omni_nick("Yld");

  // Fiber operations (exposed to user)
  OMNI_NAM_FSPN = omni_nick("FSpn");
  OMNI_NAM_FRSM = omni_nick("FRsm");
  OMNI_NAM_FDNP = omni_nick("FDn?");
  OMNI_NAM_FRST = omni_nick("FRst");
  OMNI_NAM_FMBX = omni_nick("FMbx");

  // Booleans
  OMNI_NAM_TRUE = omni_nick("True");
  OMNI_NAM_FALS = omni_nick("Fals");

  // Collection operations
  OMNI_NAM_SET  = omni_nick("Set");
  OMNI_NAM_SORT = omni_nick("Sort");
  OMNI_NAM_GRPB = omni_nick("GrpB");
  OMNI_NAM_ZIP  = omni_nick("Zip");
  OMNI_NAM_FLAT = omni_nick("Flat");
  OMNI_NAM_CONC = omni_nick("Conc");
  OMNI_NAM_MAP  = omni_nick("Map");
  OMNI_NAM_FILT = omni_nick("Filt");
  OMNI_NAM_FOLD = omni_nick("Fold");
  OMNI_NAM_TAKE = omni_nick("Take");
  OMNI_NAM_DROP = omni_nick("Drop");
  OMNI_NAM_REV  = omni_nick("Rev");
  OMNI_NAM_FIND = omni_nick("Find");
  OMNI_NAM_ANY  = omni_nick("Any");
  OMNI_NAM_ALL  = omni_nick("All");
  OMNI_NAM_CONJ = omni_nick("Conj");
  OMNI_NAM_REMV = omni_nick("Remv");
  OMNI_NAM_INTR = omni_nick("Intr");
  OMNI_NAM_UNIN = omni_nick("Unin");
  OMNI_NAM_DIFF = omni_nick("Diff");
  OMNI_NAM_KEYS = omni_nick("Keys");
  OMNI_NAM_VALS = omni_nick("Vals");
  OMNI_NAM_ENTS = omni_nick("Ents");
  OMNI_NAM_MERG = omni_nick("Merg");
  OMNI_NAM_ASOC = omni_nick("Asoc");
  OMNI_NAM_DISS = omni_nick("Diss");
  OMNI_NAM_GET  = omni_nick("Get");
  OMNI_NAM_LEN  = omni_nick("Len");
  OMNI_NAM_EMPT = omni_nick("Empt");
  OMNI_NAM_CONT = omni_nick("Cont");
  OMNI_NAM_INDX = omni_nick("Indx");
  OMNI_NAM_NTH  = omni_nick("Nth");
  OMNI_NAM_SLCE = omni_nick("Slce");
  OMNI_NAM_FRNG = omni_nick("FRng");

  // String operations
  OMNI_NAM_STR  = omni_nick("Str");
  OMNI_NAM_SUPR = omni_nick("SUpR");
  OMNI_NAM_SLWR = omni_nick("SLwR");
  OMNI_NAM_STRM = omni_nick("STrm");
  OMNI_NAM_SSPL = omni_nick("SSpl");
  OMNI_NAM_SJOI = omni_nick("SJoi");
  OMNI_NAM_SRPL = omni_nick("SRpl");
  OMNI_NAM_SSUB = omni_nick("SSub");
  OMNI_NAM_SIND = omni_nick("SInd");
  OMNI_NAM_SSTA = omni_nick("SSta");
  OMNI_NAM_SEND = omni_nick("SEnd");
  OMNI_NAM_SCNT = omni_nick("SCnt");
  OMNI_NAM_SREV = omni_nick("SRev");
  OMNI_NAM_SPAD = omni_nick("SPad");
  OMNI_NAM_SCAP = omni_nick("SCap");
  OMNI_NAM_SCHC = omni_nick("SChc");
  OMNI_NAM_SLEN = omni_nick("SLen");
  OMNI_NAM_SEMP = omni_nick("SEmp");
  OMNI_NAM_SCMP = omni_nick("SCmp");
  OMNI_NAM_SREP = omni_nick("SRep");
  OMNI_NAM_FMTS = omni_nick("Fmts");
  OMNI_NAM_FLIT = omni_nick("Flit");
  OMNI_NAM_FEXP = omni_nick("Fexp");

  // Regex operations
  OMNI_NAM_REGX = omni_nick("Regx");
  OMNI_NAM_RMAT = omni_nick("RMat");
  OMNI_NAM_RFND = omni_nick("RFnd");
  OMNI_NAM_RFNA = omni_nick("RFnA");
  OMNI_NAM_RRPL = omni_nick("RRpl");
  OMNI_NAM_RSPL = omni_nick("RSpl");
  OMNI_NAM_RGRP = omni_nick("RGrp");
  OMNI_NAM_RMRS = omni_nick("RMRs");

  // Grammar DSL
  OMNI_NAM_GRAM = omni_nick("Gram");
  OMNI_NAM_RULE = omni_nick("Rule");
  OMNI_NAM_GSEQ = omni_nick("GSeq");
  OMNI_NAM_GALT = omni_nick("GAlt");
  OMNI_NAM_GSTR = omni_nick("GStr");
  OMNI_NAM_GCHR = omni_nick("GChr");
  OMNI_NAM_GREF = omni_nick("GRef");
  OMNI_NAM_GOPT = omni_nick("GOpt");
  OMNI_NAM_GSTA = omni_nick("GSta");
  OMNI_NAM_GPLS = omni_nick("GPls");
  OMNI_NAM_GNOT = omni_nick("GNot");
  OMNI_NAM_GAND = omni_nick("GAnd");
  OMNI_NAM_GCAP = omni_nick("GCap");
  OMNI_NAM_GACT = omni_nick("GAct");
  OMNI_NAM_GANY = omni_nick("GAny");
  OMNI_NAM_PRSR = omni_nick("Prsr");
  OMNI_NAM_PRES = omni_nick("PRes");

  // List comprehensions
  OMNI_NAM_CMPR = omni_nick("Cmpr");
  OMNI_NAM_CFOR = omni_nick("CFor");
  OMNI_NAM_CWHN = omni_nick("CWhn");
  OMNI_NAM_CYLD = omni_nick("CYld");

  OMNI_NAMES_READY = 1;
}
