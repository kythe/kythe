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
(ns ui.util
  (:require-macros [cljs.core.async.macros :refer [go-loop]])
  (:require [cljs.core.async :refer [<!]])
  (:import [goog Uri]))

(defn fix-encoding
  "Tries to fix the encoding of strings with non-ASCII characters. Useful for displaying the result
  of b64/decodeString."
  [str]
  (js/decodeURIComponent (js/escape str)))

(defn basename
  "Returns the basename of the given path"
  [path]
  (cond
   (empty? path) path
   (= (get path (- (count path) 1)) "/") (basename (subs path 0 (- (count path) 1)))
   :else (subs path (+ 1 (.lastIndexOf path "/")))))

(defn path-display
  ([{:keys [corpus root path]}]
     (path-display corpus root path))
  ([corpus root path]
     (str corpus ":" root ":" path)))

(defn handle-ch
  ([ch state handler]
     (go-loop [state state]
       (recur (handler (<! ch) state))))
  ([ch handler]
     (go-loop []
       (handler (<! ch))
       (recur))))

(defn- decode-ticket-query [query]
  (let [params (into {}
                 (for [arg (.split query "?")]
                   (let [[key val] (.split arg "=" 2)]
                     [(keyword key) val])))]
    {:language (:lang params)
     :path (:path params)
     :root (:root params)}))

(defn ticket->vname [ticket]
  (let [uri (Uri. ticket)]
    (when-not (= (.getScheme uri))
      (.log js/console (str "WARNING: invalid ticket '" ticket "'")))
    (assoc (decode-ticket-query (.getDecodedQuery uri))
      :corpus (.getDomain uri)
      :signature (.getFragment uri))))

(defn- maybe-encode-with-prefix [prefix s]
  (if s (str prefix (js/encodeURIComponent s)) ""))

(defn vname->ticket [vname]
  (str
    "kythe:"
    (maybe-encode-with-prefix "//" (:corpus vname))
    (maybe-encode-with-prefix "?lang=" (:language vname))
    (maybe-encode-with-prefix "?path=" (:path vname))
    (maybe-encode-with-prefix "?root=" (:root vname))
    (maybe-encode-with-prefix "#" (:signature vname))))

(def ^:private abbreviations
  {:lang :language
   :sig :signature})

(defn parse-url-state []
  (let [uri (Uri. (subs (.-hash js/location) 1))
        q (.getQueryData uri)
        state (into {} (for [key (.getKeys q)
                             :let [k (keyword key)]]
                         [(or (k abbreviations) k) (.get q key)]))]
    (if (empty? (.getPath uri))
      state
      (assoc state :path (.getPath uri)))))

(defn set-url-state [state]
  (let [path (:path state)
        q (Uri.QueryData.)]
    (doseq [[k v] (dissoc state :path)
            :when v]
      (.add q (name k) v))
    (set! (.-hash js/location) (str "#" path "?" q))))
