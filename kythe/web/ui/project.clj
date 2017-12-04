(defproject com.google/kythe-xrefs-ui "0.1.0"
  :description "Browser UI for a Kythe xrefs server. Decorates files with anchors for cross-references and node information"
  :url "https://kythe.io"
  :mailing-list {:name "Kythe Community"
                 :archive "https://groups.google.com/group/kythe"
                 :post "https://groups.google.com/group/kythe/post"
                 :subscribe "http://groups.google.com/group/kythe/boxsubscribe"
                 :unsubscribe "http://groups.google.com/group/kythe/boxsubscribe"}
  :dependencies [[org.clojure/clojure "1.8.0"]
                 [org.clojure/clojurescript "1.9.946"
                  :exclusions [org.apache.ant/ant]]
                 [org.clojure/core.async "0.3.465"]
                 [org.omcljs/om "0.9.0"]
                 [cljs-ajax "0.7.3"]]
  :plugins [[lein-cljsbuild "1.1.1"]
            [lein-licenses "0.2.0"]]
  :hooks [leiningen.cljsbuild]
  :cljsbuild {
    :builds [{:id "prod"
              :compiler {:output-to "resources/public/js/main.js"
                         :output-dir "target"
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
