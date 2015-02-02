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
(ns ui.core
  "Main UI view and glue"
  (:require [cljs.core.async :refer [chan]]
            [om.core :as om :include-macros true]
            [om.dom :as dom :include-macros true]
            [ui.filetree :refer [filetree-view]]
            [ui.service :as service]
            [ui.src :refer [src-view construct-decorations line-in-string]]
            [ui.util :refer [handle-ch]]
            [ui.xrefs :refer [xrefs-view]]))

(defn- replace-state! [state key]
  #(om/transact! state key (constantly %)))

(defn- kythe-app [state owner]
  (reify
    om/IInitState
    (init-state [_]
      {:file-to-view (chan)
       :xrefs-to-view (chan)})
    om/IWillMount
    (will-mount [_]
      (handle-ch (om/get-state owner :file-to-view) nil
        (fn [file last-ticket]
          (let [file (if (:ticket file) file {:ticket file})
                ticket (:ticket file)
                offset (:offset file)]
           (cond
             (not= ticket last-ticket)
             (do
               (om/transact! state :current-file (constantly {:loading true}))
               (service/get-file ticket
                 (fn [decorations]
                   (let [decorations (construct-decorations decorations)
                         scroll-to-line (when offset (line-in-string (:source-text decorations) offset))]
                     (om/transact! state :current-file (constantly
                                                         {:line scroll-to-line
                                                          :decorations decorations}))))
                 (replace-state! state :current-file)))
             offset (om/transact! state :current-file
                      (fn [file]
                        (assoc file
                          :line (line-in-string (:source-text (:decorations file)) offset)))))
           ticket)))
      (handle-ch (om/get-state owner :xrefs-to-view)
        (fn [target-ticket]
          (om/transact! state :current-xrefs (constantly {:loading target-ticket}))
          (service/get-xrefs target-ticket
            (replace-state! state :current-xrefs)
            (replace-state! state :current-xrefs))))
      (service/get-corpus-roots
        (fn [corpus-roots]
          (om/transact! state :files
            (fn [_]
              (into {}
                (map (fn [cr] [cr {:contents {}}])
                  (mapcat (fn [[corpus roots]]
                            (map (fn [root]
                                   {:corpus corpus
                                    :root (if (empty? root) nil root)})
                              roots))
                    corpus-roots))))))
        (replace-state! state :files)))
    om/IRenderState
    (render-state [_ {:keys [file-to-view xrefs-to-view]}]
      (dom/div #js {:id "container"}
        (dom/div #js {:className "container-row"}
          (om/build filetree-view (:files state)
            {:init-state {:file-to-view file-to-view}})
          (om/build src-view (:current-file state)
            {:init-state {:xrefs-to-view xrefs-to-view}}))
        (om/build xrefs-view (:current-xrefs state)
          {:init-state {:file-to-view file-to-view}})))))

(def ^:private kythe-state (atom {:current-file {}
                                  :current-xrefs {}}))

(om/root kythe-app kythe-state
  {:target (.getElementById js/document "kythe")})
