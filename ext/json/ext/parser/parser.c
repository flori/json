
#line 1 "ext/json/ext/parser/parser.rl"
#include "../fbuffer/fbuffer.h"
#include "parser.h"

#if defined HAVE_RUBY_ENCODING_H
# define EXC_ENCODING rb_utf8_encoding(),
# ifndef HAVE_RB_ENC_RAISE
static void
enc_raise(rb_encoding *enc, VALUE exc, const char *fmt, ...)
{
    va_list args;
    VALUE mesg;

    va_start(args, fmt);
    mesg = rb_enc_vsprintf(enc, fmt, args);
    va_end(args);

    rb_exc_raise(rb_exc_new3(exc, mesg));
}
#   define rb_enc_raise enc_raise
# endif
#else
# define EXC_ENCODING /* nothing */
# define rb_enc_raise rb_raise
#endif

/* unicode */

static const char digit_values[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1,
    -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1
};

static UTF32 unescape_unicode(const unsigned char *p)
{
    char b;
    UTF32 result = 0;
    b = digit_values[p[0]];
    if (b < 0) return UNI_REPLACEMENT_CHAR;
    result = (result << 4) | (unsigned char)b;
    b = digit_values[p[1]];
    if (b < 0) return UNI_REPLACEMENT_CHAR;
    result = (result << 4) | (unsigned char)b;
    b = digit_values[p[2]];
    if (b < 0) return UNI_REPLACEMENT_CHAR;
    result = (result << 4) | (unsigned char)b;
    b = digit_values[p[3]];
    if (b < 0) return UNI_REPLACEMENT_CHAR;
    result = (result << 4) | (unsigned char)b;
    return result;
}

static int convert_UTF32_to_UTF8(char *buf, UTF32 ch)
{
    int len = 1;
    if (ch <= 0x7F) {
        buf[0] = (char) ch;
    } else if (ch <= 0x07FF) {
        buf[0] = (char) ((ch >> 6) | 0xC0);
        buf[1] = (char) ((ch & 0x3F) | 0x80);
        len++;
    } else if (ch <= 0xFFFF) {
        buf[0] = (char) ((ch >> 12) | 0xE0);
        buf[1] = (char) (((ch >> 6) & 0x3F) | 0x80);
        buf[2] = (char) ((ch & 0x3F) | 0x80);
        len += 2;
    } else if (ch <= 0x1fffff) {
        buf[0] =(char) ((ch >> 18) | 0xF0);
        buf[1] =(char) (((ch >> 12) & 0x3F) | 0x80);
        buf[2] =(char) (((ch >> 6) & 0x3F) | 0x80);
        buf[3] =(char) ((ch & 0x3F) | 0x80);
        len += 3;
    } else {
        buf[0] = '?';
    }
    return len;
}

static VALUE mJSON, mExt, cParser, eParserError, eNestingError;
static VALUE CNaN, CInfinity, CMinusInfinity;

static ID i_json_creatable_p, i_json_create, i_create_id, i_create_additions,
          i_chr, i_max_nesting, i_allow_nan, i_symbolize_names,
          i_object_class, i_array_class, i_decimal_class, i_key_p,
          i_deep_const_get, i_match, i_match_string, i_aset, i_aref,
          i_leftshift, i_new;


#line 125 "ext/json/ext/parser/parser.rl"



#line 107 "ext/json/ext/parser/parser.c"
static const char _JSON_object_actions[] = {
	0, 1, 0, 1, 1, 1, 2
};

static const char _JSON_object_key_offsets[] = {
	0, 0, 1, 8, 14, 16, 17, 19, 
	20, 36, 43, 49, 51, 52, 54, 55, 
	57, 58, 60, 61, 63, 64, 66, 67, 
	69, 70, 72, 73
};

static const char _JSON_object_trans_keys[] = {
	123, 13, 32, 34, 47, 125, 9, 10, 
	13, 32, 47, 58, 9, 10, 42, 47, 
	42, 42, 47, 10, 13, 32, 34, 45, 
	47, 73, 78, 91, 102, 110, 116, 123, 
	9, 10, 48, 57, 13, 32, 44, 47, 
	125, 9, 10, 13, 32, 34, 47, 9, 
	10, 42, 47, 42, 42, 47, 10, 42, 
	47, 42, 42, 47, 10, 42, 47, 42, 
	42, 47, 10, 42, 47, 42, 42, 47, 
	10, 0
};

static const char _JSON_object_single_lengths[] = {
	0, 1, 5, 4, 2, 1, 2, 1, 
	12, 5, 4, 2, 1, 2, 1, 2, 
	1, 2, 1, 2, 1, 2, 1, 2, 
	1, 2, 1, 0
};

static const char _JSON_object_range_lengths[] = {
	0, 0, 1, 1, 0, 0, 0, 0, 
	2, 1, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0
};

static const char _JSON_object_index_offsets[] = {
	0, 0, 2, 9, 15, 18, 20, 23, 
	25, 40, 47, 53, 56, 58, 61, 63, 
	66, 68, 71, 73, 76, 78, 81, 83, 
	86, 88, 91, 93
};

static const char _JSON_object_indicies[] = {
	0, 1, 0, 0, 2, 3, 4, 0, 
	1, 5, 5, 6, 7, 5, 1, 8, 
	9, 1, 10, 8, 10, 5, 8, 5, 
	9, 7, 7, 11, 11, 12, 11, 11, 
	11, 11, 11, 11, 11, 7, 11, 1, 
	13, 13, 14, 15, 4, 13, 1, 14, 
	14, 2, 16, 14, 1, 17, 18, 1, 
	19, 17, 19, 14, 17, 14, 18, 20, 
	21, 1, 22, 20, 22, 13, 20, 13, 
	21, 23, 24, 1, 25, 23, 25, 7, 
	23, 7, 24, 26, 27, 1, 28, 26, 
	28, 0, 26, 0, 27, 1, 0
};

