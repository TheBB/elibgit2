BUILD_OPTIONS=-DCMAKE_BUILD_TYPE=Debug

ifeq '$(findstring ;,$(PATH))' ';'
    UNAME := Windows
else
    UNAME := $(shell uname 2>/dev/null || echo Unknown)
    UNAME := $(patsubst CYGWIN%,Cygwin,$(UNAME))
    UNAME := $(patsubst MSYS%,MSYS,$(UNAME))
    UNAME := $(patsubst MINGW%,MSYS,$(UNAME))
endif

ifeq ($(UNAME),MSYS)
	BUILD_OPTIONS+= -G "MSYS Makefiles"
endif

# If the variable USE_SYSTEM_LIBGIT2 is set to *any* value, use the
# system provided libgit2 library.
USE_SYSTEM_LIBGIT2? := \
	$(if $(or $(USE_SYSTEM_LIBGIT2),\
	 	  $(findstring USE_SYSTEM_LIBGIT2,$(BUILD_OPTIONS))),\
		true)

ifeq "$(TRAVIS)" "true"
## Makefile for Travis ###################################################
#
#  TODO Move this to a separate file ".travis.mk" once Emake supports
#  that.  (Apparently it already does, but I couldn't get it to work.)

EMAKE_SHA1       ?= 1b23379eb5a9f82d3e2d227d0f217864e40f23e0
PACKAGE_BASENAME := libgit

include emake.mk

build/libegit2.so:
	git submodule update --init
	mkdir -p build
	cd build && cmake .. $(BUILD_OPTIONS) && make

test: EMACS_ARGS += -L build/ -l libegit2
test: build/libegit2.so test-ert

else
## Makefile for local use ################################################

-include .config.mk

PKG = libgit

ELS  = $(PKG).el
ELCS = $(ELS:.el=.elc)

EMACS      ?= emacs
EMACS_ARGS ?=

LOAD_PATH  ?= -L . -L build

.PHONY: test libgit2 submodule-update

all: lisp

help:
	$(info make all               - build everything)
	$(info make module            - generate module)
	$(info make lisp              - generate byte-code and autoloads)
	$(info make submodule-init    - update the submodule)
	$(info make submodule-update  - update the submodule)
	$(info make test              - run tests)
	$(info make clean             - remove generated files)
	@printf "\n"

module: build/libegit2.so

build/libegit2.so: libgit2
	@printf "Building $<\n"
	@mkdir -p build
	@cd build && cmake .. $(BUILD_OPTIONS) && make

lisp: $(ELCS) loaddefs module

loaddefs: $(PKG)-autoloads.el

%.elc: %.el
	@printf "Compiling $<\n"
	@$(EMACS) -Q --batch $(EMACS_ARGS) $(LOAD_PATH) -f batch-byte-compile $<

test: libgit.elc build/libegit2.so
	$(EMACS) -Q --batch $(LOAD_PATH) -l libgit \
	  $(addprefix -l test/,$(shell ls test)) \
	  -f ert-run-tests-batch-and-exit

submodule-init: libgit2

submodule-update:
	@git submodule update

libgit2:
ifeq ($(USE_SYSTEM_LIBGIT2?),)
	@git submodule update --init
else
	@echo "Using the system provided libgit2 library"
endif

CLEAN  = $(ELCS) $(PKG)-autoloads.el build

clean:
	@printf "Cleaning...\n"
	@rm -rf $(CLEAN)

define LOADDEFS_TMPL
;;; $(PKG)-autoloads.el --- automatically extracted autoloads
;;
;;; Code:
(add-to-list 'load-path (directory-file-name \
(or (file-name-directory #$$) (car load-path))))

;; Local Variables:
;; version-control: never
;; no-byte-compile: t
;; no-update-autoloads: t
;; End:
;;; $(PKG)-autoloads.el ends here
endef
export LOADDEFS_TMPL
#'

$(PKG)-autoloads.el: $(ELS)
	@printf "Generating $@\n"
	@printf "%s" "$$LOADDEFS_TMPL" > $@
	@$(EMACS) -Q --batch --eval "(progn\
	(setq make-backup-files nil)\
	(setq vc-handled-backends nil)\
	(setq default-directory (file-truename default-directory))\
	(setq generated-autoload-file (expand-file-name \"$@\"))\
	(setq find-file-visit-truename t)\
	(update-directory-autoloads default-directory))"

endif
