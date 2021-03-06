(ert-deftest tree-inspect ()
  (with-temp-dir path
    (init)
    (write "file1" "some content")
    (write "file2" "some other content")
    (write "dir1/file3" "wow such content")
    (write "dir1/file4" "unbelievable")
    (write "dir2/file5" "vincemcmahon.gif")
    (add "file1" "file2" "dir1/file3" "dir1/file4" "dir2/file5")
    (commit)
    (let* ((repo (libgit-repository-open path))
           (head (libgit-reference-name-to-id repo "HEAD"))
           (commit (libgit-commit-lookup repo head))
           (root (libgit-commit-tree commit))
           (blob1-id (caddr (libgit-tree-entry-byname root "file1")))
           (blob2-id (caddr (libgit-tree-entry-byname root "file2")))
           (tree1-id (caddr (libgit-tree-entry-byname root "dir1")))
           (tree1 (libgit-tree-lookup repo tree1-id))
           (blob3-id (caddr (libgit-tree-entry-byname tree1 "file3")))
           (blob4-id (caddr (libgit-tree-entry-byname tree1 "file4")))
           (tree2-id (caddr (libgit-tree-entry-byname root "dir2")))
           (tree2 (libgit-tree-lookup repo tree2-id))
           (blob5-id (caddr (libgit-tree-entry-byname tree2 "file5")))
           collect)

      (should (= 4 (libgit-tree-entrycount root)))
      (should (= 2 (libgit-tree-entrycount tree1)))
      (should (= 1 (libgit-tree-entrycount tree2)))

      ;; Getting entries by name, id and path
      (should (equal `(blob blob ,blob1-id "file1") (libgit-tree-entry-byname root "file1")))
      (should (equal `(blob blob ,blob2-id "file2") (libgit-tree-entry-byid root blob2-id)))
      (should (equal `(blob blob ,blob3-id "file3") (libgit-tree-entry-bypath root "dir1/file3")))
      (should (equal `(tree tree ,tree1-id "dir1") (libgit-tree-entry-byname root "dir1")))

      ;; Getting entries by index (relies on a reliable ordering)
      (should (equal (cl-loop for i below 4 collect (libgit-tree-entry-byindex root i))
                     `((tree tree ,tree1-id "dir1")
                       (tree tree ,tree2-id "dir2")
                       (blob blob ,blob1-id "file1")
                       (blob blob ,blob2-id "file2"))))
      (should (equal (cl-loop for i below 2 collect (libgit-tree-entry-byindex tree1 i))
                     `((blob blob ,blob3-id "file3")
                       (blob blob ,blob4-id "file4"))))
      (should (equal (libgit-tree-entry-byindex tree2 0)
                     `(blob blob ,blob5-id "file5")))

      ;; Pre-order traversal
      (libgit-tree-walk root 'pre (lambda (path entry) (push (cons path entry) collect)))
      (should (equal (reverse collect)
                     `(("" tree tree ,tree1-id "dir1")
                       ("dir1/" blob blob ,blob3-id "file3")
                       ("dir1/" blob blob ,blob4-id "file4")
                       ("" tree tree ,tree2-id "dir2")
                       ("dir2/" blob blob ,blob5-id "file5")
                       ("" blob blob ,blob1-id "file1")
                       ("" blob blob ,blob2-id "file2"))))

      ;; Post-order traversal
      (setq collect nil)
      (libgit-tree-walk root 'post (lambda (path entry) (push (cons path entry) collect)))
      (should (equal (reverse collect)
                     `(("dir1/" blob blob ,blob3-id "file3")
                       ("dir1/" blob blob ,blob4-id "file4")
                       ("" tree tree ,tree1-id "dir1")
                       ("dir2/" blob blob ,blob5-id "file5")
                       ("" tree tree ,tree2-id "dir2")
                       ("" blob blob ,blob1-id "file1")
                       ("" blob blob ,blob2-id "file2"))))

      ;; Pre-order traversal with `skip' return value
      (setq collect nil)
      (libgit-tree-walk root 'pre (lambda (path entry) (push (cons path entry) collect) 'skip))
      (should (equal (reverse collect)
                     `(("" tree tree ,tree1-id "dir1")
                       ("" tree tree ,tree2-id "dir2")
                       ("" blob blob ,blob1-id "file1")
                       ("" blob blob ,blob2-id "file2")))))))
