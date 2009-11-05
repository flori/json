#include <string.h>
#include "ruby.h"
#if HAVE_RUBY_ST_H
#include "ruby/st.h"
#endif
#if HAVE_ST_H
#include "st.h"
#endif
#include "unicode.h"
#include <math.h>
#if HAVE_RUBY_RE_H
#include "ruby/re.h"
#endif
#if HAVE_RE_H
#include "re.h"
#endif

inline static VALUE cState_partial_generate(VALUE self, VALUE obj, VALUE depth);
inline static VALUE cState_from_state_s(VALUE self, VALUE opts);

#ifndef RHASH_TBL
#define RHASH_TBL(hsh) (RHASH(hsh)->tbl)
#endif

#ifndef RHASH_SIZE
#define RHASH_SIZE(hsh) (RHASH(hsh)->tbl->num_entries)
#endif

#ifndef RFLOAT_VALUE
#define RFLOAT_VALUE(val) (RFLOAT(val)->value)
#endif

#ifdef HAVE_RUBY_ENCODING_H
#include "ruby/encoding.h"
#define FORCE_UTF8(obj) rb_enc_associate((obj), rb_utf8_encoding())
static VALUE CEncoding_UTF_8;
static ID i_encoding, i_encode;
#else
#define FORCE_UTF8(obj)
#endif

static VALUE mJSON, mExt, mGenerator, cState, mGeneratorMethods, mObject,
             mHash, mArray, mInteger, mFloat, mString, mString_Extend,
             mTrueClass, mFalseClass, mNilClass, eGeneratorError,
             eNestingError, CRegexp_MULTILINE;

static ID i_to_s, i_to_json, i_new, i_indent, i_space, i_space_before,
          i_object_nl, i_array_nl, i_max_nesting,
          i_allow_nan, i_ascii_only, i_pack, i_unpack, i_create_id, i_extend;

typedef struct JSON_Generator_StateStruct {
    char *indent;
    long indent_len;
    char *space;
    long space_len;
    char *space_before;
    long space_before_len;
    char *object_nl;
    long object_nl_len;
    char *array_nl;
    long array_nl_len;
    FBuffer *array_delim;
    FBuffer *object_delim;
    FBuffer *object_delim2;
    long max_nesting;
    char allow_nan;
    char ascii_only;
} JSON_Generator_State;

#define GET_STATE(self)                       \
    JSON_Generator_State *state;              \
    Data_Get_Struct(self, JSON_Generator_State, state);

#define RSTRING_PAIR(string) RSTRING_PTR(string), RSTRING_LEN(string)

/* 
 * Document-module: JSON::Ext::Generator
 *
 * This is the JSON generator implemented as a C extension. It can be
 * configured to be used by setting
 *
 *  JSON.generator = JSON::Ext::Generator
 *
 * with the method generator= in JSON.
 *
 */

/*
 * call-seq: to_json(state = nil, depth = 0)
 *
 * Returns a JSON string containing a JSON object, that is generated from
 * this Hash instance.
 * _state_ is a JSON::State object, that can also be used to configure the
 * produced JSON string output further.
 * _depth_ is used to find out nesting depth, to indent accordingly.
 */
static VALUE mHash_to_json(int argc, VALUE *argv, VALUE self)
{
    VALUE state, depth;

    rb_scan_args(argc, argv, "02", &state, &depth);
    state = cState_from_state_s(cState, state);
    return cState_partial_generate(state, self, depth);
}

/*
 * call-seq: to_json(state = nil, depth = 0)
 *
 * Returns a JSON string containing a JSON array, that is generated from
 * this Array instance.
 * _state_ is a JSON::State object, that can also be used to configure the
 * produced JSON string output further.
 * _depth_ is used to find out nesting depth, to indent accordingly.
 */
static VALUE mArray_to_json(int argc, VALUE *argv, VALUE self) {
    VALUE state, depth;
    rb_scan_args(argc, argv, "02", &state, &depth);
    state = cState_from_state_s(cState, state);
    return cState_partial_generate(state, self, depth);
}

/*
 * call-seq: to_json(*)
 *
 * Returns a JSON string representation for this Integer number.
 */
static VALUE mInteger_to_json(int argc, VALUE *argv, VALUE self)
{
    VALUE state, depth;
    rb_scan_args(argc, argv, "02", &state, &depth);
    state = cState_from_state_s(cState, state);
    return cState_partial_generate(state, self, depth);
}

