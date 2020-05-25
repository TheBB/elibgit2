#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "emacs-module.h"
#include "interface.h"


void em_init(emacs_env *env)
{
    esyms_init(env);

    em_define_error(env, esym_wrong_value_argument, "Wrong argument value passed", esym_nil);
    em_define_error(env, esym_giterr, "Git error", esym_nil);
    em_define_error(env, esym_giterr_nomemory, "Git error: out of memory", esym_giterr);
    em_define_error(env, esym_giterr_os, "Git error: OS", esym_giterr);
    em_define_error(env, esym_giterr_invalid, "Git error: invalid", esym_giterr);
    em_define_error(env, esym_giterr_reference, "Git error: reference", esym_giterr);
    em_define_error(env, esym_giterr_zlib, "Git error: zlib", esym_giterr);
    em_define_error(env, esym_giterr_repository, "Git error: repository", esym_giterr);
    em_define_error(env, esym_giterr_config, "Git error: config", esym_giterr);
    em_define_error(env, esym_giterr_regex, "Git error: regex", esym_giterr);
    em_define_error(env, esym_giterr_odb, "Git error: ODB", esym_giterr);
    em_define_error(env, esym_giterr_index, "Git error: index", esym_giterr);
    em_define_error(env, esym_giterr_object, "Git error: object", esym_giterr);
    em_define_error(env, esym_giterr_net, "Git error: net", esym_giterr);
    em_define_error(env, esym_giterr_tag, "Git error: tag", esym_giterr);
    em_define_error(env, esym_giterr_tree, "Git error: tree", esym_giterr);
    em_define_error(env, esym_giterr_indexer, "Git error: indexer", esym_giterr);
    em_define_error(env, esym_giterr_ssl, "Git error: SSL", esym_giterr);
    em_define_error(env, esym_giterr_submodule, "Git error: submodule", esym_giterr);
    em_define_error(env, esym_giterr_thread, "Git error: thread", esym_giterr);
    em_define_error(env, esym_giterr_stash, "Git error: stash", esym_giterr);
    em_define_error(env, esym_giterr_checkout, "Git error: checkout", esym_giterr);
    em_define_error(env, esym_giterr_fetchhead, "Git error: fetch-head", esym_giterr);
    em_define_error(env, esym_giterr_merge, "Git error: merge", esym_giterr);
    em_define_error(env, esym_giterr_ssh, "Git error: SSH", esym_giterr);
    em_define_error(env, esym_giterr_filter, "Git error: filter", esym_giterr);
    em_define_error(env, esym_giterr_revert, "Git error: revert", esym_giterr);
    em_define_error(env, esym_giterr_callback, "Git error: callback", esym_giterr);
    em_define_error(env, esym_giterr_cherrypick, "Git error: cherry-pick", esym_giterr);
    em_define_error(env, esym_giterr_describe, "Git error: describe", esym_giterr);
    em_define_error(env, esym_giterr_rebase, "Git error: rebase", esym_giterr);
    em_define_error(env, esym_giterr_filesystem, "Git error: filesystem", esym_giterr);
    em_define_error(env, esym_giterr_patch, "Git error: patch", esym_giterr);
    em_define_error(env, esym_giterr_worktree, "Git error: worktree", esym_giterr);
    em_define_error(env, esym_giterr_sha1, "Git error: SHA-1", esym_giterr);
}

/**
 * Call an Emacs function without error checking.
 * @param env The active Emacs environment.
 * @param func The function to call.
 * @param nargs The number of arguments that follow.
 * @return The function return value.
 */
static emacs_value em_funcall(emacs_env *env, emacs_value func, ptrdiff_t nargs, ...)
{
    emacs_value args[nargs];

    va_list vargs;
    va_start(vargs, nargs);
    for (ptrdiff_t i = 0; i < nargs; i++)
        args[i] = va_arg(vargs, emacs_value);
    va_end(vargs);

    return env->funcall(env, func, nargs, args);
}

bool em_assert(emacs_env *env, emacs_value predicate, emacs_value arg)
{
    bool cond = EM_EXTRACT_BOOLEAN(em_funcall(env, predicate, 1, arg));
    if (!cond)
        em_signal_wrong_type(env, predicate, arg);
    return cond;
}

