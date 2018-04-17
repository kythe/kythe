FROM openjdk:8 as java
FROM gcr.io/kythe_repo/kythe-javac-extractor as javac-extractor
FROM maven:latest as maven
FROM debian:jessie

VOLUME /repo
ENV KYTHE_ROOT_DIRECTORY=/repo
VOLUME /out
ENV KYTHE_OUTPUT_DIRECTORY=/out
WORKDIR /repo

COPY --from=java /docker-java-home /docker-java-home
ENV JAVA_HOME=/docker-java-home
ENV PATH=$JAVA_HOME/bin:$PATH

COPY --from=javac-extractor /opt/kythe/extractors/javac-wrapper.sh /opt/kythe/extractors/javac-wrapper.sh
COPY --from=javac-extractor /opt/kythe/extractors/javac_extractor.jar /opt/kythe/extractors/javac_extractor.jar
ENV REAL_JAVAC=$JAVA_HOME/bin/javac
ENV JAVAC_EXTRACTOR_JAR=/opt/kythe/extractors/javac_extractor.jar
ENV JAVAC_WRAPPER=/opt/kythe/extractors/javac-wrapper.sh

COPY --from=maven /usr/share/maven /usr/share/maven
ENV MAVEN_HOME=/usr/share/maven
ENV PATH=$MAVEN_HOME/bin:$PATH
ENV MAVEN_OPTS="-Dmaven.compiler.forceJavacCompilerUse=true -Dmaven.compiler.fork=true -Dmaven.compiler.executable=$JAVAC_WRAPPER"
ENTRYPOINT ["mvn", "clean", "compile"]
