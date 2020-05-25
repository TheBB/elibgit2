#include <stdarg.h>
#include <string.h>

#include "emacs-module.h"
#include "symbols.h"

#ifndef INTERFACE_H
#define INTERFACE_H

// Assert that VAL is a cons cell, signal an error and return otherwise.
#define EM_ASSERT_CONS(val)                                             \
    do { if (!em_assert(env, esym_consp, (val))) return esym_nil; } while (0)

// Assert that VAL is a function, signal an error and return otherwise.
#define EM_ASSERT_FUNCTION(val)                                         \
    do { if (!em_assert(env, esym_functionp, (val))) return esym_nil; } while (0)

// Assert that VAL is a string, signal an error and return otherwise.
#define EM_ASSERT_STRING(val)                                           \
    do { if (!em_assert(env, esym_stringp, (val))) return esym_nil; } while (0)

// Assert that VAL is a string or nil, signal an error and return otherwise.
#define EM_ASSERT_STRING_OR_NIL(val)                                    \
    do { if (EM_EXTRACT_BOOLEAN(val)) EM_ASSERT_STRING(val); } while (0)

// Assert that VAL is an integer, signal an error and return otherwise.
#define EM_ASSERT_INTEGER(val)                                          \
    do { if (!em_assert(env, esym_integerp, (val))) return esym_nil; } while (0)

// Assert that VAL is an integer or nil, signal an error and return otherwise.
#define EM_ASSERT_INTEGER_OR_NIL(val)                                   \
    do { if (EM_EXTRACT_BOOLEAN(val)) EM_ASSERT_INTEGER(val); } while (0)

// Assert that VAL is an user-ptr, signal an error and return otherwise.
#define EM_ASSERT_USER_PTR(val)                                         \
    do { if (!em_assert(env, esym_user_ptrp, (val))) return esym_nil; } while (0)

// Normalize an emacs_value string path. This macro may return.
#define EM_NORMALIZE_PATH(val)                                  \
    do {                                                        \
        (val) = em_expand_file_name(env, val);                  \
        EM_RETURN_NIL_IF_NLE();                                 \
    } while (0)

// Extract a boolean from an emacs_value.
#define EM_EXTRACT_BOOLEAN(val) (env->is_not_nil(env, (val)) ? 1 : 0)

// Extract a string from an emacs_value.
// Caller is responsible for ensuring that the emacs_value represents a string.
#define EM_EXTRACT_STRING(val) em_get_string(env, (val));

// Extract an integer from an emacs_value.
// Caller is responsible for ensuring that the emacs_value represents an integer.
#define EM_EXTRACT_INTEGER(val) env->extract_integer(env, (val))

// Extract an integer from an emacs_value with a default.
// Caller is responsible for ensuring that the emacs_value represents an integer or nil.
#define EM_EXTRACT_INTEGER_OR_DEFAULT(val, default)                     \
    (EM_EXTRACT_BOOLEAN(val) ? EM_EXTRACT_INTEGER(val) : (default))

// Extract a string from an emacs_value, or NULL.
// Caller is responsible for ensuring that the emacs_value represents a string or nil.
#define EM_EXTRACT_STRING_OR_NULL(val)                                  \
    (EM_EXTRACT_BOOLEAN(val) ? em_get_string(env, (val)) : NULL)

// Extract a user pointer from an emacs_value.
// Caller is responsible for ensuring that the emacs_value represents a user pointer.
#define EM_EXTRACT_USER_PTR(val) env->get_user_ptr(env, (val))

// Call (eq a b) in Emacs
#define EM_EQ(a,b) (env->eq(env, (a), (b)))

// Create an Emacs integer
#define EM_INTEGER(val) (env->make_integer(env, (val)))

// Create an Emacs string from a null-terminated char*
#define EM_STRING(val) (env->make_string(env, (val), strlen(val)))

// Create an Emacs user pointer
#define EM_USER_PTR(val, fin) (env->make_user_ptr(env, (fin), (val)))

// Return if a non-local exit is set
#define EM_RETURN_IF_NLE(val)                    \
    do {                                         \
        if (env->non_local_exit_check(env))      \
            return (val);                        \
    } while (0)

#define EM_RETURN_NIL_IF_NLE() EM_RETURN_IF_NLE(esym_nil)

/**
 * Initiate a loop over an Emacs list.
 * If any element is not a cons cell or nil, it WILL signal an error and return nil.
 * @param var Variable bound to each car.
 * @param listvar List to loop over.
 * @param name Unique name identifying the loop.
 */
