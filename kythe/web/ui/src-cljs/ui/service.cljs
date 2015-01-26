;; Copyright 2014 Google Inc. All rights reserved.
;;
;; Licensed under the Apache License, Version 2.0 (the "License");
;; you may not use this file except in compliance with the License.
;; You may obtain a copy of the License at
;;
;;   http://www.apache.org/licenses/LICENSE-2.0
;;
;; Unless required by applicable law or agreed to in writing, software
;; distributed under the License is distributed on an "AS IS" BASIS,
;; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
;; See the License for the specific language governing permissions and
;; limitations under the License.
(ns ui.service
  "Namespace for functions communicating with the xrefs server"
  (:require [ajax.core :refer [GET]]
            [ui.util :as util]))

(defn get-corpus-roots
  "Requests all of the known corpus roots"
  [handler error-handler]
  (GET "corpusRoots"
       {:response-format :json
        :handler handler
        :error-handler error-handler}))

(defn get-directory
  "Requests the contents of the given directory"
  [corpus root path handler error-handler]
  (GET (str "dir" path)
       {:params {:corpus corpus
                 :root (or root "")}
        :response-format :json
        :handler handler
        :error-handler error-handler}))

(defn ^:private normalize-xrefs [xrefs]
  (into {} (for [[k refs] xrefs]
             [k (for [ref refs]
                  (assoc ref :file (util/normalize-vname (:file ref))))])))

(defn get-xrefs
  "Requests the cross-references for the given node ticket"
  [ticket handler error-handler]
  (GET "xrefs"
       {:params {:ticket ticket}
        :response-format :json
        :keywords? true
        :handler #(handler (normalize-xrefs %))
        :error-handler error-handler}))

(defn get-file
  "Requests the source text and decorations of the given file"
  [vname handler error-handler]
  (GET (str "file/" (:path vname))
       {:params (select-keys vname [:corpus :root :language :signature])
        :response-format :json
        :keywords? true
        :handler handler
        :error-handler error-handler}))
