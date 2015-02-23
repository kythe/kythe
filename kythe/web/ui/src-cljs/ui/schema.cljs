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
(ns ui.schema)

(def node-kind-fact "/kythe/node/kind")

(def anchor-loc-filter "/kythe/loc/*")
(def anchor-start "/kythe/loc/start")
(def anchor-end "/kythe/loc/end")

(def text-fact "/kythe/text")

(def childof-edge "/kythe/edge/childof")

(defn filter-nodes-by-kind [kind nodes]
  "Returns the nodes of the given kind from the given collection"
  (filter (fn [[_ {k node-kind-fact}]] (= kind k)) nodes))
