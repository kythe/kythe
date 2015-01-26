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
  (:require [cljs.core.async :refer [<!]]))

(defn keywordize-keys [m]
  (into {} (for [[k v] m] [(keyword k) v])))

(defn starts-with [str prefix]
  (= (.indexOf str prefix) 0))

(defn dirname
  "Returns the absolute dirname of the given path"
  [path]
  (if (empty? path)
    "/"
    (let [idx (.lastIndexOf path "/")]
      (if (< idx 0)
        "/"
        (let [dir (subs path 0 (+ 1 idx))]
          (if (starts-with dir "/")
            dir
            (str "/" dir)))))))

(defn basename
  "Returns the basename of the given path"
  [path]
  (cond
   (empty? path) path
   (= (get path (- (count path) 1)) "/") (basename (subs path 0 (- (count path) 1)))
   :else (subs path (+ 1 (.lastIndexOf path "/")))))

(defn split-path [path]
  (filter #(not (empty? %)) (.split path "/")))

(defn normalize-vname [vname]
  (into {} (for [k [:corpus :root :path :signature :language]]
             [k (or (k vname) "")])))

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