static const char _JSON_object_trans_targs[] = {
	2, 0, 3, 23, 27, 3, 4, 8, 
	5, 7, 6, 9, 19, 9, 10, 15, 
	11, 12, 14, 13, 16, 18, 17, 20, 
	22, 21, 24, 26, 25
};

static const char _JSON_object_trans_actions[] = {
	0, 0, 3, 0, 5, 0, 0, 0, 
	0, 0, 0, 1, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0
};

static const int JSON_object_start = 1;
static const int JSON_object_first_final = 27;
static const int JSON_object_error = 0;

static const int JSON_object_en_main = 1;


#line 166 "ext/json/ext/parser/parser.rl"


static char *JSON_parse_object(JSON_Parser *json, char *p, char *pe, VALUE *result, int current_nesting)
{
    int cs = EVIL;
    VALUE last_name = Qnil;
    VALUE object_class = json->object_class;

    if (json->max_nesting && current_nesting > json->max_nesting) {
        rb_raise(eNestingError, "nesting of %d is too deep", current_nesting);
    }

    *result = NIL_P(object_class) ? rb_hash_new() : rb_class_new_instance(0, 0, object_class);

    
#line 205 "ext/json/ext/parser/parser.c"
	{
	cs = JSON_object_start;
	}

#line 181 "ext/json/ext/parser/parser.rl"
    
#line 212 "ext/json/ext/parser/parser.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _JSON_object_trans_keys + _JSON_object_key_offsets[cs];
	_trans = _JSON_object_index_offsets[cs];

	_klen = _JSON_object_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _JSON_object_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _JSON_object_indicies[_trans];
	cs = _JSON_object_trans_targs[_trans];

	if ( _JSON_object_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _JSON_object_actions + _JSON_object_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 133 "ext/json/ext/parser/parser.rl"
	{
        VALUE v = Qnil;
        char *np = JSON_parse_value(json, p, pe, &v, current_nesting);
        if (np == NULL) {
            p--; {p++; goto _out; }
        } else {
            if (NIL_P(json->object_class)) {
                rb_hash_aset(*result, last_name, v);
            } else {
                rb_funcall(*result, i_aset, 2, last_name, v);
            }
            {p = (( np))-1;}
        }
    }
	break;
	case 1:
#line 148 "ext/json/ext/parser/parser.rl"
	{
        char *np;
        json->parsing_name = 1;
        np = JSON_parse_string(json, p, pe, &last_name);
        json->parsing_name = 0;
        if (np == NULL) { p--; {p++; goto _out; } } else {p = (( np))-1;}
    }
	break;
	case 2:
#line 156 "ext/json/ext/parser/parser.rl"
	{ p--; {p++; goto _out; } }
	break;
#line 317 "ext/json/ext/parser/parser.c"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 182 "ext/json/ext/parser/parser.rl"

    if (cs >= JSON_object_first_final) {
        if (json->create_additions) {
            VALUE klassname;
            if (NIL_P(json->object_class)) {
              klassname = rb_hash_aref(*result, json->create_id);
            } else {
              klassname = rb_funcall(*result, i_aref, 1, json->create_id);
            }
            if (!NIL_P(klassname)) {
                VALUE klass = rb_funcall(mJSON, i_deep_const_get, 1, klassname);
                if (RTEST(rb_funcall(klass, i_json_creatable_p, 0))) {
                    *result = rb_funcall(klass, i_json_create, 1, *result);
                }
            }
        }
        return p + 1;
    } else {
        return NULL;
    }
}



#line 355 "ext/json/ext/parser/parser.c"
static const char _JSON_value_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5, 1, 6, 1, 
	7, 1, 8, 1, 9
};

static const char _JSON_value_key_offsets[] = {
	0, 0, 16, 18, 19, 21, 22, 24, 
	25, 27, 28, 29, 30, 31, 32, 33, 
	34, 35, 36, 37, 38, 39, 40, 41, 
	42, 43, 44, 45, 46, 47
};

static const char _JSON_value_trans_keys[] = {
	13, 32, 34, 45, 47, 73, 78, 91, 
	102, 110, 116, 123, 9, 10, 48, 57, 
	42, 47, 42, 42, 47, 10, 42, 47, 
	42, 42, 47, 10, 110, 102, 105, 110, 
	105, 116, 121, 97, 78, 97, 108, 115, 
	101, 117, 108, 108, 114, 117, 101, 13, 
	32, 47, 9, 10, 0
};

static const char _JSON_value_single_lengths[] = {
	0, 12, 2, 1, 2, 1, 2, 1, 
	2, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 3
};

static const char _JSON_value_range_lengths[] = {
	0, 2, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 1
};

static const char _JSON_value_index_offsets[] = {
	0, 0, 15, 18, 20, 23, 25, 28, 
	30, 33, 35, 37, 39, 41, 43, 45, 
	47, 49, 51, 53, 55, 57, 59, 61, 
	63, 65, 67, 69, 71, 73
};

static const char _JSON_value_trans_targs[] = {
	1, 1, 29, 29, 6, 10, 17, 29, 
	19, 23, 26, 29, 1, 29, 0, 3, 
	5, 0, 4, 3, 4, 29, 3, 29, 
	5, 7, 9, 0, 8, 7, 8, 1, 
	7, 1, 9, 11, 0, 12, 0, 13, 
	0, 14, 0, 15, 0, 16, 0, 29, 
	0, 18, 0, 29, 0, 20, 0, 21, 
	0, 22, 0, 29, 0, 24, 0, 25, 
	0, 29, 0, 27, 0, 28, 0, 29, 
	0, 29, 29, 2, 29, 0, 0
};