ptrdiff_t em_assert_list(emacs_env *env, emacs_value predicate, emacs_value arg)
{
    ptrdiff_t nelems = 0;
    bool predp = EM_EXTRACT_BOOLEAN(predicate);

    while (em_consp(env, arg)) {
        emacs_value car = em_car(env, arg);
        if (predp && !em_assert(env, predicate, car))
            return -1;
        nelems++;
        arg = em_cdr(env, arg);
    }

    if (EM_EXTRACT_BOOLEAN(arg)) {
        em_signal_wrong_type(env, esym_listp, arg);
        return -1;
    }

    return nelems;
}

void em_signal(emacs_env *env, emacs_value error, const char *_msg)
{
    emacs_value msg = EM_STRING(_msg);
    env->non_local_exit_signal(env, error, em_cons(env, msg, esym_nil));
}

void em_signal_wrong_type(emacs_env *env, emacs_value expected, emacs_value actual)
{
    env->non_local_exit_signal(
        env, esym_wrong_type_argument,
        em_cons(env, expected, em_cons(env, actual, esym_nil))
    );
}

void em_signal_wrong_value(emacs_env *env, emacs_value actual)
{
    env->non_local_exit_signal(env, esym_wrong_value_argument, actual);
}

void em_signal_args_out_of_range(emacs_env *env, intmax_t index)
{
    env->non_local_exit_signal(env, esym_args_out_of_range, EM_INTEGER(index));
}

char *em_get_string_with_size(emacs_env *env, emacs_value arg, ptrdiff_t *size)
{
    env->copy_string_contents(env, arg, NULL, size);

    char *buf = (char*) malloc((*size) * sizeof(char));
    env->copy_string_contents(env, arg, buf, size);

    (*size)--;
    return buf;
}

char *em_get_string(emacs_env *env, emacs_value arg)
{
    ptrdiff_t size;
    return em_get_string_with_size(env, arg, &size);
}

emacs_value em_cons(emacs_env *env, emacs_value car, emacs_value cdr)
{
    return em_funcall(env, esym_cons, 2, car, cdr);
}

bool em_consp(emacs_env *env, emacs_value cell)
{
    return EM_EXTRACT_BOOLEAN(em_funcall(env, esym_consp, 1, cell));
}

emacs_value em_car(emacs_env *env, emacs_value cell)
{
    return em_funcall(env, esym_car, 1, cell);
}

emacs_value em_cdr(emacs_env *env, emacs_value cell)
{
    return em_funcall(env, esym_cdr, 1, cell);
}

emacs_value em_list(emacs_env *env, emacs_value *objects, ptrdiff_t nobjects)
{
    return env->funcall(env, esym_list, nobjects, objects);
}

bool em_listp(emacs_env *env, emacs_value object)
{
    return EM_EXTRACT_BOOLEAN(em_funcall(env, esym_listp, 1, object));
}

ptrdiff_t em_length(emacs_env *env, emacs_value sequence)
{
    emacs_value result = em_funcall(env, esym_length, 1, sequence);
    ptrdiff_t length = env->extract_integer(env, result);
    EM_RETURN_IF_NLE(-1);
    return length;
}

emacs_value em_assq(emacs_env *env, emacs_value key, emacs_value list)
{
    return em_funcall(env, esym_assq, 2, key, list);
}

void em_define_error(emacs_env *env, emacs_value symbol, const char *msg, emacs_value parent)
{
    em_funcall(env, esym_define_error, 3, symbol, EM_STRING(msg), parent);
}

void em_defun(emacs_env *env, const char *name, emacs_value func)
{
    em_funcall(env, esym_defalias, 2, env->intern(env, name), func);
}

emacs_value em_expand_file_name(emacs_env *env, emacs_value path)
{
    return em_funcall(env, esym_expand_file_name, 1, path);
}

void em_provide(emacs_env *env, const char *feature)
{
    em_funcall(env, esym_provide, 1, env->intern(env, feature));
}

bool em_user_ptrp(emacs_env *env, emacs_value val)
{
    return EM_EXTRACT_BOOLEAN(em_funcall(env, esym_user_ptrp, 1, val));
}

char *em_default_directory(emacs_env *env)
{
    emacs_value dir = em_funcall(env, esym_symbol_value, 1, esym_default_directory);
    dir = em_expand_file_name(env, dir);
    return em_get_string(env, dir);
}

