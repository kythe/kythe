(defproject com.google/kythe-xrefs-ui "0.1.0"
  :description "Browser UI for a Kythe xrefs server. Decorates files with anchors for cross-references and node information"
  :url "https://kythe.io"
  :mailing-list {:name "Kythe Community"
                 :archive "https://groups.google.com/group/kythe"
                 :post "https://groups.google.com/group/kythe/post"
                 :subscribe "http://groups.google.com/group/kythe/boxsubscribe"
                 :unsubscribe "http://groups.google.com/group/kythe/boxsubscribe"}
  :dependencies [[org.clojure/clojure "1.7.0"]
                 [org.clojure/clojurescript "0.0-2371"
                  :exclusions [org.apache.ant/ant]]
                 [org.clojure/core.async "0.1.346.0-17112a-alpha"]
                 [om "0.8.0-alpha2"]
                 [cljs-ajax "0.3.3"]]
  :plugins [[lein-cljsbuild "1.1.0"]
            [lein-licenses "0.2.0"]]
  :hooks [leiningen.cljsbuild]
  :cljsbuild {
    :builds [{:id "prod"
              :compiler {:output-to "resources/public/js/main.js"
                         :optimizations :advanced
                         :preamble ["react/react.min.js"]
                         :externs ["react/react.js"]
                         :closure-warnings {:externs-validation :off
                                            :non-standard-jsdoc :off}}}
             {:id "dev"
              :compiler {:output-to "dev/js/main.js"
                         :output-dir "dev/js"
                         :optimizations :none
                         :source-map true}}]})