static const char _JSON_value_trans_actions[] = {
	0, 0, 11, 13, 0, 0, 0, 15, 
	0, 0, 0, 17, 0, 13, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 9, 
	0, 0, 0, 7, 0, 0, 0, 0, 
	0, 0, 0, 3, 0, 0, 0, 0, 
	0, 1, 0, 0, 0, 0, 0, 5, 
	0, 0, 0, 0, 0, 0, 0
};

static const char _JSON_value_from_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 19
};

static const int JSON_value_start = 1;
static const int JSON_value_first_final = 29;
static const int JSON_value_error = 0;

static const int JSON_value_en_main = 1;


#line 282 "ext/json/ext/parser/parser.rl"


static char *JSON_parse_value(JSON_Parser *json, char *p, char *pe, VALUE *result, int current_nesting)
{
    int cs = EVIL;

    
#line 448 "ext/json/ext/parser/parser.c"
	{
	cs = JSON_value_start;
	}

#line 289 "ext/json/ext/parser/parser.rl"
    
#line 455 "ext/json/ext/parser/parser.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_acts = _JSON_value_actions + _JSON_value_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 9:
#line 269 "ext/json/ext/parser/parser.rl"
	{ p--; {p++; goto _out; } }
	break;
#line 476 "ext/json/ext/parser/parser.c"
		}
	}

	_keys = _JSON_value_trans_keys + _JSON_value_key_offsets[cs];
	_trans = _JSON_value_index_offsets[cs];

	_klen = _JSON_value_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _JSON_value_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	cs = _JSON_value_trans_targs[_trans];

	if ( _JSON_value_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _JSON_value_actions + _JSON_value_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 211 "ext/json/ext/parser/parser.rl"
	{
        *result = Qnil;
    }
	break;
	case 1:
#line 214 "ext/json/ext/parser/parser.rl"
	{
        *result = Qfalse;
    }
	break;
	case 2:
#line 217 "ext/json/ext/parser/parser.rl"
	{
        *result = Qtrue;
    }
	break;
	case 3:
#line 220 "ext/json/ext/parser/parser.rl"
	{
        if (json->allow_nan) {
            *result = CNaN;
        } else {
            rb_enc_raise(EXC_ENCODING eParserError, "%u: unexpected token at '%s'", __LINE__, p - 2);
        }
    }
	break;
	case 4:
#line 227 "ext/json/ext/parser/parser.rl"
	{
        if (json->allow_nan) {
            *result = CInfinity;
        } else {
            rb_enc_raise(EXC_ENCODING eParserError, "%u: unexpected token at '%s'", __LINE__, p - 8);
        }
    }
	break;
	case 5:
#line 234 "ext/json/ext/parser/parser.rl"
	{
        char *np = JSON_parse_string(json, p, pe, result);
        if (np == NULL) { p--; {p++; goto _out; } } else {p = (( np))-1;}
    }
	break;
	case 6:
#line 239 "ext/json/ext/parser/parser.rl"
	{
        char *np;
        if(pe > p + 8 && !strncmp(MinusInfinity, p, 9)) {
            if (json->allow_nan) {
                *result = CMinusInfinity;
                {p = (( p + 10))-1;}
                p--; {p++; goto _out; }
            } else {
                rb_enc_raise(EXC_ENCODING eParserError, "%u: unexpected token at '%s'", __LINE__, p);
            }
        }
        np = JSON_parse_float(json, p, pe, result);
        if (np != NULL) {p = (( np))-1;}
        np = JSON_parse_integer(json, p, pe, result);
        if (np != NULL) {p = (( np))-1;}
        p--; {p++; goto _out; }
    }
	break;
	case 7:
#line 257 "ext/json/ext/parser/parser.rl"
	{
        char *np;
        np = JSON_parse_array(json, p, pe, result, current_nesting + 1);
        if (np == NULL) { p--; {p++; goto _out; } } else {p = (( np))-1;}
    }
	break;
	case 8:
#line 263 "ext/json/ext/parser/parser.rl"
	{
        char *np;
        np =  JSON_parse_object(json, p, pe, result, current_nesting + 1);
        if (np == NULL) { p--; {p++; goto _out; } } else {p = (( np))-1;}
    }
	break;
#line 621 "ext/json/ext/parser/parser.c"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 290 "ext/json/ext/parser/parser.rl"

    if (cs >= JSON_value_first_final) {
        return p;
    } else {
        return NULL;
    }
}


#line 644 "ext/json/ext/parser/parser.c"
static const char _JSON_integer_actions[] = {
	0, 1, 0
};

static const char _JSON_integer_key_offsets[] = {
	0, 0, 4, 7, 9, 9
};

static const char _JSON_integer_trans_keys[] = {
	45, 48, 49, 57, 48, 49, 57, 48, 
	57, 48, 57, 0
};

static const char _JSON_integer_single_lengths[] = {
	0, 2, 1, 0, 0, 0
};

static const char _JSON_integer_range_lengths[] = {
	0, 1, 1, 1, 0, 1
};

static const char _JSON_integer_index_offsets[] = {
	0, 0, 4, 7, 9, 10
};

static const char _JSON_integer_indicies[] = {
	0, 2, 3, 1, 2, 3, 1, 1, 
	4, 1, 3, 4, 0
};

static const char _JSON_integer_trans_targs[] = {
	2, 0, 3, 5, 4
};

static const char _JSON_integer_trans_actions[] = {
	0, 0, 0, 0, 1
};

static const int JSON_integer_start = 1;
static const int JSON_integer_first_final = 3;
static const int JSON_integer_error = 0;

