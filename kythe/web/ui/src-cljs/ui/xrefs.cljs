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
            [clojure.string :refer [trim]]
            [goog.crypt.base64 :as b64]
            [goog.string :as gstring]
            [goog.string.format]
            [om.core :as om :include-macros true]
            [om.dom :as dom :include-macros true]
            [ui.schema :as schema]
            [ui.service :as service]
            [ui.util :refer [ticket->vname]]))

(defn- page-navigation [xrefs-to-view pages current-page-token ticket]
  (let [current-page (first (first (filter #(= (second %) current-page-token) (map-indexed (fn [i t] [i t]) pages))))
        prev-page (when (> current-page 0)
                    (nth pages (dec current-page)))
        next-page (when (< (inc current-page) (count pages))
                    (nth pages (inc current-page)))]
   (dom/nav nil
     (apply dom/ul #js {:className "pagination pagination-sm"}
       (concat
         [(dom/li #js {:className (when-not prev-page "disabled")}
            (dom/a #js {:href "#"
                        :ariaLabel "Previous"
                        :onClick (when prev-page
                                   (fn [e]
                                     (.preventDefault e)
                                     (put! xrefs-to-view {:ticket ticket
                                                          :page_token prev-page})))}
              "<"))]
         (map-indexed (fn [page token]
                        (dom/li #js {:className (when (= token current-page-token)
                                                  "active")}
                          (dom/a #js {:href "#"
                                      :onClick (fn [e]
                                                 (.preventDefault e)
                                                 (put! xrefs-to-view {:ticket ticket
                                                                      :page_token token}))}
                            (str (inc page)))))
           pages)
         [(dom/li #js {:className (when-not next-page "disabled")}
            (dom/a #js {:href "#"
                        :ariaLabel "Next"
                        :onClick (when next-page
                                   (fn [e]
                                     (.preventDefault e)
                                     (put! xrefs-to-view {:ticket ticket
                                                          :page_token next-page})))}
              ">"))])))))

(defn- collect-anchors [anchors]
  (into {}
    (for [[file anchors] (group-by :parent anchors)]
      [file (sort-by (comp :byte_offset :start) anchors)])))

(defn- display-anchor [file anchor file-to-view]
  (let [line (:line_number (:snippet_start anchor))
        snippet (:snippet anchor)]
    (when (and line snippet)
     (dom/p #js {:className "snippet"}
       (dom/a #js {:href "#"
                   :title (gstring/format "[%d:%d-%d:%d) %s %s"
                            line (:column_offset (:start anchor))
                            (:line_number (:end anchor)) (:column_offset (:end anchor))
                            (:kind anchor)
                            (:text anchor))
                   :onClick (fn [e]
                              (.preventDefault e)
                              (put! file-to-view
                                {:ticket file
                                 :anchor (:ticket anchor)
                                 :line line}))}
         (str line))
       ;; TODO(schroederc): narrow/expand multi-line snippets
       ": " (dom/span nil (trim snippet))))))

(defn- display-related-anchors [title related-anchors file-to-view]
  (when related-anchors
    (dom/ul nil
      (dom/li nil
        (dom/strong nil title)
        (apply dom/ul nil
          (map (fn [[file anchors]]
                 (apply dom/li nil
                   (:path (ticket->vname file))
                   (filter identity (map (fn [anchor] (display-anchor file anchor file-to-view)) anchors))))
            (collect-anchors (map :anchor related-anchors))))))))

(defn- xrefs-view [state owner]
  (reify
    om/IRenderState
    (render-state [_ {:keys [file-to-view xrefs-to-view anchor-info]}]
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
          :else
          [(dom/strong nil (:ticket (:cross-references state)))
           (display-related-anchors "Definitions:" (:definition (:cross-references state)) file-to-view)
           (display-related-anchors "Declarations:" (:declaration (:cross-references state)) file-to-view)
           (display-related-anchors "Documentation:" (:documentation (:cross-references state)) file-to-view)
           (display-related-anchors "References:" (:reference (:cross-references state)) file-to-view)
           (when (:related_node (:cross-references state))
             (dom/ul nil
               (dom/li nil
                 (dom/strong nil "Related Nodes:")
                 (apply dom/ul nil
                   (map (fn [[relation nodes]]
                          (dom/li nil
                            (dom/strong nil relation) " "
                            (apply dom/ul nil
                              (map (fn [{:keys [ticket]}]
                                     (dom/li nil
                                       (str
                                         (or
                                           (get-in (:nodes @state) [(keyword ticket)
                                                                    :facts
                                                                    (keyword schema/node-kind-fact)])
                                           "UNKNOWN")
                                         " ")
                                       (dom/a #js {:href "#"
                                                   :onClick (fn [e]
                                                              (.preventDefault e)
                                                              (put! xrefs-to-view ticket))}
                                         ticket)))
                                nodes))))
                     (group-by :relation_kind (:related_node (:cross-references state))))))))
           (page-navigation xrefs-to-view (cons "" (:pages state)) (or (:current-page-token state) "") (:ticket (:cross-references state)))])))))
