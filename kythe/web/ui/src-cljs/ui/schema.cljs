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
(ns ui.schema)

(def ^:const edge-prefix "/kythe/edge/")

(defn trim-edge-prefix [kind]
  (if (= 0 (.indexOf kind edge-prefix))
    (subs kind (count edge-prefix))
    kind))

(def ^:const node-kind-fact "/kythe/node/kind")

(def ^:const anchor-loc-filter "/kythe/loc/*")
(def ^:const anchor-start "/kythe/loc/start")
(def ^:const anchor-end "/kythe/loc/end")

(def ^:const text-fact "/kythe/text")

(def ^:const childof-edge "/kythe/edge/childof")
(def ^:const defines-edge "/kythe/edge/defines")
(def ^:const documents-edge "/kythe/edge/documents")