static const int JSON_integer_en_main = 1;


#line 306 "ext/json/ext/parser/parser.rl"


static char *JSON_parse_integer(JSON_Parser *json, char *p, char *pe, VALUE *result)
{
    int cs = EVIL;

    
#line 698 "ext/json/ext/parser/parser.c"
	{
	cs = JSON_integer_start;
	}

#line 313 "ext/json/ext/parser/parser.rl"
    json->memo = p;
    
#line 706 "ext/json/ext/parser/parser.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _JSON_integer_trans_keys + _JSON_integer_key_offsets[cs];
	_trans = _JSON_integer_index_offsets[cs];

	_klen = _JSON_integer_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _JSON_integer_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _JSON_integer_indicies[_trans];
	cs = _JSON_integer_trans_targs[_trans];

	if ( _JSON_integer_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _JSON_integer_actions + _JSON_integer_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 303 "ext/json/ext/parser/parser.rl"
	{ p--; {p++; goto _out; } }
	break;
#line 784 "ext/json/ext/parser/parser.c"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 315 "ext/json/ext/parser/parser.rl"

    if (cs >= JSON_integer_first_final) {
        long len = p - json->memo;
        fbuffer_clear(json->fbuffer);
        fbuffer_append(json->fbuffer, json->memo, len);
        fbuffer_append_char(json->fbuffer, '\0');
        *result = rb_cstr2inum(FBUFFER_PTR(json->fbuffer), 10);
        return p + 1;
    } else {
        return NULL;
    }
}


#line 812 "ext/json/ext/parser/parser.c"
static const char _JSON_float_actions[] = {
	0, 1, 0
};

static const char _JSON_float_key_offsets[] = {
	0, 0, 4, 7, 10, 12, 16, 18, 
	23, 29, 29
};

static const char _JSON_float_trans_keys[] = {
	45, 48, 49, 57, 48, 49, 57, 46, 
	69, 101, 48, 57, 43, 45, 48, 57, 
	48, 57, 46, 69, 101, 48, 57, 69, 
	101, 45, 46, 48, 57, 69, 101, 45, 
	46, 48, 57, 0
};

static const char _JSON_float_single_lengths[] = {
	0, 2, 1, 3, 0, 2, 0, 3, 
	2, 0, 2
};

static const char _JSON_float_range_lengths[] = {
	0, 1, 1, 0, 1, 1, 1, 1, 
	2, 0, 2
};

static const char _JSON_float_index_offsets[] = {
	0, 0, 4, 7, 11, 13, 17, 19, 
	24, 29, 30
};

static const char _JSON_float_indicies[] = {
	0, 2, 3, 1, 2, 3, 1, 4, 
	5, 5, 1, 6, 1, 7, 7, 8, 
	1, 8, 1, 4, 5, 5, 3, 1, 
	5, 5, 1, 6, 9, 1, 1, 1, 
	1, 8, 9, 0
};

static const char _JSON_float_trans_targs[] = {
	2, 0, 3, 7, 4, 5, 8, 6, 
	10, 9
};

static const char _JSON_float_trans_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 1
};

static const int JSON_float_start = 1;
static const int JSON_float_first_final = 8;
static const int JSON_float_error = 0;

static const int JSON_float_en_main = 1;


#line 340 "ext/json/ext/parser/parser.rl"


