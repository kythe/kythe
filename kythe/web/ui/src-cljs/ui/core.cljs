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
  (:require [cljs.core.async :refer [chan put!]]
            [om.core :as om :include-macros true]
            [om.dom :as dom :include-macros true]
            [ui.filetree :refer [filetree-view]]
            [ui.schema :as schema]
            [ui.service :as service]
            [ui.src :refer [src-view construct-decorations line-in-string]]
            [ui.util :refer [handle-ch parse-url-state set-url-state ticket->vname vname->ticket]]
            [ui.xrefs :refer [xrefs-view]]))

(defn- replace-state! [state key]
  #(om/transact! state key (constantly %)))

(defn- kythe-app [state owner]
  (reify
    om/IInitState
    (init-state [_]
      {:file-to-view (chan)
       :xrefs-to-view (chan)
       :hover (chan)})
    om/IWillMount
    (will-mount [_]

      ;; Restore page state based on URL initially given
      (let [state (parse-url-state)
            file (select-keys state [:path :corpus :root])]
        (when-not (empty? file)
          (put! (om/get-state owner :file-to-view)
            (assoc (select-keys state [:offset :line])
              :ticket (vname->ticket file)))))

      ;; Handle all jump requests to files and anchors within a file
      (handle-ch (om/get-state owner :file-to-view) nil
        (fn [file last-ticket]
          (let [file (if (:ticket file) file {:ticket file})
                ticket (:ticket file)
                offset (:offset file)
                line (:line file)
                anchor (:anchor file)]
            (cond
              (not= ticket last-ticket)
              (do
                (om/transact! state :current-file (constantly {:loading true}))
                (service/get-file ticket
                  (fn [decorations]
                    (let [decorations (construct-decorations decorations)
                          scroll-to-line (cond
                                           line line
                                           offset (line-in-string (:source-text decorations) offset))]
                      (set-url-state
                        (assoc (ticket->vname ticket)
                          :line scroll-to-line
                          :offset offset))
                      (om/transact! state :current-file (constantly
                                                          {:line scroll-to-line
                                                           :decorations decorations}))
                      (put! (om/get-state owner :hover) {:xref-jump anchor})))
                  #(om/transact! state :current-file (constantly (assoc % :requested-file ticket)))))
              (or line offset)
              (do
                (set-url-state
                  (assoc (ticket->vname ticket) :offset offset :line line))
                (om/transact! state :current-file
                  (fn [file]
                    (assoc file :line (or line (line-in-string (:source-text (:decorations file)) offset)))))
                (put! (om/get-state owner :hover) {:xref-jump anchor})))
            ticket)))

      ;; Handle all requests for the xrefs pane
      (handle-ch (om/get-state owner :xrefs-to-view)
        (fn [target]
          (let [target (if (:ticket target) target {:ticket target})
                page-token (:page_token target)]
            (om/transact! state :current-xrefs #(assoc % :loading (:ticket target)))
            (service/get-xrefs (:ticket target) target
              (fn [xrefs]
                (om/transact! state :current-xrefs
                  (fn [prev]
                    (let [prev-pages (if (= (:ticket (:cross-references prev)) (:ticket (:cross-references xrefs)))
                                       (:pages prev)
                                       [])]
                      (assoc xrefs :current-page-token page-token
                        :pages
                        (cond
                          (not (:next xrefs)) prev-pages
                          (and page-token (= (last prev-pages) page-token))
                          (conj prev-pages (:next xrefs))
                          (some #{(:next xrefs)} prev-pages)
                          prev-pages
                          :else
                          [(:next xrefs)]))))))
              (replace-state! state :current-xrefs)))))

      ;; Populate the initial filetree data
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
    (render-state [_ {:keys [file-to-view xrefs-to-view hover]}]
      (dom/div #js {:id "container"}
        (dom/div #js {:className "container-row"}
          (om/build filetree-view (:files state)
            {:init-state {:file-to-view file-to-view}})
          (om/build src-view (:current-file state)
            {:init-state {:xrefs-to-view xrefs-to-view
                          :file-to-view file-to-view
                          :hover hover}}))
        (om/build xrefs-view (:current-xrefs state)
          {:init-state {:xrefs-to-view xrefs-to-view
                        :file-to-view file-to-view}})))))

(def ^:private kythe-state (atom {:current-file {}
                                  :current-xrefs {}}))

(om/root kythe-app kythe-state
  {:target (.getElementById js/document "kythe")})