/*
 * call-seq: to_json(*)
 *
 * Returns a JSON string representation for this Float number.
 */
static VALUE mFloat_to_json(int argc, VALUE *argv, VALUE self)
{
    VALUE state, depth;
    rb_scan_args(argc, argv, "02", &state, &depth);
    state = cState_from_state_s(cState, state);
    return cState_partial_generate(state, self, depth);
}

/*
 * call-seq: String.included(modul)
 *
 * Extends _modul_ with the String::Extend module.
 */
static VALUE mString_included_s(VALUE self, VALUE modul) {
    VALUE result = rb_funcall(modul, i_extend, 1, mString_Extend);
    return result;
}

/*
 * call-seq: to_json(*)
 *
 * This string should be encoded with UTF-8 A call to this method
 * returns a JSON string encoded with UTF16 big endian characters as
 * \u????.
 */
static VALUE mString_to_json(int argc, VALUE *argv, VALUE self)
{
    VALUE state, depth;
    rb_scan_args(argc, argv, "02", &state, &depth);
    state = cState_from_state_s(cState, state);
    return cState_partial_generate(state, self, depth);
}

/*
 * call-seq: to_json_raw_object()
 *
 * This method creates a raw object hash, that can be nested into
 * other data structures and will be generated as a raw string. This
 * method should be used, if you want to convert raw strings to JSON
 * instead of UTF-8 strings, e. g. binary data.
 */
static VALUE mString_to_json_raw_object(VALUE self) {
    VALUE ary;
    VALUE result = rb_hash_new();
    rb_hash_aset(result, rb_funcall(mJSON, i_create_id, 0), rb_class_name(rb_obj_class(self)));
    ary = rb_funcall(self, i_unpack, 1, rb_str_new2("C*"));
    rb_hash_aset(result, rb_str_new2("raw"), ary);
    return result;
}

/*
 * call-seq: to_json_raw(*args)
 *
 * This method creates a JSON text from the result of a call to
 * to_json_raw_object of this String.
 */
static VALUE mString_to_json_raw(int argc, VALUE *argv, VALUE self) {
    VALUE obj = mString_to_json_raw_object(self);
    Check_Type(obj, T_HASH);
    return mHash_to_json(argc, argv, obj);
}

/*
 * call-seq: json_create(o)
 *
 * Raw Strings are JSON Objects (the raw bytes are stored in an array for the
 * key "raw"). The Ruby String can be created by this module method.
 */
static VALUE mString_Extend_json_create(VALUE self, VALUE o) {
    VALUE ary;
    Check_Type(o, T_HASH);
    ary = rb_hash_aref(o, rb_str_new2("raw"));
    return rb_funcall(ary, i_pack, 1, rb_str_new2("C*"));
}

/*
 * call-seq: to_json(*)
 *
 * Returns a JSON string for true: 'true'.
 */
static VALUE mTrueClass_to_json(int argc, VALUE *argv, VALUE self)
{
    VALUE state, depth;
    rb_scan_args(argc, argv, "02", &state, &depth);
    state = cState_from_state_s(cState, state);
    return cState_partial_generate(state, self, depth);
}

/*
 * call-seq: to_json(*)
 *
 * Returns a JSON string for false: 'false'.
 */
static VALUE mFalseClass_to_json(int argc, VALUE *argv, VALUE self)
{
    VALUE state, depth;
    rb_scan_args(argc, argv, "02", &state, &depth);
    state = cState_from_state_s(cState, state);
    return cState_partial_generate(state, self, depth);
}

/*
 * call-seq: to_json(*)
 *
 */
static VALUE mNilClass_to_json(int argc, VALUE *argv, VALUE self)
{
    VALUE state, depth;
    rb_scan_args(argc, argv, "02", &state, &depth);
    state = cState_from_state_s(cState, state);
    return cState_partial_generate(state, self, depth);
}

/*
 * call-seq: to_json(*)
 *
 * Converts this object to a string (calling #to_s), converts
 * it to a JSON string, and returns the result. This is a fallback, if no
 * special method #to_json was defined for some object.
 */
