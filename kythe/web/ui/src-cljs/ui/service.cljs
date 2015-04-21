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
  (:require [ajax.core :refer [GET POST]]
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

(defn- unwrap-dir-response [resp]
  {:dirs (into {}
           (map (fn [ticket]
                  (let [uri (util/ticket->vname ticket)]
                    [(:path uri) {}]))
             (get resp "subdirectory")))
   :files (into {}
            (map (fn [ticket]
                   (let [vname (util/ticket->vname ticket)]
                     [(util/basename (:path vname)) {:ticket ticket
                                                     :vname vname}]))
              (get resp "file")))})

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

(defn- unwrap-edges-response [resp]
  {:edge_set (:edge_set resp)
   :nodes (into {} (map (juxt :ticket
                          #(into {} (map (juxt :name (comp b64/decodeString :value)) (:fact %))))
                     (:node resp)))
   :next (:next_page_token resp)})

(defn get-edges
  "Requests the outward edges from the given node ticket"
  ([ticket handler error-handler]
   (get-edges ticket {} handler error-handler))
  ([ticket opts handler error-handler]
   (POST "edges"
     {:params (merge {:filter [schema/node-kind-fact
                               schema/anchor-loc-filter]
                      :page_size 20}
                opts
                {:ticket (if (seq? ticket) ticket [ticket])})
      :format :json
      :response-format :json
      :keywords? true
      :handler (comp handler unwrap-edges-response)
      :error-handler error-handler})))

(defn get-file
  "Requests the source text and decorations of the given file"
  [ticket handler error-handler]
  (POST "decorations"
    {:params {:location {:ticket ticket}
              :source_text true
              :references true}
     :format :json
     :response-format :json
     :keywords? true
     :handler handler
     :error-handler error-handler}))

(defn get-search
  "Searches for nodes by partial VName and known fact values"
  [query handler error-handler]
  (POST "search"
    {:params (assoc query :fact
               (for [{:keys [name value]} (:fact query)]
                 {:name name
                  :value (b64/encodeString value)}))
     :format :json
     :response-format :json
     :keywords? true
     :handler (comp handler :ticket)
     :error-handler error-handler}))
