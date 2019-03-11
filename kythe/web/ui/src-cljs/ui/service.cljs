;; Copyright 2014 The Kythe Authors. All rights reserved.
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
  (:require [ajax.core :refer [GET POST]]
            [clojure.string :as string]
            [goog.crypt.base64 :as b64]
            [ui.schema :as schema]
            [ui.util :as util]))

(defn- unwrap-corpus-roots-response [resp]
  (into {}
    (for [corpusRoots (get resp "corpus")]
      [(get corpusRoots "name") (get corpusRoots "root")])))

(defn get-corpus-roots
  "Requests all of the known corpus roots"
  [handler error-handler]
  (GET "corpusRoots"
    {:response-format :json
     :handler (comp handler unwrap-corpus-roots-response)
     :error-handler error-handler}))

(defn- concat-path [& components]
  (string/join "/" (filter #(not (empty? %)) components)))

(defn- unwrap-dir-response [resp]
  (let [{:strs [corpus root path entry]} resp
        vname {:corpus corpus :root root}]
    {:dirs (into {}
                 (map (fn [{:strs [name]}] [(concat-path path name) {}])
                      (filter (fn [{:strs [kind]}] (= "DIRECTORY" kind))
                              entry)))
     :files (into {}
                  (map (fn [e] (let [name (get e "name")
                                     vname (assoc vname :path (concat-path path name))]
                                 [name {:vname vname :ticket (util/vname->ticket vname)}]))
                       (filter (fn [{:strs [kind]}] (= "FILE" kind))
                               entry)))}))

(defn get-directory
  "Requests the contents of the given directory"
  [corpus root path handler error-handler]
  (POST "dir"
    {:params {:corpus corpus
              :root root
              :path path}
     :format :json
     :response-format :json
     :handler (comp handler unwrap-dir-response)
     :error-handler error-handler}))

(defn- unwrap-node [node]
  {:facts (into {}
                (map (juxt first
                           (comp util/fix-encoding b64/decodeString second))
                     (:facts node)))
   :definition (:definition node)})

(defn- unwrap-xrefs-response [resp]
  {:cross-references (if (= 1 (count (:cross_references resp)))
                       (second (first (:cross_references resp)))
                       (:cross_references resp))
   :nodes (into {} (map (juxt first (comp unwrap-node second)) (:nodes resp)))
   :definition_locations (:definition_locations resp)
   :next (:next_page_token resp)})

(defn get-xrefs
  "Requests the global references, definitions and declarations of the given node ticket."
  ([ticket handler error-handler]
   (get-xrefs ticket {} handler error-handler))
  ([ticket opts handler error-handler]
   (POST "xrefs"
     {:params (merge {:definition_kind    "BINDING_DEFINITIONS"
                      :declaration_kind   "ALL_DECLARATIONS"
                      :reference_kind     "NON_CALL_REFERENCES"
                      :caller_kind        "OVERRIDE_CALLERS"
                      :snippets           "DEFAULT"
                      :node_definitions true
                      :filter [schema/node-kind-fact]
                      :anchor_text true
                      :page_size 20}
                opts
                {:ticket (if (seq? ticket) ticket [ticket])})
      :format :json
      :response-format :json
      :keywords? true
      :handler (comp handler unwrap-xrefs-response)
      :error-handler error-handler})))

(defn get-file
  "Requests the source text and decorations of the given file"
  [ticket handler error-handler]
  (POST "decorations"
    {:params {:location {:ticket ticket}
              :source_text true
              :references true
              :target_definitions true}
     :format :json
     :response-format :json
     :keywords? true
     :handler handler
     :error-handler error-handler}))