static VALUE mObject_to_json(int argc, VALUE *argv, VALUE self)
{
    VALUE state, depth;
    VALUE string = rb_funcall(self, i_to_s, 0);
    rb_scan_args(argc, argv, "02", &state, &depth);
    Check_Type(string, T_STRING);
    state = cState_from_state_s(cState, state);
    return cState_partial_generate(state, string, depth);
}

static void State_free(JSON_Generator_State *state) {
    if (state->indent) ruby_xfree(state->indent);
    if (state->space) ruby_xfree(state->space);
    if (state->space_before) ruby_xfree(state->space_before);
    if (state->object_nl) ruby_xfree(state->object_nl);
    if (state->array_nl) ruby_xfree(state->array_nl);
    if (state->array_delim) fbuffer_free(state->array_delim);
    if (state->object_delim) fbuffer_free(state->object_delim);
    if (state->object_delim2) fbuffer_free(state->object_delim2);
    ruby_xfree(state);
}

static JSON_Generator_State *State_allocate()
{
    JSON_Generator_State *state = ALLOC(JSON_Generator_State);
    return state;
}

static VALUE cState_s_allocate(VALUE klass)
{
    JSON_Generator_State *state = State_allocate();
    return Data_Wrap_Struct(klass, NULL, State_free, state);
}

/*
 * call-seq: configure(opts)
 *
 * Configure this State instance with the Hash _opts_, and return
 * itself.
 */
static VALUE cState_configure(VALUE self, VALUE opts)
{
    VALUE tmp;
    GET_STATE(self);
    tmp = rb_convert_type(opts, T_HASH, "Hash", "to_hash");
    if (NIL_P(tmp)) tmp = rb_convert_type(opts, T_HASH, "Hash", "to_h");
    if (NIL_P(tmp)) {
        rb_raise(rb_eArgError, "opts has to be hash like or convertable into a hash");
    }
    opts = tmp;
    tmp = rb_hash_aref(opts, ID2SYM(i_indent));
    if (RTEST(tmp)) {
        Check_Type(tmp, T_STRING);
        state->indent = strdup(RSTRING_PTR(tmp));
        state->indent_len = strlen(state->indent);
    }
    tmp = rb_hash_aref(opts, ID2SYM(i_space));
    if (RTEST(tmp)) {
        Check_Type(tmp, T_STRING);
        state->space = strdup(RSTRING_PTR(tmp));
        state->space_len = strlen(state->space);
    }
    tmp = rb_hash_aref(opts, ID2SYM(i_space_before));
    if (RTEST(tmp)) {
        Check_Type(tmp, T_STRING);
        state->space_before = strdup(RSTRING_PTR(tmp));
        state->space_before_len = strlen(state->space_before);
    }
    tmp = rb_hash_aref(opts, ID2SYM(i_array_nl));
    if (RTEST(tmp)) {
        Check_Type(tmp, T_STRING);
        state->array_nl = strdup(RSTRING_PTR(tmp));
        state->array_nl_len = strlen(state->array_nl);
    }
    tmp = rb_hash_aref(opts, ID2SYM(i_object_nl));
    if (RTEST(tmp)) {
        Check_Type(tmp, T_STRING);
        state->object_nl = strdup(RSTRING_PTR(tmp));
        state->object_nl_len = strlen(state->object_nl);
    }
    tmp = ID2SYM(i_max_nesting);
    state->max_nesting = 19;
    if (st_lookup(RHASH_TBL(opts), tmp, 0)) {
        VALUE max_nesting = rb_hash_aref(opts, tmp);
        if (RTEST(max_nesting)) {
            Check_Type(max_nesting, T_FIXNUM);
            state->max_nesting = FIX2LONG(max_nesting);
        } else {
            state->max_nesting = 0;
        }
    }
    tmp = rb_hash_aref(opts, ID2SYM(i_allow_nan));
    state->allow_nan = RTEST(tmp);
    tmp = rb_hash_aref(opts, ID2SYM(i_ascii_only));
    state->ascii_only = RTEST(tmp);
    return self;
}

/*
 * call-seq: to_h
 *
 * Returns the configuration instance variables as a hash, that can be
 * passed to the configure method.
 */
