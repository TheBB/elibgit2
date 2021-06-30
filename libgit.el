;;; libgit.el --- Thin bindings to libgit2. -*- lexical-binding: t; -*-

;; Copyright (C) 2018 TheBB
;;
;; Author: Eivind Fonn <evfonn@gmail.com>
;; URL: https://github.com/magit/libegit2
;; Version: 0.0.1
;; Keywords: git vc
;; Package-Requires: ((emacs "25.1"))

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

;;; Commentary:

;; This package provides thin bindings to libgit2. To use these bindings,
;; issue a call to (require 'libgit). This will load the dynamic module,
;; or prompt the user to build it.

;;; Code:

(unless module-file-suffix
  (error "Module support not detected, libgit can't work"))

(defcustom libgit-cmake-generator nil
  "CMake generator to use."
  :type 'string
  :group 'libgit)

(defvar libgit--root
  (file-name-directory (or load-file-name buffer-file-name))
  "Directory where libgit is installed.")

(defvar libgit--build-dir
  (expand-file-name "build" libgit--root)
  "Directory where the libegit2 dynamic module file should be built.")

(defvar libgit--module-file
  (expand-file-name "libegit2.so" libgit--build-dir)
  "Path to the libegit2 dynamic module file.")

(defun libgit--configure ()
  "Run the configure step of libegit2 asynchronously.

On successful exit, pass control on to the build step."
  (make-directory libgit--build-dir 'parents)
  (let ((default-directory libgit--build-dir))
    (set-process-sentinel
     (if libgit-cmake-generator
         (start-process "libgit-cmake" "*libgit build*" "cmake" ".." "-G" libgit-cmake-generator)
       (start-process "libgit-cmake" "*libgit build*" "cmake" ".."))
     (lambda (proc _event)
       (when (eq 'exit (process-status proc))
         (if (= 0 (process-exit-status proc))
             (libgit--build)
           (pop-to-buffer "*libgit build*")
           (error "libgit: configuring failed with exit code %d" (process-exit-status proc))))))))

(defun libgit--build ()
  "Run the build step of libegit2 asynchronously.

On successful exit, pass control on to the load step."
  (let ((default-directory libgit--build-dir))
    (set-process-sentinel
     (start-process "libgit-cmake" "*libgit build*" "cmake" "--build" ".")
     (lambda (proc _event)
       (when (eq 'exit (process-status proc))
         (if (= 0 (process-exit-status proc))
             (libgit--load)
           (pop-to-buffer "*libgit build*")
           (error "libgit: building failed with exit code %d" (process-exit-status proc))))))))

(defun libgit--load ()
  "Load the `libegit2' dynamic module.
If that fails, then raise an error."
  (unless (featurep 'libegit2)
    (load libgit--module-file nil t t))
  (unless (featurep 'libegit2)
    (error "libgit: unable to load the libegit2 dynamic module")))

;;;###autoload
(defun libgit-load ()
  "Load the `libegit2' dynamic module.
If the module is not available, then offer to build it."
  (interactive)
  (cond
   ((file-exists-p libgit--module-file)
    (libgit--load))
   ((and (not noninteractive)
         (y-or-n-p "libgit must be built, do so now?"))
    (libgit--configure))
   (noninteractive
    (message "libgit was not loaded!"))
   (t
    (error "libgit was not loaded!"))))

(libgit-load)

(provide 'libgit)

;;; libgit.el ends here