emacs_value em_decode_time(emacs_env *env, intmax_t timestamp, intmax_t offset)
{
    return em_funcall(env, esym_decode_time, 2,
                      EM_INTEGER(timestamp),
                      EM_INTEGER(offset));
}

bool em_encode_time(emacs_env *env, emacs_value time, intmax_t *timestamp, intmax_t *offset)
{
    emacs_value encoded = em_funcall(env, esym_apply, 2, esym_encode_time, time);
    EM_RETURN_IF_NLE(false);

    emacs_value high = em_car(env, encoded);
    *timestamp = EM_EXTRACT_INTEGER(high) << 16;

    encoded = em_cdr(env, encoded);
    emacs_value low = em_car(env, encoded);
    *timestamp += EM_EXTRACT_INTEGER(low);

    emacs_value last = em_funcall(env, esym_last, 1, time);
    if (!em_consp(env, last)) {
        em_signal_wrong_type(env, last, esym_consp);
        return false;
    }
    emacs_value zone = em_car(env, last);
    if (!EM_EXTRACT_BOOLEAN(em_funcall(env, esym_integerp, 1, zone))) {
        em_signal_wrong_type(env, zone, esym_integerp);
        return false;
    }
    *offset = EM_EXTRACT_INTEGER(zone);

    return true;
}

void em_insert(emacs_env *env, const char *ptr, size_t length)
{
    em_funcall(env, esym_insert, 1, env->make_string(env, ptr, length));
}

emacs_value em_string_as_unibyte(emacs_env *env, emacs_value str)
{
    return em_funcall(env, esym_string_as_unibyte, 1, str);
}


// =============================================================================
// Symbol <-> enum map functions

static bool em_findsym(esym_enumval *out, emacs_env *env, emacs_value value, esym_map *map, bool required)
{
    while (map->symbol != NULL) {
        if (EM_EQ(*map->symbol, value)) {
            *out = map->value;
            return true;
        }
        map++;
    }

    if (required)
        em_signal_wrong_value(env, value);
    return false;
}