static VALUE cState_to_h(VALUE self)
{
    VALUE result = rb_hash_new();
    GET_STATE(self);
    rb_hash_aset(result, ID2SYM(i_indent), rb_str_new2(state->indent));
    rb_hash_aset(result, ID2SYM(i_space), rb_str_new2(state->space));
    rb_hash_aset(result, ID2SYM(i_space_before), rb_str_new2(state->space_before));
    rb_hash_aset(result, ID2SYM(i_object_nl), rb_str_new2(state->object_nl));
    rb_hash_aset(result, ID2SYM(i_array_nl), rb_str_new2(state->array_nl));
    rb_hash_aset(result, ID2SYM(i_allow_nan), state->allow_nan ? Qtrue : Qfalse);
    rb_hash_aset(result, ID2SYM(i_ascii_only), state->ascii_only ? Qtrue : Qfalse);
    rb_hash_aset(result, ID2SYM(i_max_nesting), LONG2FIX(state->max_nesting));
    return result;
}

inline static VALUE fbuffer2rstring(FBuffer *buffer)
{
    NEWOBJ(str, struct RString);
    OBJSETUP(str, rb_cString, T_STRING);

    str->ptr = FBUFFER_PTR(buffer);
    str->len = FBUFFER_LEN(buffer);
    str->aux.capa = FBUFFER_CAPA(buffer);

    return (VALUE) str;
}

void generate_json(FBuffer *buffer, VALUE Vstate, JSON_Generator_State *state, VALUE obj, long depth)
{
    VALUE tmp;
    switch (TYPE(obj)) {
        case T_HASH:
            {
                char *object_nl = state->object_nl;
                long object_nl_len = state->object_nl_len;
                char *indent = state->indent;
                long indent_len = state->indent_len;
                long max_nesting = state->max_nesting;
                char *delim = FBUFFER_PTR(state->object_delim);
                long delim_len = FBUFFER_LEN(state->object_delim);
                char *delim2 = FBUFFER_PTR(state->object_delim2);
                long delim2_len = FBUFFER_LEN(state->object_delim2);
                int i, j;
                depth++;
                if (max_nesting != 0 && depth > max_nesting) {
                    fbuffer_free(buffer);
                    rb_raise(eNestingError, "nesting of %ld is too deep", depth);
                }
                fbuffer_append_char(buffer, '{');
                VALUE keys = rb_funcall(obj, rb_intern("keys"), 0);
                VALUE key, key_to_s;
                for(i = 0; i < RARRAY_LEN(keys); i++) {
                    if (i > 0) fbuffer_append(buffer, delim, delim_len);
                    if (object_nl) {
                        fbuffer_append(buffer, object_nl, object_nl_len);
                    }
                    if (indent) {
                        for (j = 0; j < depth; j++) {
                            fbuffer_append(buffer, indent, indent_len);
                        }
                    }
                    key = rb_ary_entry(keys, i);
                    key_to_s = rb_funcall(key, i_to_s, 0);
                    Check_Type(key_to_s, T_STRING);
                    generate_json(buffer, Vstate, state, key_to_s, depth);
                    fbuffer_append(buffer, delim2, delim2_len);
                    generate_json(buffer, Vstate, state, rb_hash_aref(obj, key), depth);
                }
                depth--;
                if (object_nl) {
                    fbuffer_append(buffer, object_nl, object_nl_len);
                    if (indent) {
                        for (j = 0; j < depth; j++) {
                            fbuffer_append(buffer, indent, indent_len);
                        }
                    }
                }
                fbuffer_append_char(buffer, '}');
            }
            break;
        case T_ARRAY:
            {
                char *array_nl = state->array_nl;
                long array_nl_len = state->array_nl_len;
                char *indent = state->indent;
                long indent_len = state->indent_len;
                long max_nesting = state->max_nesting;
                char *delim = FBUFFER_PTR(state->array_delim);
                long delim_len = FBUFFER_LEN(state->array_delim);
                int i, j;
                depth++;
                if (max_nesting != 0 && depth > max_nesting) {
                    fbuffer_free(buffer);
                    rb_raise(eNestingError, "nesting of %ld is too deep", depth);
                }
                fbuffer_append_char(buffer, '[');
                if (array_nl) fbuffer_append(buffer, array_nl, array_nl_len);
                for(i = 0; i < RARRAY_LEN(obj); i++) {
                    if (i > 0) fbuffer_append(buffer, delim, delim_len);
                    if (indent) {
                        for (j = 0; j < depth; j++) {
                            fbuffer_append(buffer, indent, indent_len);
                        }
                    }
                    generate_json(buffer, Vstate, state, rb_ary_entry(obj, i), depth);
                }
                depth--;
                if (array_nl) {
                    fbuffer_append(buffer, array_nl, array_nl_len);
                    if (indent) {
                        for (j = 0; j < depth; j++) {
                            fbuffer_append(buffer, indent, indent_len);
                        }
                    }
                }
                fbuffer_append_char(buffer, ']');
            }
            break;
        case T_STRING:
            fbuffer_append_char(buffer, '"');
#ifdef HAVE_RUBY_ENCODING_H
            obj = rb_funcall(obj, i_encode, 1, CEncoding_UTF_8);
#endif
            if (state->ascii_only) {
                convert_UTF8_to_JSON_ASCII(buffer, obj);
            } else {
                convert_UTF8_to_JSON(buffer, obj);
            }
            fbuffer_append_char(buffer, '"');
            break;
        case T_NIL:
            fbuffer_append(buffer, "null", 4);
            break;
        case T_FALSE:
            fbuffer_append(buffer, "false", 5);
            break;
        case T_TRUE:
            fbuffer_append(buffer, "true", 4);
            break;
        case T_FIXNUM:
        case T_BIGNUM:
            tmp = rb_funcall(obj, i_to_s, 0);
            fbuffer_append(buffer, RSTRING_PAIR(tmp));
            break;
        case T_FLOAT:
            {
                char allow_nan = state->allow_nan;
                double value = RFLOAT_VALUE(obj);
                tmp = rb_funcall(obj, i_to_s, 0);
                if (!allow_nan && isinf(value)) {
                    fbuffer_free(buffer);
                    rb_raise(eGeneratorError, "%u: %s not allowed in JSON", __LINE__, StringValueCStr(tmp));
                } else if (!allow_nan && isnan(value)) {
                    fbuffer_free(buffer);
                    rb_raise(eGeneratorError, "%u: %s not allowed in JSON", __LINE__, StringValueCStr(tmp));
                }
                fbuffer_append(buffer, RSTRING_PAIR(tmp));
            }
            break;
        default:
            if (rb_respond_to(obj, i_to_json)) {
                tmp = rb_funcall(obj, i_to_json, 2, Vstate, INT2FIX(depth + 1));
                Check_Type(tmp, T_STRING);
                fbuffer_append(buffer, RSTRING_PAIR(tmp));
            } else {
                tmp = rb_funcall(obj, i_to_s, 0);
                Check_Type(tmp, T_STRING);
                generate_json(buffer, Vstate, state, tmp, depth + 1);
            }
            break;
    }
}