#define EM_DOLIST(var, listvar, name)                               \
    emacs_value __cell##name = (listvar);                           \
    __loop##name:                                                   \
    if (!EM_EXTRACT_BOOLEAN(__cell##name)) goto __end##name;        \
    if (!em_assert(env, esym_consp, __cell##name)) return esym_nil;    \
    emacs_value (var) = em_car(env, __cell##name)

/**
 * Close a loop over an Emacs lisp.
 * @param name: Unique name identifying the loop.
 */
#define EM_DOLIST_END(name)                     \
    __cell##name = em_cdr(env, __cell##name);   \
    goto __loop##name;                          \
    __end##name:

/**
 * Initialize the libegit2-emacs interface.
 * This function should only be called once.
 */
void em_init(emacs_env *env);

/**
 * Signal a wrong-type-argument error if PREDICATE does not apply to ARG.
 * @param env The active Emacs environment.
 * @param predicate The predicate.
 * @param arg The argument.
 * @return True iff an error was signaled.
 */
bool em_assert(emacs_env *env, emacs_value predicate, emacs_value arg);

/**
 * Signal a wrong-type-argument error if ARG is not a proper list, every
 * element of which satisfies PREDICATE. PREDICATE may be nil, in which
 * case only the list-ness is checked.
 * @param env The active Emacs environment.
 * @param predicate The predicate.
 * @param arg The list.
 * @return The number of elements in the list, or negative if error.
 */
ptrdiff_t em_assert_list(emacs_env *env, emacs_value predicate, emacs_value arg);

/**
 * Signal an error with string message.
 * @param env The active Emacs environment.
 * @param error The error symbol.
 * @param _msg The error message.
 */
void em_signal(emacs_env *env, emacs_value error, const char* _msg);

/**
 * Signal a wrong-type-argument error.
 * @param env The active Emacs environment.
 * @param expected Symbol describing the expected type.
 * @param actual Emacs value that does not have the expected type.
 */
void em_signal_wrong_type(emacs_env *env, emacs_value expected, emacs_value actual);

/**
 * Signal a wrong-value-argument error.
 * @param env The active Emacs environment.
 * @param actual Emacs value that does not have the expected value.
 */
void em_signal_wrong_value(emacs_env *env, emacs_value actual);

/**
 * Signal an args-out-of-range error.
 * @param env The active Emacs environment.
 * @param index The erroneous index.
 */
void em_signal_args_out_of_range(emacs_env *env, intmax_t index);

/**
 * Return a string with size from an emacs_value.
 * Caller is responsible for ensuring that the value is a string, and to free the returned pointer.
 * @param env The active Emacs environment.
 * @param arg Emacs value representing a string.
 * @param size Where the size will be stored.
 * @return The string (owned pointer).
 */
char *em_get_string_with_size(emacs_env *env, emacs_value arg, ptrdiff_t *size);

/**
 * Return a string from an emacs_value.
 * Caller is responsible for ensuring that the value is a string, and to free the returned pointer.
 * @param env The active Emacs environment.
 * @param arg Emacs value representing a string.
 * @return The string (owned pointer).
 */
char *em_get_string(emacs_env *env, emacs_value arg);

/**
 * Call (cons car cdr) in Emacs.
 * @param env The active Emacs environment.
 * @param car The car.
 * @param cdr The cdr.
 * @return The cons cell.
 */
emacs_value em_cons(emacs_env *env, emacs_value car, emacs_value cdr);

/**
 * Call (consp cell) in Emacs.
 * @param env The active Emacs environment.
 * @param cell The cell you're testing.
 * @return True if cell is a cons cell, false otherwise.
 */
bool em_consp(emacs_env *env, emacs_value cell);

/**
 * Call (car cell) in Emacs.
 * @param env The active Emacs environment.
 * @param cell the cell to get the car of.
 * @return the car of the cell or nil.
 */
emacs_value em_car(emacs_env *env, emacs_value cell);

/**
 * Call (cdr cell) in Emacs.
 * @param env The active Emacs environment.
 * @param cell the cell to get the cdr of.
 * @return the cdr of the cell or nil.
 */
emacs_value em_cdr(emacs_env *env, emacs_value cell);

/**
 * Call (list OBJECTS...) in Emacs.
 * @param env The active Emacs environment.
 * @param objects Array of objects.
 * @param nobjects Number of \p objects.
 */
emacs_value em_list(emacs_env *env, emacs_value *objects, ptrdiff_t nobjects);

/**
 * Call (listp OBJECT) in Emacs.
 * @param env The active Emacs environment.
 * @param object An emacs value.
 */
bool em_listp(emacs_env *env, emacs_value object);

/**
 * Call (length SEQUENCE) in Emacs.
 * @param env The active Emacs environment.
 * @param object An emacs sequence.
 * @return Length of the sequence, or -1 on error.
 */
ptrdiff_t em_length(emacs_env *env, emacs_value sequence);

/**
 * Call (assq key list) in Emacs.
 * @param env The active Emacs environment.
 * @param key The key to lookup.
 * @param list The associated list (alist).
 * @return The first cons cell whose car is \p key.
 */
emacs_value em_assq(emacs_env *env, emacs_value key, emacs_value list);

/**
 * Call (define-error SYMBOL MSG) in Emacs.
 * @param env The active Emacs environment.
 * @param symbol The error symbol.
 * @param msg The error description.
 * @param parent The parent error.
 */
void em_define_error(emacs_env *env, emacs_value symbol, const char *msg, emacs_value parent);

/**
 * Define a function in Emacs, using defalias.
 * @param env The active Emacs environment.
 * @param name Symbol name of the desired function.
 * @param func Function to bind.
 */
void em_defun(emacs_env *env, const char *name, emacs_value func);

/**
 * Call (expand-file-name PATH) in Emacs.
 * @param env The active Emacs environment.
 * @param path The path to expand.
 */
emacs_value em_expand_file_name(emacs_env *env, emacs_value path);

/**
 * Provide a feature to Emacs.
 * @param env The active Emacs environment.
 * @param name Symbol name of the feature to provide.
 */
void em_provide(emacs_env *env, const char *feature);

/**
 * Check if a value is a user pointer.
 * @param env The active Emacs environment.
 * @param val Value to check.
 * @return True iff val is a user pointer.
 */
bool em_user_ptrp(emacs_env *env, emacs_value val);

/**
 * Return the value of default-directory in Emacs.
 */
char *em_default_directory(emacs_env *env);

/**
 * Run (decode-time TIMESTAMP OFFSET) in Emacs.
 */
emacs_value em_decode_time(emacs_env *env, intmax_t timestamp, intmax_t offset);

/**
 * Run (encode-time ...) in Emacs.
 */
bool em_encode_time(emacs_env *env, emacs_value time, intmax_t *timestamp, intmax_t *offset);

/**
 * Run (insert ...) in Emacs.
 */
void em_insert(emacs_env *env, const char *ptr, size_t length);

/**
 * Convert an emacs string to unibyte.
 */
emacs_value em_string_as_unibyte(emacs_env *env, emacs_value str);


// =============================================================================
// Symbol <-> enum map functions

bool em_findsym_branch(git_branch_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_checkout_strategy(git_checkout_strategy_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_config_level(git_config_level_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_delta(git_delta_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_diff_format(git_diff_format_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_diff_find(git_diff_find_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_describe_strategy(git_describe_strategy_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_fetch_prune(git_fetch_prune_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_filemode(git_filemode_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_merge_file_favor(git_merge_file_favor_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_otype(git_otype *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_pathspec_flag(git_pathspec_flag_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_proxy(git_proxy_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_remote_autotag_option(git_remote_autotag_option_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_reset(git_reset_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_submodule_ignore(git_submodule_ignore_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_submodule_recurse(git_submodule_recurse_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_submodule_update(git_submodule_update_t *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_stage(int *out, emacs_env *env, emacs_value value, bool required);
bool em_findsym_status_show(git_status_show_t *out, emacs_env *env, emacs_value value, bool required);

emacs_value em_findenum_checkout_notify(git_checkout_notify_t value);
emacs_value em_findenum_delta(git_delta_t value);
emacs_value em_findenum_direction(git_direction value);
emacs_value em_findenum_error(git_error_t value);
emacs_value em_findenum_filemode(git_filemode_t value);
emacs_value em_findenum_merge_preference(git_merge_preference_t value);
emacs_value em_findenum_otype(git_otype value);
emacs_value em_findenum_submodule_ignore(git_submodule_ignore_t value);
emacs_value em_findenum_submodule_update(git_submodule_update_t value);
emacs_value em_findenum_submodule_recurse(git_submodule_recurse_t value);
emacs_value em_findenum_remote_autotag_option(git_remote_autotag_option_t value);
emacs_value em_findenum_repository_state(git_repository_state_t value);
emacs_value em_findenum_stage(int value);

typedef bool setter(void *out, emacs_env *env, emacs_value value, bool on, bool required);

setter em_setflag_checkout_notify;
setter em_setflag_diff_option;
setter em_setflag_index_add_option;
setter em_setflag_merge_file_flag;
setter em_setflag_merge_flag;
setter em_setflag_sort;
setter em_setflag_stash_flags;
setter em_setflag_status_opt;

bool em_setflags_list(void *out, emacs_env *env, emacs_value list, bool required, setter *setter);
bool em_setflags_alist(void *out, emacs_env *env, emacs_value alist, bool required, setter *setter);

emacs_value em_getlist_credtype(emacs_env *env, git_credtype_t value);
emacs_value em_getlist_indexcap(emacs_env *env, int value);
emacs_value em_getlist_merge_analysis(emacs_env *env, git_merge_analysis_t value);
emacs_value em_getlist_status(emacs_env *env, git_status_t value);
emacs_value em_getlist_submodule_status(emacs_env *env, git_submodule_status_t value);

bool em_checkflag_feature(emacs_value *out, emacs_env *env, emacs_value symbol, git_feature_t value, bool required);
bool em_checkflag_submodule_status(emacs_value *out, emacs_env *env, emacs_value symbol,
                                   git_submodule_status_t value, bool required);

#endif /* INTERFACE_H */
