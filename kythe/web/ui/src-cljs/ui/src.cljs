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
(ns ui.src
  "View for source pane"
  (:require [cljs.core.async :refer [put!]]
            [goog.crypt.base64 :as b64]
            [om.core :as om :include-macros true]
            [om.dom :as dom :include-macros true]
            [ui.schema :as schema]
            [ui.service :as service]
            [ui.util :refer [fix-encoding handle-ch]]))

(defn- overlay-anchors
  "Reduces each group of overlapping anchors into a series of non-overlapping anchors and concats
  the results together.  The resulting sequence of anchors are sorted by their positions."
  [anchors]
  (reduce (fn map-anchor [anchors n]
            (if (empty? anchors)
              (cons n anchors)
              (let [[first & rest] anchors]
                (cond
                 (>= (:start n) (:end first))         ;; n is past first
                 (cons first
                       (map-anchor rest n))
                 (<= (:end n) (:start first))         ;; n is before first
                 (cons n anchors)
                 (and (= (:start n) (:start first))   ;; n is the same as first
                      (= (:end n) (:end first)))
                 anchors
                 (< (:start n) (:start first))        ;; n overlaps first's start
                 (cons (assoc n :end (:start first)) anchors)
                 (> (:end n) (:end first))            ;; n overlap's first's end
                 (let [[next & _] rest]
                   (if (and next
                            (< (:start next) (:end n)))
                     anchors
                     (cons first
                           (cons (assoc n :start (:end first))
                                 rest))))
                 (= (:start n) (:start first))        ;; n is a prefix of first
                 (cons n (cons (assoc first :start (:end n))
                               rest))
                 (= (:end n) (:end first))            ;; n is a suffix of first
                 (cons (assoc first :end (:start n))
                       (cons n rest))
                 :else                                ;; n is contained within first
                 (cons (assoc first :end (:start n))
                       (cons n
                             (cons (assoc first :start (:end n))
                                   rest)))))))
          (list)
          ;; the order of anchors matter for the resulting sliced anchors sequence
          (sort-by (juxt :start :end) anchors)))

(defn- group-overlapping-anchors
  "Returns a sequence of sequences of overlapping anchors"
  [anchors]
  (let [[overlaps [leftover _ __]]
        (reduce (fn [[overlaps [anchors cur-start cur-end]] n]
                  (if (<= (:start n) cur-end)
                    [overlaps [(conj anchors n) cur-start (max cur-end (:end n))]]
                    [(conj overlaps anchors) [[n] (:start n) (:end n)]]))
                [[] [[] 0 0]]
                (sort-by :start anchors))]
    (drop-while empty?
                (if (empty? leftover)
                  overlaps
                  (conj overlaps leftover)))))

(defn- count-lines [text] (count (.split text "\n")))

(defn construct-decorations
  "Returns a seq of precomputed text/anchors to display as the source-text"
  [decorations]
  (let [src (b64/decodeString (:source_text decorations))
        definitions (:definition_locations decorations)
        anchors (mapcat overlay-anchors
                  (group-overlapping-anchors
                    (filter (fn [{:keys [start end kind]}]
                              (and (> end 0) (< start end)))
                            (map (fn [{:keys [source_ticket target_ticket anchor_start anchor_end kind target_definition]}]
                                   (let [def (when target_definition
                                               (get definitions (keyword target_definition)))]
                                     {:start (:byte_offset anchor_start)
                                      :end (:byte_offset anchor_end)
                                      :target-definition def
                                      :kind (schema/trim-edge-prefix kind)
                                      :anchor-ticket source_ticket
                                      :target-ticket target_ticket}))
                                 (filter #(not (contains? #{schema/documents-edge schema/defines-edge} (:kind %)))
                          (:reference decorations))))))]
    {:source-text src
     :num-lines (count-lines src)
     :nodes
     (loop [mark 0
            anchors anchors
            nodes []]
       (if (empty? anchors)
         (conj nodes (fix-encoding (subs src mark)))
         (let [{:keys [:start :end] :as anchor} (first anchors)]
           (if (< start mark)
             (do
               (.log js/console (str "Overlapping anchor at offest " start))
               (recur mark (rest anchors) nodes))
             (recur end (rest anchors)
               (conj (if (> start mark)
                       (conj nodes (fix-encoding (subs src mark start)))
                       nodes)
                 (assoc anchor :text (fix-encoding (subs src start end)))))))))}))