/*
 * call-seq: partial_generate(obj)
 *
 * Generates a part of a JSON document from object +obj+ and returns the
 * result.
 */
inline static VALUE cState_partial_generate(VALUE self, VALUE obj, VALUE depth)
{
    VALUE result;
    FBuffer *buffer = fbuffer_alloc();
    GET_STATE(self);

    if (state->object_delim) {
        fbuffer_clear(state->object_delim);
    } else {
        state->object_delim = fbuffer_alloc_with_length(16);
    }
    fbuffer_append_char(state->object_delim, ',');
    if (state->object_delim2) {
        fbuffer_clear(state->object_delim2);
    } else {
        state->object_delim2 = fbuffer_alloc_with_length(16);
    }
    fbuffer_append_char(state->object_delim2, ':');
    if (state->space) fbuffer_append(state->object_delim2, state->space, state->space_len);

    if (state->array_delim) {
        fbuffer_clear(state->array_delim);
    } else {
        state->array_delim = fbuffer_alloc_with_length(16);
    }
    fbuffer_append_char(state->array_delim, ',');
    if (state->array_nl) fbuffer_append(state->array_delim, state->array_nl, state->array_nl_len);

    generate_json(buffer, self, state, obj, NIL_P(depth) ? 0 : FIX2INT(depth));
    result = fbuffer2rstring(buffer);
    fbuffer_free_only_buffer(buffer);
    FORCE_UTF8(result);
    return result;
}

/*
 * call-seq: generate(obj)
 *
 * Generates a valid JSON document from object +obj+ and returns the
 * result. If no valid JSON document can be created this method raises a
 * GeneratorError exception.
 */