#define MKFINDSYM(type, map)                                            \
    bool em_findsym_##map(                                              \
        type *out, emacs_env *env, emacs_value value, bool required)    \
    {                                                                   \
        esym_enumval val = {0};                                         \
        bool retval = em_findsym(                                       \
            &val, env, value, esym_##map##_map, required);              \
        *out = val.map;                                                 \
        return retval;                                                  \
    }

MKFINDSYM(git_branch_t, branch);
MKFINDSYM(git_checkout_strategy_t, checkout_strategy);
MKFINDSYM(git_config_level_t, config_level);
MKFINDSYM(git_describe_strategy_t, describe_strategy);
MKFINDSYM(git_delta_t, delta);
MKFINDSYM(git_diff_format_t, diff_format);
MKFINDSYM(git_fetch_prune_t, fetch_prune);
MKFINDSYM(git_filemode_t, filemode);
MKFINDSYM(git_merge_file_favor_t, merge_file_favor);
MKFINDSYM(git_otype, otype);
MKFINDSYM(git_pathspec_flag_t, pathspec_flag);
MKFINDSYM(git_proxy_t, proxy);
MKFINDSYM(git_remote_autotag_option_t, remote_autotag_option);
MKFINDSYM(git_reset_t, reset);
MKFINDSYM(git_submodule_ignore_t, submodule_ignore);
MKFINDSYM(git_submodule_recurse_t, submodule_recurse);
MKFINDSYM(git_submodule_update_t, submodule_update);
MKFINDSYM(int, stage);
MKFINDSYM(git_status_show_t, status_show);
MKFINDSYM(git_diff_find_t, diff_find);

#undef MKFINDSYM

#define MKFINDENUM(type, mapname)                                       \
    emacs_value em_findenum_##mapname(type value)                       \
    {                                                                   \
        esym_map *map = esym_##mapname##_map;                           \
        while (map->symbol != NULL) {                                   \
            if (map->value.mapname == value)                            \
                return *map->symbol;                                    \
            map++;                                                      \
        }                                                               \
        return esym_nil;                                                \
    }

MKFINDENUM(git_checkout_notify_t, checkout_notify);
MKFINDENUM(git_delta_t, delta);
MKFINDENUM(git_direction, direction);
MKFINDENUM(git_error_t, error);
MKFINDENUM(git_filemode_t, filemode);
MKFINDENUM(git_merge_preference_t, merge_preference);
MKFINDENUM(git_otype, otype);
MKFINDENUM(git_submodule_ignore_t, submodule_ignore);
MKFINDENUM(git_submodule_update_t, submodule_update);
MKFINDENUM(git_submodule_recurse_t, submodule_recurse);
MKFINDENUM(git_remote_autotag_option_t, remote_autotag_option);
MKFINDENUM(git_repository_state_t, repository_state);
MKFINDENUM(int, stage);

#undef MKFINDENUM

#define MKSETFLAG(type, map)                                            \
    bool em_setflag_##map(                                              \
        void *out, emacs_env *env, emacs_value value,                   \
        bool on, bool required)                                         \
    {                                                                   \
        esym_enumval val;                                               \
        if (!em_findsym(&val, env, value, esym_##map##_map, required))  \
            return false;                                               \
        if (on)                                                         \
            *((type*) out) |= val.map;                                  \
        else                                                            \
            *((type*) out) &= ~(val.map);                               \
        return true;                                                    \
    }

MKSETFLAG(git_checkout_notify_t, checkout_notify);
MKSETFLAG(git_diff_option_t, diff_option);
MKSETFLAG(git_index_add_option_t, index_add_option);
MKSETFLAG(git_merge_file_flag_t, merge_file_flag);
MKSETFLAG(git_merge_flag_t, merge_flag);
MKSETFLAG(git_sort_t, sort);
MKSETFLAG(git_stash_flags, stash_flags);
MKSETFLAG(git_status_opt_t, status_opt);

#undef MKSETFLAG

bool em_setflags_list(void *out, emacs_env *env, emacs_value list,
                      bool required, setter *setter)
{
    while (em_consp(env, list)) {
        emacs_value car = em_car(env, list);
        if (!setter(out, env, car, true, required)) {
            if (required) return false;
        }
        list = em_cdr(env, list);
    }
    if (EM_EXTRACT_BOOLEAN(list)) {
        em_signal_wrong_type(env, esym_consp, list);
        return false;
    }
    return true;
}

bool em_setflags_alist(void *out, emacs_env *env, emacs_value alist,
                       bool required, setter *setter)
{
    while (em_consp(env, alist)) {
        emacs_value cons = em_car(env, alist);
        if (!em_assert(env, esym_consp, cons))
            return false;
        emacs_value car = em_car(env, cons);
        emacs_value cdr = em_cdr(env, cons);
        if (!setter(out, env, car, EM_EXTRACT_BOOLEAN(cdr), required)) {
            if (required) return false;
        }
        alist = em_cdr(env, alist);
    }
    if (EM_EXTRACT_BOOLEAN(alist)) {
        em_signal_wrong_type(env, esym_consp, alist);
        return false;
    }
    return true;
}

#define MKGETLIST(type, mapname)                                        \
    emacs_value em_getlist_##mapname(emacs_env *env, type value)        \
    {                                                                   \
        esym_map *map = esym_##mapname##_map;                           \
        emacs_value ret = esym_nil;                                     \
        ptrdiff_t nen = 0;                                              \
        for (; map[nen].symbol != NULL; nen++);                         \
        while ((nen--) > 0) {                                           \
            if (map[nen].value.mapname & value)                         \
                ret = em_cons(env, *map[nen].symbol, ret);              \
        }                                                               \
        return ret;                                                     \
    }

MKGETLIST(git_credtype_t, credtype);
MKGETLIST(int, indexcap);
MKGETLIST(git_merge_analysis_t, merge_analysis);
MKGETLIST(git_status_t, status);
MKGETLIST(git_submodule_status_t, submodule_status);

#undef MKGETLIST

#define MKCHECKFLAG(type, map)                                          \
    bool em_checkflag_##map(                                            \
        emacs_value *out, emacs_env *env, emacs_value symbol,           \
        type value, bool required)                                      \
    {                                                                   \
        esym_enumval val;                                               \
        if (!em_findsym(&val, env, symbol, esym_##map##_map, required)) \
            return false;                                               \
        *out = (val.map & value) ? esym_t : esym_nil;                   \
        return true;                                                    \
    }

MKCHECKFLAG(git_feature_t, feature);
MKCHECKFLAG(git_submodule_status_t, submodule_status);

#undef MKCHECKFLAG
