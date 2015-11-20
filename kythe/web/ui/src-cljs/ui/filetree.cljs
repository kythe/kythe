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
(ns ui.filetree
  "View for filetree pane"
  (:require [cljs.core.async :refer [put!]]
            [om.core :as om :include-macros true]
            [om.dom :as dom :include-macros true]
            [ui.service :as service]
            [ui.util :refer [path-display basename]]))

(defn- dir-view [state owner {:keys [corpus root path name]}]
  (reify
    om/IInitState
    (init-state [_]
      {:expanded true :status nil :failure nil})
    om/IRenderState
    (render-state [_ {:keys [expanded status failure file-to-view]}]
      (dom/li nil
        (dom/a #js {:title (path-display corpus root path)
                    :href "#"
                    :onClick
                    (fn [e]
                      (.preventDefault e)
                      (condp = (om/get-state owner :status)
                        :loading nil
                        :loaded (om/update-state! owner :expanded not)
                        (do
                          (om/set-state! owner :status :loading)
                          (om/set-state! owner :failure nil)
                          (service/get-directory corpus root path
                            (fn [dir]
                              (om/transact! state :contents (constantly dir))
                              (om/set-state! owner :status :loaded))
                            (fn [err]
                              (om/set-state! owner :failure err)
                              (om/set-state! owner :status nil))))))}
          (str (or name (path-display corpus root path)) "/ ")
          (when failure
            (dom/span #js {:className "glyphicon glyphicon-exclamation-sign"
                           :title (or (:original-text (:parse-error failure))
                                    "Unknown error occurred")}))
          (when (= :state :loading)
            (dom/span #js {:className "glyphicon glyphicon-repeat spinner"})))
        (when (and expanded (not (empty? (:contents state))))
          (apply dom/ul #js {:className "nav nav-pills nav-stacked"}
            (concat
              (map (fn [[path dir]]
                     (om/build dir-view dir {:opts {:corpus corpus
                                                    :root root
                                                    :name (basename path)
                                                    :path path}
                                             :init-state {:file-to-view file-to-view}}))
                (sort-by first (:dirs (:contents state))))
              (map (fn [[name {:keys [vname ticket]}]]
                     (dom/li nil
                       (dom/a #js {:title (path-display vname)
                                   :href "#"
                                   :onClick (fn [e]
                                              (put! file-to-view (om/value ticket))
                                              (.preventDefault e))}
                         name)))
                (sort-by first (:files (:contents state)))))))))))

(defn filetree-view [state owner]
  (reify
    om/IRenderState
    (render-state [_ {:keys [file-to-view]}]
      (dom/div #js {:className "col-md-3 col-lg-2 border" :id "filetree-container"}
        (dom/h3 nil "Files")
        (if (:failure state)
          (dom/div nil
            (dom/p nil "Error getting corpus roots: " (dom/br nil) (:status-text state))
            (dom/p nil "Please "
              (dom/button #js {:className "btn btn-danger btn-xs"
                               :onClick #(.reload js/location)}
                "refresh")
              " the page to try again!"))
          (apply dom/ul #js {:className "nav nav-pills nav-stacked"}
            (map (fn [[{:keys [corpus root]} root-dir]]
                   (om/build dir-view root-dir {:opts {:corpus corpus
                                                       :root root
                                                       :path ""}
                                                :init-state {:file-to-view file-to-view}}))
              state)))))))