inline static VALUE cState_generate(VALUE self, VALUE obj)
{
    VALUE result = cState_partial_generate(self, obj, Qnil);
    VALUE re, args[2];
    args[0] = rb_str_new2("\\A\\s*(?:\\[.*\\]|\\{.*\\})\\s*\\Z");
    args[1] = CRegexp_MULTILINE;
    re = rb_class_new_instance(2, args, rb_cRegexp);
    if (NIL_P(rb_reg_match(re, result))) {
        rb_raise(eGeneratorError, "only generation of JSON objects or arrays allowed");
    }
    return result;
}

/*
 * call-seq: new(opts = {})
 *
 * Instantiates a new State object, configured by _opts_.
 *
 * _opts_ can have the following keys:
 *
 * * *indent*: a string used to indent levels (default: ''),
 * * *space*: a string that is put after, a : or , delimiter (default: ''),
 * * *space_before*: a string that is put before a : pair delimiter (default: ''),
 * * *object_nl*: a string that is put at the end of a JSON object (default: ''), 
 * * *array_nl*: a string that is put at the end of a JSON array (default: ''),
 * * *allow_nan*: true if NaN, Infinity, and -Infinity should be
 *   generated, otherwise an exception is thrown, if these values are
 *   encountered. This options defaults to false.
 */
static VALUE cState_initialize(int argc, VALUE *argv, VALUE self)
{
    VALUE opts;
    GET_STATE(self);
    MEMZERO(state, JSON_Generator_State, 1);
    state->max_nesting = 19;
    rb_scan_args(argc, argv, "01", &opts);
    if (!NIL_P(opts)) cState_configure(self, opts);
    return self;
}

/*
 * call-seq: from_state(opts)
 *
 * Creates a State object from _opts_, which ought to be Hash to create a
 * new State instance configured by _opts_, something else to create an
 * unconfigured instance. If _opts_ is a State object, it is just returned.
 */
inline static VALUE cState_from_state_s(VALUE self, VALUE opts)
{
    if (rb_obj_is_kind_of(opts, self)) {
        return opts;
    } else if (rb_obj_is_kind_of(opts, rb_cHash)) {
        return rb_funcall(self, i_new, 1, opts);
    } else {
        return rb_funcall(self, i_new, 0);
    }
}

/*
 * call-seq: indent()
 *
 * This string is used to indent levels in the JSON text.
 */
static VALUE cState_indent(VALUE self)
{
    GET_STATE(self);
    return state->indent ? rb_str_new2(state->indent) : rb_str_new2("");
}

/*
 * call-seq: indent=(indent)
 *
 * This string is used to indent levels in the JSON text.
 */
static VALUE cState_indent_set(VALUE self, VALUE indent)
{
    GET_STATE(self);
    Check_Type(indent, T_STRING);
    if (RSTRING_LEN(indent) == 0) {
        if (state->indent) {
            ruby_xfree(state->indent);
            state->indent = NULL;
        }
    } else {
        if (state->indent) ruby_xfree(state->indent);
        state->indent = strdup(RSTRING_PTR(indent));
    }
    return Qnil;
}

/*
 * call-seq: space()
 *
 * This string is used to insert a space between the tokens in a JSON
 * string.
 */
static VALUE cState_space(VALUE self)
{
    GET_STATE(self);
    return state->space ? rb_str_new2(state->space) : rb_str_new2("");
}

/*
 * call-seq: space=(space)
 *
 * This string is used to insert a space between the tokens in a JSON
 * string.
 */
static VALUE cState_space_set(VALUE self, VALUE space)
{
    GET_STATE(self);
    Check_Type(space, T_STRING);
    if (RSTRING_LEN(space) == 0) {
        if (state->space) {
            ruby_xfree(state->space);
            state->space = NULL;
        }
    } else {
        if (state->space) ruby_xfree(state->space);
        state->space = strdup(RSTRING_PTR(space));
    }
    return Qnil;
}

/*
 * call-seq: space_before()
 *
 * This string is used to insert a space before the ':' in JSON objects.
 */
static VALUE cState_space_before(VALUE self)
{
    GET_STATE(self);
    return state->space_before ? rb_str_new2(state->space_before) : rb_str_new2("");
}

/*
 * call-seq: space_before=(space_before)
 *
 * This string is used to insert a space before the ':' in JSON objects.
 */
