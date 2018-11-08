(ert-deftest branch-create ()
  (with-temp-dir path
    (init)
    (commit-change "test" "content")
    (let ((repo (libgit-repository-open path)))
      (should (libgit-branch-create repo "new-branch" "HEAD"))
      (should-error (libgit-branch-create repo "new-branch" "HEAD"))))
  (with-temp-dir path
    (init)
    (commit-change "test" "content")
    (run "git" "branch" "second")
    (run "git" "checkout" "second")
    (commit-change "test2" "content2")
    (let ((repo (libgit-repository-open path)))
      (should-error (libgit-branch-create repo "master" "second"))
      (should (libgit-branch-create repo "master" "second" t)))))

(ert-deftest branch-create-from-annotated ()
  (with-temp-dir path
    (init)
    (commit-change "test" "content")
    (let ((repo (libgit-repository-open path)))
      (should (libgit-branch-create-from-annotated repo "new-branch" "HEAD"))
      (should-error (libgit-branch-create-from-annotated repo "new-branch" "HEAD"))))
  (with-temp-dir path
    (init)
    (commit-change "test" "content")
    (run "git" "branch" "second")
    (run "git" "checkout" "second")
    (commit-change "test2" "content2")
    (let ((repo (libgit-repository-open path)))
      (should-error (libgit-branch-create-from-annotated repo "master" "second"))
      (should (libgit-branch-create-from-annotated repo "master" "second" t)))))

(ert-deftest branch-lookup ()
  (with-temp-dir path
    (init)
    (commit-change "test" "content")
    (run "git" "branch" "second")
    (let ((repo (libgit-repository-open path)))
      (should (libgit-branch-lookup repo "master"))
      (should (libgit-branch-lookup repo "second"))
      (should (libgit-branch-lookup repo "second"))
      (should-error (libgit-branch-lookup repo "third"))
      (should-error (libgit-branch-lookup repo "master" t)))))

(ert-deftest branch-delete ()
  (with-temp-dir path
    (init)
    (commit-change "test" "content")
    (run "git" "branch" "second")
    (let* ((repo (libgit-repository-open path))
           (masterref (libgit-branch-lookup repo "master"))
           (secondref (libgit-branch-lookup repo "second")))
      (should-error (libgit-branch-delete masterref))
      (libgit-branch-delete secondref))))

(ert-deftest branch-checked-out-p ()
  (with-temp-dir path
    (init)
    (commit-change "test" "content")
    (run "git" "branch" "second")
    (let* ((repo (libgit-repository-open path))
           (masterref (libgit-branch-lookup repo "master"))
           (secondref (libgit-branch-lookup repo "second")))
      (should (libgit-branch-checked-out-p masterref))
      (should-not (libgit-branch-checked-out-p secondref)))))

(ert-deftest branch-head-p ()
  (with-temp-dir path
    (init)
    (commit-change "test" "content")
    (run "git" "branch" "second")
    (let* ((repo (libgit-repository-open path))
           (masterref (libgit-branch-lookup repo "master"))
           (secondref (libgit-branch-lookup repo "second")))
      (should (libgit-branch-head-p masterref))
      (should-not (libgit-branch-head-p secondref)))))

(ert-deftest branch-iterator-new ()
  (with-temp-dir path
    (init)
    (commit-change "test" "content")
    (run "git" "branch" "second")
    (let ((repo (libgit-repository-open path)))
      (should (libgit-branch-iterator-new repo))
      (should (libgit-branch-iterator-new repo 'local))
      (should (libgit-branch-iterator-new repo 'remote))
      (should (libgit-branch-iterator-new repo 'all)))))
