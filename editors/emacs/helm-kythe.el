;;; helm-kythe.el --- Google Kythe helm interface  -*- lexical-binding: t; -*-

(require 'cl)
(require 'dash)
(require 'helm)
(require 'inf-haskell)  ;; for inferior-haskell-find-project-root
(require 's)

(defcustom helm-kythe-auto-update nil
  "*If non-nil, tag files are updated whenever a file is saved."
  :group 'helm-kythe
  :type 'boolean)

(defcustom helm-kythe-highlight-candidate t
  "Highlight candidate or not"
  :type 'boolean)

;; TODO 0.0.26 /opt/kythe/tools/http_server --listen localhost:8080 does not listen on ::1
(defcustom helm-kythe-http-server-url "http://127.0.0.1:8080"
  "kythe/tools/http_server --listen"
  :type 'string)

(defcustom helm-kythe-prefix-key (kbd "C-c k")
  "If non-nil, it is used for the prefix key of helm-kythe-xxx command."
  :type 'string)

(defcustom helm-kythe-suggested-key-mapping t
  "If non-nil, suggested key mapping is enabled."
  :type 'boolean)

(defcustom helm-kythe-file-search-paths nil
  "A list of search paths for \"kythe://?path=$path\"."
  :type '(list string))

(defface helm-kythe-active
  '()
  "Face for mode line when Kythe is active.")

(defface helm-kythe-file
  '((t :inherit font-lock-keyword-face))
  "Face for filenames in the error list.")

(defface helm-kythe-inactive
  '((t (:strike-through t)))
  "Face for mode line when Kythe is inactive.")

(defface helm-kythe-lineno
  '((t :inherit font-lock-doc-face))
  "Face for line numbers in the error list.")

(defface helm-kythe-match
  '((t :inherit helm-match))
  "Face for word matched against tagname")

(defvar helm-kythe--find-file-action
  (helm-make-actions
   "Open file" #'helm-kythe--action-openfile
   "Open file other window" #'helm-kythe--action-openfile-other-window))

(defvar helm-kythe--use-otherwin nil)

(defvar helm-source-helm-kythe-definitions
  (helm-build-in-buffer-source "Find definitions"
    :init #'helm-kythe--source-definitions
    :real-to-display #'helm-kythe--candidate-transformer
    :action #'helm-kythe--find-file-action))

(defvar helm-source-helm-kythe-imenu
  (helm-build-in-buffer-source "imenu"
    :init #'helm-kythe--source-imenu
    :real-to-display #'helm-kythe--candidate-transformer
    :action #'helm-kythe--find-file-action))

(defvar helm-source-helm-kythe-references
  (helm-build-in-buffer-source "Find references"
    :init #'helm-kythe--source-references
    :real-to-display #'helm-kythe--candidate-transformer
    :action #'helm-kythe--find-file-action))

(defvar helm-source-helm-kythe-ticket-definitions
  (helm-build-in-buffer-source "Find ticket definitions"
    :init #'helm-kythe--source-ticket-definitions
    :real-to-display #'helm-kythe--candidate-transformer
    :action #'helm-kythe--find-file-action))

(defvar-local helm-kythe-applied-decorations-p nil
  "Non-nil if decorations have been applied.")

(defconst helm-kythe--buffer "*helm helm-kythe*")
(defconst helm-kythe--anchor-regex "\\`\\([^:]+\\):\\([^:]+\\):\\([^:]+\\):\\(.*\\)")

(define-error 'helm-kythe-error "helm-kythe error")

(defun helm-kythe--action-openfile (candidate)
  (when (string-match helm-kythe--anchor-regex candidate)
    (let ((filename (match-string-no-properties 1 candidate))
          (line (string-to-number (match-string-no-properties 2 candidate)))
          (column (string-to-number (match-string-no-properties 3 candidate))))
      (when (helm-kythe--find-file
             (if helm-kythe--use-otherwin #'find-file-other-window #'find-file) filename)
        (goto-line line)
        (forward-char column)
        (recenter)))))

(defun helm-kythe--action-openfile-other-window (candidate)
  (let ((helm-kythe--use-otherwin t))
    (helm-kythe--action-openfile candidate)))

(defun helm-kythe--anchor-to-candidate (anchor)
  (format "%s:%d:%d:%s"
          (helm-kythe--path-from-ticket (alist-get 'parent anchor))
          (alist-get 'line_number (alist-get 'start anchor))
          (or (alist-get 'column_offset (alist-get 'start anchor)) 0)  ;; TODO Cabal cpu: 'start' of definition_locations of getSystemArch = X86_64 does not have column_offset
          (alist-get 'snippet anchor)))

(defun helm-kythe--candidate-transformer (candidate)
  (if (and helm-kythe-highlight-candidate
           (string-match helm-kythe--anchor-regex candidate))
      (format "%s:%s:%s:%s"
              (propertize (match-string 1 candidate) 'face 'helm-kythe-file)
              (propertize (match-string 2 candidate) 'face 'helm-kythe-lineno)
              (propertize (match-string 3 candidate) 'face 'helm-kythe-lineno)
              (match-string 4 candidate))
      candidate))

(defun helm-kythe--char-offsets (object start-key end-key)
  (cons (byte-to-position (1+ (alist-get 'byte_offset (alist-get start-key object))))
        (byte-to-position (1+ (alist-get 'byte_offset (alist-get end-key object))))))

(defun helm-kythe--common (srcs)
  (let ((helm-quit-if-no-candidate t)
        (helm-execute-action-at-once-if-one t))
    (helm :sources srcs :buffer helm-kythe--buffer)))

(defun helm-kythe--find-file (open-func path)
  (cond ((file-name-absolute-p path) (funcall open-func path))
        ((s-ends-with? path (buffer-file-name)) t)
        (t
         (-if-let* ((_ (equal major-mode 'haskell-mode))
                    (project-root (helm-kythe--haskell-find-project-root))
                    (f (concat (file-name-directory project-root) path))
                    (_ (file-exists-p f)))
             (funcall open-func f)
           (loop for search-path in helm-kythe-file-search-paths do
                 (-if-let* ((f (concat search-path path))
                            (_ (file-exists-p f)))
                     (funcall open-func f)))))))

(defun helm-kythe--haskell-find-project-root ()
  (let ((p (inferior-haskell-find-project-root (current-buffer))))
    (while (not (string-match-p ".-[0-9]" (file-name-nondirectory p)))
      (setq p (directory-file-name (file-name-directory p))))
    p))

(defun helm-kythe--path-from-ticket (ticket)
  (when-let (i (string-match "path=\\([^#]+\\)" ticket)) (match-string 1 ticket)))

(defun helm-kythe-post (path data)
  (let ((url-request-method "POST")
        (url-request-extra-headers '(("Content-Type" . "application/json")))
        (url-request-data (json-encode data)))
    (with-current-buffer (url-retrieve-synchronously (concat helm-kythe-http-server-url path) t)
      (beginning-of-buffer)
      (re-search-forward "\n\n")
      (buffer-string)
      (condition-case nil
          (save-excursion (json-read))
        ('json-error
         (signal 'helm-kythe-error (concat "Kythe http_server error: " (string-trim (buffer-substring-no-properties (point) (point-max))))))))))

(defun helm-kythe--propertize-mode-line (face)
  (propertize " Kythe" 'face face))

(defun helm-kythe--source-definitions ()
  (with-current-buffer (helm-candidate-buffer 'global)
    (erase-buffer))
  (-when-let* ((ticket (-some->> (get-text-property (point) 'helm-kythe-reference) (alist-get 'target_ticket)))
               (defs (helm-kythe-get-definitions ticket)))
    (helm-init-candidates-in-buffer 'global (mapcar #'helm-kythe--anchor-to-candidate defs))
    ))

(defun helm-kythe--source-imenu ()
  (with-current-buffer (helm-candidate-buffer 'global)
    (erase-buffer))
  (let ((xs))
    (save-excursion
      (beginning-of-buffer)
      (while (not (eobp))
        (let ((def (get-text-property (point) 'helm-kythe-definition))
              (next-change
               (or (next-property-change (point) (current-buffer))
                   (point-max))))
          (when def (push (helm-kythe--anchor-to-candidate def) xs))
          (goto-char next-change))))
    (helm-init-candidates-in-buffer 'global (nreverse xs))))

(defun helm-kythe--source-references ()
  (with-current-buffer (helm-candidate-buffer 'global)
    (erase-buffer))
  (-when-let* ((ticket (-some->> (get-text-property (point) 'helm-kythe-reference) (alist-get 'target_ticket)))
               (refs (helm-kythe-get-references ticket)))
    (helm-init-candidates-in-buffer 'global (mapcar #'helm-kythe--anchor-to-candidate refs))))

(defun helm-kythe--source-ticket-definitions ()
  (with-current-buffer (helm-candidate-buffer 'global)
    (erase-buffer))
  (-when-let* ((ticket (helm-comp-read "Ticket: " '()))
               (defs (helm-kythe-get-definitions ticket)))
    (helm-init-candidates-in-buffer 'global (mapcar #'helm-kythe--anchor-to-candidate defs))))

(defun helm-kythe-post-decorations (ticket)
  (helm-kythe-post "/decorations"
              `((location . ((ticket . ,ticket)))
                (references . t)
                (target_definitions . t))))

(defun helm-kythe-post-dir (path)
  (helm-kythe-post "/dir" `((path . ,path))))

(defun helm-kythe-post-xrefs (ticket data)
  (helm-kythe-post "/xrefs"
              `((anchor_text . t)
                (ticket . [,ticket])
                ,@data
                )))

(defun helm-kythe-apply-decorations ()
  (interactive)
  (when-let (filename (buffer-file-name))
    (if (eq major-mode 'haskell-mode)
        (-when-let* [(project-root (helm-kythe--haskell-find-project-root))
                     (filename (buffer-file-name))]
          (condition-case ex
              (helm-kythe-decorations (concat (file-name-nondirectory project-root) "/" (file-relative-name filename project-root)))
            ('helm-kythe-error (error "%s" (cdr ex)))))
      (loop for search-path in helm-kythe-file-search-paths do
            (when-let (path (file-relative-name filename search-path))
              (helm-kythe-decorations path)
              (return))))))

;; .cross_references | values[].definition[].anchor
(defun helm-kythe-get-definitions (ticket)
  (when-let (xrefs (-some->> (helm-kythe-post-xrefs ticket '((definition_kind . "BINDING_DEFINITIONS")))
                         (assoc 'cross_references) (cdr)))
    (-mapcat (lambda (xref)
                   (when-let (defs (alist-get 'definition (cdr xref)))
                     (mapcar (lambda (x) (alist-get 'anchor x)) defs)
                     )) xrefs)))

;; .cross_references | values[].reference[].anchor
(defun helm-kythe-get-references (ticket)
  (when-let (xrefs (-some->> (helm-kythe-post-xrefs ticket '((reference_kind . "ALL_REFERENCES")))
                             (assoc 'cross_references) (cdr)))
    (-mapcat (lambda (xref)
                   (when-let (refs (alist-get 'reference (cdr xref)))
                     ;; 'start' of /kythe/edge/ref/doc do not have column_offset
                     ;; (-filter (lambda (x) (not (equal (alist-get 'kind x) "/kythe/edge/ref/doc"))) (mapcar (lambda (x) (alist-get 'anchor x)) refs))
                     (mapcar (lambda (x) (alist-get 'anchor x)) refs)
                     )) xrefs)))

(defun helm-kythe-get-xrefs (ticket)
  (-some->> (helm-kythe-post-xrefs ticket '((declaration_kind . "ALL_DECLARATIONS")
                              (definition_kind . "BINDING_DEFINITIONS")
                              (reference_kind . "ALL_REFERENCES")
                              ))
           (assoc 'cross_references) (cdr)))

(defun helm-kythe-decorations (filepath)
  (when-let (refs (helm-kythe-post-decorations (concat "kythe://?path=" filepath)))
    (mapc (lambda (ref)
            (-let [(start . end) (helm-kythe--char-offsets ref 'anchor_start 'anchor_end)]
              (put-text-property start end 'helm-kythe-reference ref))) (alist-get 'reference refs))
    (mapc (lambda (ticket-def)
            (-let [def (cdr ticket-def)]
              ;; cxx_extractor or cxx_indexer, "definition_locations" of a.cc may include tickets of "stdio.h"
              (when (equal filepath (helm-kythe--path-from-ticket (alist-get 'ticket def)))
                (-let [(start . end) (helm-kythe--char-offsets def 'start 'end)]
                  (put-text-property start end 'helm-kythe-definition def))))) (alist-get 'definition_locations refs))
    nil))

(defun helm-kythe-find-definitions ()
  (interactive)
  (helm-kythe--common '(helm-source-helm-kythe-definitions)))

(defun helm-kythe-find-references ()
  (interactive)
  (helm-kythe--common '(helm-source-helm-kythe-references)))

(defun helm-kythe-find-ticket-definitions ()
  (interactive)
  (helm-kythe--common '(helm-source-helm-kythe-ticket-definitions)))

(defun helm-kythe-dwim ()
  ;; TODO
  )

(defun helm-kythe-imenu ()
  (interactive)
  (helm-kythe--common '(helm-source-helm-kythe-imenu)))

(defun helm-kythe-resume ()
  (interactive)
  (unless (get-buffer helm-kythe--buffer)
    (error "Error: helm-kythe buffer does not exist."))
  (helm-resume helm-kythe--buffer))

(defun helm-kythe-update-index ()
  ;; TODO
  )

(defvar helm-kythe-mode-map (make-sparse-keymap))
(defvar helm-kythe--mode-line (helm-kythe--propertize-mode-line 'helm-kythe-inactive))

;;;###autoload
(define-minor-mode helm-kythe-mode ()
  "Enable helm-kythe-mode"
  :init-value nil
  :lighter helm-kythe--mode-line
  :global nil
  :keymap helm-kythe-mode-map
  (if helm-kythe-mode
      (progn (when helm-kythe-auto-update
               (add-hook 'after-save-hook 'helm-kythe-update-index nil t))
             (helm-kythe-apply-decorations))
    (when helm-kythe-auto-update
      (remove-hook 'after-save-hook 'helm-kythe-update-index t))))

(when helm-kythe-suggested-key-mapping
  (let ((command-table '(("a" . helm-kythe-apply-decorations)
                         ("d" . helm-kythe-find-definitions)
                         ;; ("D" . helm-kythe-find-ticket-definitions)
                         ("l" . helm-kythe-imenu)
                         ("r" . helm-kythe-find-references)
                         ("R" . helm-kythe-resume)))
        (key-func (if (string-prefix-p "\\" helm-kythe-prefix-key)
                      #'concat
                    (lambda (prefix key) (kbd (concat prefix " " key))))))
    (loop for (key . command) in command-table
          do
          (define-key helm-kythe-mode-map (funcall key-func helm-kythe-prefix-key key) command))
    ))

(provide 'helm-kythe)