static VALUE cState_space_before_set(VALUE self, VALUE space_before)
{
    GET_STATE(self);
    Check_Type(space_before, T_STRING);
    if (RSTRING_LEN(space_before) == 0) {
        if (state->space_before) {
            ruby_xfree(state->space_before);
            state->space_before = NULL;
        }
    } else {
        if (state->space_before) ruby_xfree(state->space_before);
        state->space_before = strdup(RSTRING_PTR(space_before));
    }
    return Qnil;
}

/*
 * call-seq: object_nl()
 *
 * This string is put at the end of a line that holds a JSON object (or
 * Hash).
 */
static VALUE cState_object_nl(VALUE self)
{
    GET_STATE(self);
    return state->object_nl ? rb_str_new2(state->object_nl) : rb_str_new2("");
}

/*
 * call-seq: object_nl=(object_nl)
 *
 * This string is put at the end of a line that holds a JSON object (or
 * Hash).
 */
static VALUE cState_object_nl_set(VALUE self, VALUE object_nl)
{
    GET_STATE(self);
    Check_Type(object_nl, T_STRING);
    if (RSTRING_LEN(object_nl) == 0) {
        if (state->object_nl) {
            ruby_xfree(state->object_nl);
            state->object_nl = NULL;
        }
    } else {
        if (state->object_nl) ruby_xfree(state->object_nl);
        state->object_nl = strdup(RSTRING_PTR(object_nl));
    }
    return Qnil;
}

/*
 * call-seq: array_nl()
 *
 * This string is put at the end of a line that holds a JSON array.
 */
static VALUE cState_array_nl(VALUE self)
{
    GET_STATE(self);
    return state->array_nl ? rb_str_new2(state->array_nl) : rb_str_new2("");
}

/*
 * call-seq: array_nl=(array_nl)
 *
 * This string is put at the end of a line that holds a JSON array.
 */
static VALUE cState_array_nl_set(VALUE self, VALUE array_nl)
{
    GET_STATE(self);
    Check_Type(array_nl, T_STRING);
    if (RSTRING_LEN(array_nl) == 0) {
        if (state->array_nl) {
            ruby_xfree(state->array_nl);
            state->array_nl = NULL;
        }
    } else {
        if (state->array_nl) ruby_xfree(state->array_nl);
        state->array_nl = strdup(RSTRING_PTR(array_nl));
    }
    return Qnil;
}

/*
 * call-seq: max_nesting
 *
 * This integer returns the maximum level of data structure nesting in
 * the generated JSON, max_nesting = 0 if no maximum is checked.
 */
static VALUE cState_max_nesting(VALUE self)
{
    GET_STATE(self);
    return LONG2FIX(state->max_nesting);
}

/*
 * call-seq: max_nesting=(depth)
 *
 * This sets the maximum level of data structure nesting in the generated JSON
 * to the integer depth, max_nesting = 0 if no maximum should be checked.
 */
static VALUE cState_max_nesting_set(VALUE self, VALUE depth)
{
    GET_STATE(self);
    Check_Type(depth, T_FIXNUM);
    return state->max_nesting = FIX2LONG(depth);
}

/*
 * call-seq: allow_nan?
 *
 * Returns true, if NaN, Infinity, and -Infinity should be generated, otherwise
 * returns false.
 */
static VALUE cState_allow_nan_p(VALUE self)
{
    GET_STATE(self);
    return state->allow_nan ? Qtrue : Qfalse;
}

/*
 * call-seq: ascii_only?
 *
 * Returns true, if NaN, Infinity, and -Infinity should be generated, otherwise
 * returns false.
 */
static VALUE cState_ascii_only_p(VALUE self)
{
    GET_STATE(self);
    return state->ascii_only ? Qtrue : Qfalse;
}

/*
 *
 */