(defn- src-node-view [state owner]
  (reify
    om/IRenderState
    (render-state [_ {:keys [file-to-view xrefs-to-view hover]}]
      (let [{:keys [start end anchor-ticket target-ticket target-definition kind text background]} state]
        (if anchor-ticket
          (dom/a #js {:title (str kind " => " target-ticket)
                      :href "#"
                      :onClick (fn [e]
                                 (.preventDefault e)
                                 (if (and target-definition (.-ctrlKey e))
                                   ;; Jump-to-definition (<Ctrl>-<click>)
                                   (do
                                     (.log js/console (str "Jumping to definition: " (:ticket target-definition)))
                                     (put! file-to-view
                                           {:ticket (:parent target-definition)
                                            :anchor (:ticket target-definition)
                                            :line   (:line_number (:start target-definition))}))
                                   ;; Open cross-references panel
                                   (put! xrefs-to-view target-ticket)))
                      :style #js {:backgroundColor background}
                      :onMouseEnter #(put! hover
                                       {:ticket anchor-ticket :target target-ticket})
                      :onMouseLeave #(put! hover {})}
            text)
          (dom/span nil state))))))

(defn- decorations-view [state owner]
  (reify
    om/IWillMount
    (will-mount [_]
      (handle-ch (om/get-state owner :hover)
        (fn [{:keys [ticket target xref-jump]}]
          (om/transact! state :nodes
            (fn [nodes]
              (map (fn [node]
                     (if (:anchor-ticket node)
                       (cond
                         (= xref-jump (:anchor-ticket node))
                         (assoc node :background "lightgreen")
                         (= ticket (:anchor-ticket node))
                         (assoc node :background "lightgray")
                         (= target (:target-ticket node))
                         (assoc node :background "yellow")
                         (:background node) (dissoc node :background)
                         :else node)
                       node))
                nodes))))))
    om/IRenderState
    (render-state [_ {:keys [xrefs-to-view file-to-view hover]}]
      (dom/div #js {:className "col-md-9 col-lg-10" :id "src-container"}
        (apply dom/pre #js {:id "fringe"}
          (mapcat (fn [i] [(dom/a #js {:id (str "line" i)
                                       :href "#"
                                       :onClick #(let [el (.-target %)]
                                                   (.scrollIntoView el true)
                                                   (.focus el)
                                                   (.preventDefault %))}
                             (str i)) "\n"])
            (range 1 (:num-lines state))))
        (dom/pre nil
          (apply dom/code #js {:id "src"}
            (if (empty? (:source-text state))
              nil
              (om/build-all src-node-view (:nodes state)
                {:init-state {:xrefs-to-view xrefs-to-view
                              :file-to-view file-to-view
                              :hover hover}}))))))))

(defn line-in-string [text offset] (count (.split (subs text 0 offset) "\n")))

(defn src-view [state owner]
  (reify
    om/IRenderState
    (render-state [_ {:keys [file-to-view xrefs-to-view hover]}]
      (cond
        (empty? state) (dom/span nil "Please select a file to your left!")
        (:failure state) (dom/div nil
                           (dom/p nil (str "Sorry, an error occurred while requesting file decorations for ticket: "
                                        "\"" (:requested-file state) "\""))
                           (dom/p nil (:original-text (:parse-error state))))
        (:loading state) (dom/div nil
                           (dom/span nil "Loading...")
                           (dom/span #js {:className "glyphicon glyphicon-repeat spinner"}))
        :else (om/build decorations-view (:decorations state)
                {:init-state {:xrefs-to-view xrefs-to-view
                              :file-to-view file-to-view
                              :hover hover}})))
    om/IDidUpdate
    (did-update [_ __ ___]
      (when-let [line-num (:line state)]
        ;; Scroll to line in source
        (.click (. js/document (getElementById (str "line" line-num))))
        (om/transact! state :line (constantly nil))))))
