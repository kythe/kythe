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
(ns ui.xrefs
  "View for XRefs pane"
  (:require [cljs.core.async :refer [put!]]
            [om.core :as om :include-macros true]
            [om.dom :as dom :include-macros true]
            [ui.util :refer [path-display]]))

(defn- xrefs-view [state owner]
  (reify
    om/IRenderState
    (render-state [_ {:keys [file-to-view]}]
      (apply dom/div #js {:id "xrefs"
                          :className "row border"
                          :style (when (or (:hidden state) (empty? state))
                                   #js {:display "none"})}
        (dom/button #js {:id "close-xrefs"
                         :className "close btn btn-danger btn-xs"
                         :onClick #(om/transact! state :hidden (constantly true))}
          (dom/span #js {:className "glyphicon glyphicon-remove"}))
        (cond
          (:hidden state) nil
          (:failure state) [(dom/p nil "Sorry, an error occurred!")
                            (dom/p nil (:original-text (:parse-error state)))]
          (:loading state) [(dom/span #js {:title (:loading state)}
                              "Loading xrefs... ")
                            (dom/span #js {:className "glyphicon glyphicon-repeat spinner"})]
          :else (map (fn [[kind refs]]
                       (dom/div nil
                         (dom/strong nil (subs (str kind) 1))
                         (apply dom/ul nil
                           (map #(dom/li nil
                                   (dom/a #js {:href "#"
                                               :onClick (fn [_]
                                                          (put! file-to-view
                                                            (assoc (:file %) :offset (:start %))))}
                                     (str (path-display (:file %))
                                       "[" (:start %) ":" (:end %) "]")))
                             (sort-by (juxt #(get-in % [:file :path]) :start) refs)))))
                  (sort-by first state)))))))