void Init_generator()
{
    rb_require("json/common");

    mJSON = rb_define_module("JSON");
    mExt = rb_define_module_under(mJSON, "Ext");
    mGenerator = rb_define_module_under(mExt, "Generator");

    eGeneratorError = rb_path2class("JSON::GeneratorError");
    eNestingError = rb_path2class("JSON::NestingError");

    cState = rb_define_class_under(mGenerator, "State", rb_cObject);
    rb_define_alloc_func(cState, cState_s_allocate);
    rb_define_singleton_method(cState, "from_state", cState_from_state_s, 1);
    rb_define_method(cState, "initialize", cState_initialize, -1);
    rb_define_method(cState, "indent", cState_indent, 0);
    rb_define_method(cState, "indent=", cState_indent_set, 1);
    rb_define_method(cState, "space", cState_space, 0);
    rb_define_method(cState, "space=", cState_space_set, 1);
    rb_define_method(cState, "space_before", cState_space_before, 0);
    rb_define_method(cState, "space_before=", cState_space_before_set, 1);
    rb_define_method(cState, "object_nl", cState_object_nl, 0);
    rb_define_method(cState, "object_nl=", cState_object_nl_set, 1);
    rb_define_method(cState, "array_nl", cState_array_nl, 0);
    rb_define_method(cState, "array_nl=", cState_array_nl_set, 1);
    rb_define_method(cState, "max_nesting", cState_max_nesting, 0);
    rb_define_method(cState, "max_nesting=", cState_max_nesting_set, 1);
    rb_define_method(cState, "allow_nan?", cState_allow_nan_p, 0);
    rb_define_method(cState, "ascii_only?", cState_ascii_only_p, 0);
    rb_define_method(cState, "configure", cState_configure, 1);
    rb_define_method(cState, "to_h", cState_to_h, 0);
    rb_define_method(cState, "generate", cState_generate, 1);
    rb_define_method(cState, "partial_generate", cState_partial_generate, 1);

    mGeneratorMethods = rb_define_module_under(mGenerator, "GeneratorMethods");
    mObject = rb_define_module_under(mGeneratorMethods, "Object");
    rb_define_method(mObject, "to_json", mObject_to_json, -1);
    mHash = rb_define_module_under(mGeneratorMethods, "Hash");
    rb_define_method(mHash, "to_json", mHash_to_json, -1);
    mArray = rb_define_module_under(mGeneratorMethods, "Array");
    rb_define_method(mArray, "to_json", mArray_to_json, -1);
    mInteger = rb_define_module_under(mGeneratorMethods, "Integer");
    rb_define_method(mInteger, "to_json", mInteger_to_json, -1);
    mFloat = rb_define_module_under(mGeneratorMethods, "Float");
    rb_define_method(mFloat, "to_json", mFloat_to_json, -1);
    mString = rb_define_module_under(mGeneratorMethods, "String");
    rb_define_singleton_method(mString, "included", mString_included_s, 1);
    rb_define_method(mString, "to_json", mString_to_json, -1);
    rb_define_method(mString, "to_json_raw", mString_to_json_raw, -1);
    rb_define_method(mString, "to_json_raw_object", mString_to_json_raw_object, 0);
    mString_Extend = rb_define_module_under(mString, "Extend");
    rb_define_method(mString_Extend, "json_create", mString_Extend_json_create, 1);
    mTrueClass = rb_define_module_under(mGeneratorMethods, "TrueClass");
    rb_define_method(mTrueClass, "to_json", mTrueClass_to_json, -1);
    mFalseClass = rb_define_module_under(mGeneratorMethods, "FalseClass");
    rb_define_method(mFalseClass, "to_json", mFalseClass_to_json, -1);
    mNilClass = rb_define_module_under(mGeneratorMethods, "NilClass");
    rb_define_method(mNilClass, "to_json", mNilClass_to_json, -1);

    CRegexp_MULTILINE = rb_const_get(rb_cRegexp, rb_intern("MULTILINE"));
    i_to_s = rb_intern("to_s");
    i_to_json = rb_intern("to_json");
    i_new = rb_intern("new");
    i_indent = rb_intern("indent");
    i_space = rb_intern("space");
    i_space_before = rb_intern("space_before");
    i_object_nl = rb_intern("object_nl");
    i_array_nl = rb_intern("array_nl");
    i_max_nesting = rb_intern("max_nesting");
    i_allow_nan = rb_intern("allow_nan");
    i_ascii_only = rb_intern("ascii_only");
    i_pack = rb_intern("pack");
    i_unpack = rb_intern("unpack");
    i_create_id = rb_intern("create_id");
    i_extend = rb_intern("extend");
#ifdef HAVE_RUBY_ENCODING_H
    CEncoding_UTF_8 = rb_funcall(rb_path2class("Encoding"), rb_intern("find"), 1, rb_str_new2("utf-8"));
    i_encoding = rb_intern("encoding");
    i_encode = rb_intern("encode");
#endif
}