static char *JSON_parse_float(JSON_Parser *json, char *p, char *pe, VALUE *result)
{
    int cs = EVIL;

    
#line 878 "ext/json/ext/parser/parser.c"
	{
	cs = JSON_float_start;
	}

#line 347 "ext/json/ext/parser/parser.rl"
    json->memo = p;
    
#line 886 "ext/json/ext/parser/parser.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _JSON_float_trans_keys + _JSON_float_key_offsets[cs];
	_trans = _JSON_float_index_offsets[cs];

	_klen = _JSON_float_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _JSON_float_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _JSON_float_indicies[_trans];
	cs = _JSON_float_trans_targs[_trans];

	if ( _JSON_float_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _JSON_float_actions + _JSON_float_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 334 "ext/json/ext/parser/parser.rl"
	{ p--; {p++; goto _out; } }
	break;
#line 964 "ext/json/ext/parser/parser.c"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 349 "ext/json/ext/parser/parser.rl"

    if (cs >= JSON_float_first_final) {
        long len = p - json->memo;
        fbuffer_clear(json->fbuffer);
        fbuffer_append(json->fbuffer, json->memo, len);
        fbuffer_append_char(json->fbuffer, '\0');
        if (NIL_P(json->decimal_class)) {
          *result = rb_float_new(rb_cstr_to_dbl(FBUFFER_PTR(json->fbuffer), 1));
        }
        else {
          VALUE text;
          text = rb_str_new2(FBUFFER_PTR(json->fbuffer));
          *result = rb_funcall(json->decimal_class, i_new, 1, text);
        }
        return p + 1;
    } else {
        return NULL;
    }
}



#line 1000 "ext/json/ext/parser/parser.c"
static const char _JSON_array_actions[] = {
	0, 1, 0, 1, 1
};

static const char _JSON_array_key_offsets[] = {
	0, 0, 1, 18, 25, 41, 43, 44, 
	46, 47, 49, 50, 52, 53, 55, 56, 
	58, 59
};

static const char _JSON_array_trans_keys[] = {
	91, 13, 32, 34, 45, 47, 73, 78, 
	91, 93, 102, 110, 116, 123, 9, 10, 
	48, 57, 13, 32, 44, 47, 93, 9, 
	10, 13, 32, 34, 45, 47, 73, 78, 
	91, 102, 110, 116, 123, 9, 10, 48, 
	57, 42, 47, 42, 42, 47, 10, 42, 
	47, 42, 42, 47, 10, 42, 47, 42, 
	42, 47, 10, 0
};

static const char _JSON_array_single_lengths[] = {
	0, 1, 13, 5, 12, 2, 1, 2, 
	1, 2, 1, 2, 1, 2, 1, 2, 
	1, 0
};

static const char _JSON_array_range_lengths[] = {
	0, 0, 2, 1, 2, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0
};

static const char _JSON_array_index_offsets[] = {
	0, 0, 2, 18, 25, 40, 43, 45, 
	48, 50, 53, 55, 58, 60, 63, 65, 
	68, 70
};

static const char _JSON_array_indicies[] = {
	0, 1, 0, 0, 2, 2, 3, 2, 
	2, 2, 4, 2, 2, 2, 2, 0, 
	2, 1, 5, 5, 6, 7, 4, 5, 
	1, 6, 6, 2, 2, 8, 2, 2, 
	2, 2, 2, 2, 2, 6, 2, 1, 
	9, 10, 1, 11, 9, 11, 6, 9, 
	6, 10, 12, 13, 1, 14, 12, 14, 
	5, 12, 5, 13, 15, 16, 1, 17, 
	15, 17, 0, 15, 0, 16, 1, 0
};

static const char _JSON_array_trans_targs[] = {
	2, 0, 3, 13, 17, 3, 4, 9, 
	5, 6, 8, 7, 10, 12, 11, 14, 
	16, 15
};

static const char _JSON_array_trans_actions[] = {
	0, 0, 1, 0, 3, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0
};

static const int JSON_array_start = 1;
static const int JSON_array_first_final = 17;
static const int JSON_array_error = 0;

static const int JSON_array_en_main = 1;


#line 399 "ext/json/ext/parser/parser.rl"


static char *JSON_parse_array(JSON_Parser *json, char *p, char *pe, VALUE *result, int current_nesting)
{
    int cs = EVIL;
    VALUE array_class = json->array_class;

    if (json->max_nesting && current_nesting > json->max_nesting) {
        rb_raise(eNestingError, "nesting of %d is too deep", current_nesting);
    }
    *result = NIL_P(array_class) ? rb_ary_new() : rb_class_new_instance(0, 0, array_class);

    
#line 1085 "ext/json/ext/parser/parser.c"
	{
	cs = JSON_array_start;
	}

#line 412 "ext/json/ext/parser/parser.rl"
    
#line 1092 "ext/json/ext/parser/parser.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _JSON_array_trans_keys + _JSON_array_key_offsets[cs];
	_trans = _JSON_array_index_offsets[cs];

	_klen = _JSON_array_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _JSON_array_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _JSON_array_indicies[_trans];
	cs = _JSON_array_trans_targs[_trans];

	if ( _JSON_array_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _JSON_array_actions + _JSON_array_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 376 "ext/json/ext/parser/parser.rl"
	{
        VALUE v = Qnil;
        char *np = JSON_parse_value(json, p, pe, &v, current_nesting);
        if (np == NULL) {
            p--; {p++; goto _out; }
        } else {
            if (NIL_P(json->array_class)) {
                rb_ary_push(*result, v);
            } else {
                rb_funcall(*result, i_leftshift, 1, v);
            }
            {p = (( np))-1;}
        }
    }
	break;
	case 1:
#line 391 "ext/json/ext/parser/parser.rl"
	{ p--; {p++; goto _out; } }
	break;
#line 1187 "ext/json/ext/parser/parser.c"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 413 "ext/json/ext/parser/parser.rl"

    if(cs >= JSON_array_first_final) {
        return p + 1;
    } else {
        rb_enc_raise(EXC_ENCODING eParserError, "%u: unexpected token at '%s'", __LINE__, p);
        return NULL;
    }
}

static VALUE json_string_unescape(VALUE result, char *string, char *stringEnd)
{
    char *p = string, *pe = string, *unescape;
    int unescape_len;
    char buf[4];

    while (pe < stringEnd) {
        if (*pe == '\\') {
            unescape = (char *) "?";
            unescape_len = 1;
            if (pe > p) rb_str_buf_cat(result, p, pe - p);
            switch (*++pe) {
                case 'n':
                    unescape = (char *) "\n";
                    break;
                case 'r':
                    unescape = (char *) "\r";
                    break;
                case 't':
                    unescape = (char *) "\t";
                    break;
                case '"':
                    unescape = (char *) "\"";
                    break;
                case '\\':
                    unescape = (char *) "\\";
                    break;
                case 'b':
                    unescape = (char *) "\b";
                    break;
                case 'f':
                    unescape = (char *) "\f";
                    break;
                case 'u':
                    if (pe > stringEnd - 4) {
                        return Qnil;
                    } else {
                        UTF32 ch = unescape_unicode((unsigned char *) ++pe);
                        pe += 3;
                        if (UNI_SUR_HIGH_START == (ch & 0xFC00)) {
                            pe++;
                            if (pe > stringEnd - 6) return Qnil;
                            if (pe[0] == '\\' && pe[1] == 'u') {
                                UTF32 sur = unescape_unicode((unsigned char *) pe + 2);
                                ch = (((ch & 0x3F) << 10) | ((((ch >> 6) & 0xF) + 1) << 16)
                                        | (sur & 0x3FF));
                                pe += 5;
                            } else {
                                unescape = (char *) "?";
                                break;
                            }
                        }
                        unescape_len = convert_UTF32_to_UTF8(buf, ch);
                        unescape = buf;
                    }
                    break;
                default:
                    p = pe;
                    continue;
            }
            rb_str_buf_cat(result, unescape, unescape_len);
            p = ++pe;
        } else {
            pe++;
        }
    }
    rb_str_buf_cat(result, p, pe - p);
    return result;
}


#line 1281 "ext/json/ext/parser/parser.c"
static const char _JSON_string_actions[] = {
	0, 2, 0, 1
};

static const char _JSON_string_key_offsets[] = {
	0, 0, 1, 5, 8, 14, 20, 26, 
	32
};

static const char _JSON_string_trans_keys[] = {
	34, 34, 92, 0, 31, 117, 0, 31, 
	48, 57, 65, 70, 97, 102, 48, 57, 
	65, 70, 97, 102, 48, 57, 65, 70, 
	97, 102, 48, 57, 65, 70, 97, 102, 
	0
};

static const char _JSON_string_single_lengths[] = {
	0, 1, 2, 1, 0, 0, 0, 0, 
	0
};

static const char _JSON_string_range_lengths[] = {
	0, 0, 1, 1, 3, 3, 3, 3, 
	0
};

static const char _JSON_string_index_offsets[] = {
	0, 0, 2, 6, 9, 13, 17, 21, 
	25
};

static const char _JSON_string_indicies[] = {
	0, 1, 2, 3, 1, 0, 4, 1, 
	0, 5, 5, 5, 1, 6, 6, 6, 
	1, 7, 7, 7, 1, 0, 0, 0, 
	1, 1, 0
};

static const char _JSON_string_trans_targs[] = {
	2, 0, 8, 3, 4, 5, 6, 7
};

static const char _JSON_string_trans_actions[] = {
	0, 0, 1, 0, 0, 0, 0, 0
};

static const int JSON_string_start = 1;
static const int JSON_string_first_final = 8;
static const int JSON_string_error = 0;

static const int JSON_string_en_main = 1;


#line 512 "ext/json/ext/parser/parser.rl"


static int
match_i(VALUE regexp, VALUE klass, VALUE memo)
{
    if (regexp == Qundef) return ST_STOP;
    if (RTEST(rb_funcall(klass, i_json_creatable_p, 0)) &&
      RTEST(rb_funcall(regexp, i_match, 1, rb_ary_entry(memo, 0)))) {
        rb_ary_push(memo, klass);
        return ST_STOP;
    }
    return ST_CONTINUE;
}

static char *JSON_parse_string(JSON_Parser *json, char *p, char *pe, VALUE *result)
{
    int cs = EVIL;
    VALUE match_string;

    *result = rb_str_buf_new(0);
    
#line 1358 "ext/json/ext/parser/parser.c"
	{
	cs = JSON_string_start;
	}

#line 533 "ext/json/ext/parser/parser.rl"
    json->memo = p;
    
#line 1366 "ext/json/ext/parser/parser.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _JSON_string_trans_keys + _JSON_string_key_offsets[cs];
	_trans = _JSON_string_index_offsets[cs];

	_klen = _JSON_string_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _JSON_string_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _JSON_string_indicies[_trans];
	cs = _JSON_string_trans_targs[_trans];

	if ( _JSON_string_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _JSON_string_actions + _JSON_string_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 498 "ext/json/ext/parser/parser.rl"
	{
        *result = json_string_unescape(*result, json->memo + 1, p);
        if (NIL_P(*result)) {
            p--;
            {p++; goto _out; }
        } else {
            FORCE_UTF8(*result);
            {p = (( p + 1))-1;}
        }
    }
	break;
	case 1:
#line 509 "ext/json/ext/parser/parser.rl"
	{ p--; {p++; goto _out; } }
	break;
#line 1457 "ext/json/ext/parser/parser.c"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 535 "ext/json/ext/parser/parser.rl"

    if (json->create_additions && RTEST(match_string = json->match_string)) {
          VALUE klass;
          VALUE memo = rb_ary_new2(2);
          rb_ary_push(memo, *result);
          rb_hash_foreach(match_string, match_i, memo);
          klass = rb_ary_entry(memo, 1);
          if (RTEST(klass)) {
              *result = rb_funcall(klass, i_json_create, 1, *result);
          }
    }

    if (json->symbolize_names && json->parsing_name) {
      *result = rb_str_intern(*result);
    } else {
      rb_str_resize(*result, RSTRING_LEN(*result));
    }
    if (cs >= JSON_string_first_final) {
        return p + 1;
    } else {
        return NULL;
    }
}

/*
 * Document-class: JSON::Ext::Parser
 *
 * This is the JSON parser implemented as a C extension. It can be configured
 * to be used by setting
 *
 *  JSON.parser = JSON::Ext::Parser
 *
 * with the method parser= in JSON.
 *
 */

static VALUE convert_encoding(VALUE source)
{
#ifdef HAVE_RUBY_ENCODING_H
  rb_encoding *enc = rb_enc_get(source);
  if (enc == rb_ascii8bit_encoding()) {
    if (OBJ_FROZEN(source)) {
      source = rb_str_dup(source);
    }
    FORCE_UTF8(source);
  } else {
    source = rb_str_conv_enc(source, NULL, rb_utf8_encoding());
  }
#endif
    return source;
}

/*
 * call-seq: new(source, opts => {})
 *
 * Creates a new JSON::Ext::Parser instance for the string _source_.
 *
 * Creates a new JSON::Ext::Parser instance for the string _source_.
 *
 * It will be configured by the _opts_ hash. _opts_ can have the following
 * keys:
 *
 * _opts_ can have the following keys:
 * * *max_nesting*: The maximum depth of nesting allowed in the parsed data
 *   structures. Disable depth checking with :max_nesting => false|nil|0, it
 *   defaults to 100.
 * * *allow_nan*: If set to true, allow NaN, Infinity and -Infinity in
 *   defiance of RFC 4627 to be parsed by the Parser. This option defaults to
 *   false.
 * * *symbolize_names*: If set to true, returns symbols for the names
 *   (keys) in a JSON object. Otherwise strings are returned, which is
 *   also the default. It's not possible to use this option in
 *   conjunction with the *create_additions* option.
 * * *create_additions*: If set to false, the Parser doesn't create
 *   additions even if a matching class and create_id was found. This option
 *   defaults to false.
 * * *object_class*: Defaults to Hash
 * * *array_class*: Defaults to Array
 */
static VALUE cParser_initialize(int argc, VALUE *argv, VALUE self)
{
    VALUE source, opts;
    GET_PARSER_INIT;

    if (json->Vsource) {
        rb_raise(rb_eTypeError, "already initialized instance");
    }
#ifdef HAVE_RB_SCAN_ARGS_OPTIONAL_HASH
    rb_scan_args(argc, argv, "1:", &source, &opts);
#else
    rb_scan_args(argc, argv, "11", &source, &opts);
#endif
    if (!NIL_P(opts)) {
#ifndef HAVE_RB_SCAN_ARGS_OPTIONAL_HASH
        opts = rb_convert_type(opts, T_HASH, "Hash", "to_hash");
        if (NIL_P(opts)) {
            rb_raise(rb_eArgError, "opts needs to be like a hash");
        } else {
#endif
            VALUE tmp = ID2SYM(i_max_nesting);
            if (option_given_p(opts, tmp)) {
                VALUE max_nesting = rb_hash_aref(opts, tmp);
                if (RTEST(max_nesting)) {
                    Check_Type(max_nesting, T_FIXNUM);
                    json->max_nesting = FIX2INT(max_nesting);
                } else {
                    json->max_nesting = 0;
                }
            } else {
                json->max_nesting = 100;
            }
            tmp = ID2SYM(i_allow_nan);
            if (option_given_p(opts, tmp)) {
                json->allow_nan = RTEST(rb_hash_aref(opts, tmp)) ? 1 : 0;
            } else {
                json->allow_nan = 0;
            }
            tmp = ID2SYM(i_symbolize_names);
            if (option_given_p(opts, tmp)) {
                json->symbolize_names = RTEST(rb_hash_aref(opts, tmp)) ? 1 : 0;
            } else {
                json->symbolize_names = 0;
            }
            tmp = ID2SYM(i_create_additions);
            if (option_given_p(opts, tmp)) {
                json->create_additions = RTEST(rb_hash_aref(opts, tmp));
            } else {
                json->create_additions = 0;
            }
            if (json->symbolize_names && json->create_additions) {
              rb_raise(rb_eArgError,
                "options :symbolize_names and :create_additions cannot be "
                " used in conjunction");
            }
            tmp = ID2SYM(i_create_id);
            if (option_given_p(opts, tmp)) {
                json->create_id = rb_hash_aref(opts, tmp);
            } else {
                json->create_id = rb_funcall(mJSON, i_create_id, 0);
            }
            tmp = ID2SYM(i_object_class);
            if (option_given_p(opts, tmp)) {
                json->object_class = rb_hash_aref(opts, tmp);
            } else {
                json->object_class = Qnil;
            }
            tmp = ID2SYM(i_array_class);
            if (option_given_p(opts, tmp)) {
                json->array_class = rb_hash_aref(opts, tmp);
            } else {
                json->array_class = Qnil;
            }
            tmp = ID2SYM(i_decimal_class);
            if (option_given_p(opts, tmp)) {
                json->decimal_class = rb_hash_aref(opts, tmp);
            } else {
                json->decimal_class = Qnil;
            }
            tmp = ID2SYM(i_match_string);
            if (option_given_p(opts, tmp)) {
                VALUE match_string = rb_hash_aref(opts, tmp);
                json->match_string = RTEST(match_string) ? match_string : Qnil;
            } else {
                json->match_string = Qnil;
            }
#ifndef HAVE_RB_SCAN_ARGS_OPTIONAL_HASH
        }
#endif
    } else {
        json->max_nesting = 100;
        json->allow_nan = 0;
        json->create_additions = 1;
        json->create_id = rb_funcall(mJSON, i_create_id, 0);
        json->object_class = Qnil;
        json->array_class = Qnil;
        json->decimal_class = Qnil;
    }
    source = convert_encoding(StringValue(source));
    StringValue(source);
    json->len = RSTRING_LEN(source);
    json->source = RSTRING_PTR(source);;
    json->Vsource = source;
    return self;
}


#line 1657 "ext/json/ext/parser/parser.c"
static const char _JSON_actions[] = {
	0, 1, 0
};

static const char _JSON_key_offsets[] = {
	0, 0, 16, 18, 19, 21, 22, 24, 
	25, 27, 28
};

static const char _JSON_trans_keys[] = {
	13, 32, 34, 45, 47, 73, 78, 91, 
	102, 110, 116, 123, 9, 10, 48, 57, 
	42, 47, 42, 42, 47, 10, 42, 47, 
	42, 42, 47, 10, 13, 32, 47, 9, 
	10, 0
};

static const char _JSON_single_lengths[] = {
	0, 12, 2, 1, 2, 1, 2, 1, 
	2, 1, 3
};

static const char _JSON_range_lengths[] = {
	0, 2, 0, 0, 0, 0, 0, 0, 
	0, 0, 1
};

static const char _JSON_index_offsets[] = {
	0, 0, 15, 18, 20, 23, 25, 28, 
	30, 33, 35
};

static const char _JSON_indicies[] = {
	0, 0, 2, 2, 3, 2, 2, 2, 
	2, 2, 2, 2, 0, 2, 1, 4, 
	5, 1, 6, 4, 6, 7, 4, 7, 
	5, 8, 9, 1, 10, 8, 10, 0, 
	8, 0, 9, 7, 7, 11, 7, 1, 
	0
};

static const char _JSON_trans_targs[] = {
	1, 0, 10, 6, 3, 5, 4, 10, 
	7, 9, 8, 2
};

static const char _JSON_trans_actions[] = {
	0, 0, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0
};

static const int JSON_start = 1;
static const int JSON_first_final = 10;
static const int JSON_error = 0;

static const int JSON_en_main = 1;


#line 735 "ext/json/ext/parser/parser.rl"


/*
 * call-seq: parse()
 *
 *  Parses the current JSON text _source_ and returns the complete data
 *  structure as a result.
 */
static VALUE cParser_parse(VALUE self)
{
  char *p, *pe;
  int cs = EVIL;
  VALUE result = Qnil;
  GET_PARSER;

  
#line 1733 "ext/json/ext/parser/parser.c"
	{
	cs = JSON_start;
	}

#line 751 "ext/json/ext/parser/parser.rl"
  p = json->source;
  pe = p + json->len;
  
#line 1742 "ext/json/ext/parser/parser.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _JSON_trans_keys + _JSON_key_offsets[cs];
	_trans = _JSON_index_offsets[cs];

	_klen = _JSON_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _JSON_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _JSON_indicies[_trans];
	cs = _JSON_trans_targs[_trans];

	if ( _JSON_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _JSON_actions + _JSON_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 727 "ext/json/ext/parser/parser.rl"
	{
        char *np = JSON_parse_value(json, p, pe, &result, 0);
        if (np == NULL) { p--; {p++; goto _out; } } else {p = (( np))-1;}
    }
	break;
#line 1823 "ext/json/ext/parser/parser.c"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 754 "ext/json/ext/parser/parser.rl"

  if (cs >= JSON_first_final && p == pe) {
    return result;
  } else {
    rb_enc_raise(EXC_ENCODING eParserError, "%u: unexpected token at '%s'", __LINE__, p);
    return Qnil;
  }
}

static void JSON_mark(void *ptr)
{
    JSON_Parser *json = ptr;
    rb_gc_mark_maybe(json->Vsource);
    rb_gc_mark_maybe(json->create_id);
    rb_gc_mark_maybe(json->object_class);
    rb_gc_mark_maybe(json->array_class);
    rb_gc_mark_maybe(json->decimal_class);
    rb_gc_mark_maybe(json->match_string);
}

static void JSON_free(void *ptr)
{
    JSON_Parser *json = ptr;
    fbuffer_free(json->fbuffer);
    ruby_xfree(json);
}

static size_t JSON_memsize(const void *ptr)
{
    const JSON_Parser *json = ptr;
    return sizeof(*json) + FBUFFER_CAPA(json->fbuffer);
}

#ifdef NEW_TYPEDDATA_WRAPPER
static const rb_data_type_t JSON_Parser_type = {
    "JSON/Parser",
    {JSON_mark, JSON_free, JSON_memsize,},
#ifdef RUBY_TYPED_FREE_IMMEDIATELY
    0, 0,
    RUBY_TYPED_FREE_IMMEDIATELY,
#endif
};
#endif

static VALUE cJSON_parser_s_allocate(VALUE klass)
{
    JSON_Parser *json;
    VALUE obj = TypedData_Make_Struct(klass, JSON_Parser, &JSON_Parser_type, json);
    json->fbuffer = fbuffer_alloc(0);
    return obj;
}

/*
 * call-seq: source()
 *
 * Returns a copy of the current _source_ string, that was used to construct
 * this Parser.
 */
static VALUE cParser_source(VALUE self)
{
    GET_PARSER;
    return rb_str_dup(json->Vsource);
}

void Init_parser(void)
{
    rb_require("json/common");
    mJSON = rb_define_module("JSON");
    mExt = rb_define_module_under(mJSON, "Ext");
    cParser = rb_define_class_under(mExt, "Parser", rb_cObject);
    eParserError = rb_path2class("JSON::ParserError");
    eNestingError = rb_path2class("JSON::NestingError");
    rb_define_alloc_func(cParser, cJSON_parser_s_allocate);
    rb_define_method(cParser, "initialize", cParser_initialize, -1);
    rb_define_method(cParser, "parse", cParser_parse, 0);
    rb_define_method(cParser, "source", cParser_source, 0);

    CNaN = rb_const_get(mJSON, rb_intern("NaN"));
    CInfinity = rb_const_get(mJSON, rb_intern("Infinity"));
    CMinusInfinity = rb_const_get(mJSON, rb_intern("MinusInfinity"));

    i_json_creatable_p = rb_intern("json_creatable?");
    i_json_create = rb_intern("json_create");
    i_create_id = rb_intern("create_id");
    i_create_additions = rb_intern("create_additions");
    i_chr = rb_intern("chr");
    i_max_nesting = rb_intern("max_nesting");
    i_allow_nan = rb_intern("allow_nan");
    i_symbolize_names = rb_intern("symbolize_names");
    i_object_class = rb_intern("object_class");
    i_array_class = rb_intern("array_class");
    i_decimal_class = rb_intern("decimal_class");
    i_match = rb_intern("match");
    i_match_string = rb_intern("match_string");
    i_key_p = rb_intern("key?");
    i_deep_const_get = rb_intern("deep_const_get");
    i_aset = rb_intern("[]=");
    i_aref = rb_intern("[]");
    i_leftshift = rb_intern("<<");
}

/*
 * Local variables:
 * mode: c
 * c-file-style: ruby
 * indent-tabs-mode: nil
 * End:
 */
