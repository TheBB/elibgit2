#include "egit.h"

#ifndef EGIT_STASH_H
#define EGIT_STASH_H

EGIT_DEFUN(stash_drop, emacs_value _repo, emacs_value _index);
EGIT_DEFUN(stash_foreach, emacs_value _repo, emacs_value func);
EGIT_DEFUN(stash_save, emacs_value _repo, emacs_value _stasher, emacs_value _msg, emacs_value _flags);

#endif /* EGIT_STASH_H */
