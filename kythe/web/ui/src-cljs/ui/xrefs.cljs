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
            [ui.schema :as schema]
            [ui.service :as service]
            [ui.util :refer [fix-encoding]]))

(defn- fact-value [nodes ticket fact-name]
  (get (get nodes ticket) fact-name))

(defn- node-kind [nodes ticket]
  (or (fact-value nodes ticket schema/node-kind-fact) "UNKNOWN"))

(defn- line-for-offset [text offset]
  (let [i (inc (.lastIndexOf text "\n" offset))]
    [(count (.split (.substr text 0 i) "\n"))
     (fix-encoding (.substr text i (- (.indexOf text "\n" offset) i)))]))

(defn- display-node [file-to-view xrefs-to-view nodes anchor-info ticket with-link]
  (dom/div nil
    (let [kind (node-kind nodes ticket)
          file-ticket (:file (get anchor-info ticket))]
      (if (and (= kind "anchor") file-ticket)
        (dom/a #js {:href "#"
                    :onClick (fn [e]
                               (.preventDefault e)
                               (put! file-to-view
                                 {:ticket file-ticket
                                  :anchor ticket
                                  :offset (fact-value nodes ticket schema/anchor-start)}))}
          kind)
        kind))
    " "
    (if with-link
      (dom/a #js {:href "#"
                  :onClick (fn [e]
                             (.preventDefault e)
                             (put! xrefs-to-view ticket))}
        ticket)
      ticket)))

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
          :else (do
                  (let [nodes (:nodes state)
                        anchors (for [grp (:group (:edge_set state))
                                      ticket (:target_ticket grp)
                                      :when (= (node-kind nodes ticket) "anchor")]
                                  ticket)
                        anchor-info-id (str
                                      (:source_ticket (:edge_set state))
                                      (:current-page-token state))]
                    (when (and (not= (:id anchor-info) anchor-info-id) (seq anchors))
                      (service/get-edges anchors
                        {:kind [schema/childof-edge]
                         :filter [schema/text-fact
                                  schema/anchor-start
                                  schema/node-kind-fact]
                         :page_size 64}
                        (fn [edges]
                          (let [anchor-info  (into {}
                                            (for [es (:edge_set edges)
                                                  :let [nodes (:nodes edges)
                                                        file (first (:target_ticket (first (:group es))))]
                                                  :when (= (node-kind nodes file) "file")
                                                  :let [text (fact-value nodes file schema/text-fact)
                                                        anchor (:source_ticket es)
                                                        offset (js/parseInt (fact-value nodes anchor schema/anchor-start))
                                                        [line-num snippet] (line-for-offset text offset)]]
                                              [anchor {:line line-num
                                                       :offset offset
                                                       :snippet snippet
                                                       :file file}]))]
                            (om/set-state! owner :anchor-info (assoc anchor-info :id anchor-info-id))))
                        #(.log js/console (str "Error getting childof edges " %)))))

                  [(dom/strong nil (display-node file-to-view xrefs-to-view (:nodes state) anchor-info (:source_ticket (:edge_set state)) false))
                   (apply dom/ul nil
                     (map (fn [grp]
                            (dom/li nil
                              (dom/strong nil (:kind grp))
                              (apply dom/ul nil
                                (map (fn [target]
                                       (dom/li nil
                                         (display-node file-to-view xrefs-to-view (:nodes state) anchor-info target true)
                                         (when-let [{:keys [line snippet]} (get anchor-info target)]
                                           (dom/p #js {:className "snippet"} (str line ": " snippet)))))
                                  (sort-by (fn [t]
                                             [(:file (get anchor-info t))
                                              (:offset (get anchor-info t))
                                              (node-kind (:nodes state) t)])
                                    (:target_ticket grp))))))
                       (sort-by :kind (:group (:edge_set state)))))
                   (page-navigation xrefs-to-view (cons "" (:pages state)) (or (:current-page-token state) "") (:source_ticket (:edge_set state)))]))))))
