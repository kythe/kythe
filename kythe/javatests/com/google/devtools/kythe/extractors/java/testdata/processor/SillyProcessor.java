package processor;

import java.io.IOException;
import java.io.Writer;
import java.util.Collections;
import java.util.Set;
import javax.annotation.processing.Completion;
import javax.annotation.processing.ProcessingEnvironment;
import javax.annotation.processing.Processor;
import javax.annotation.processing.RoundEnvironment;
import javax.lang.model.SourceVersion;
import javax.lang.model.element.AnnotationMirror;
import javax.lang.model.element.Element;
import javax.lang.model.element.ExecutableElement;
import javax.lang.model.element.TypeElement;
import javax.tools.JavaFileObject;

/**
 * A silly annotation processor that generates source for SillyGenerator
 * when invoked.
 */
public class SillyProcessor implements Processor {
  private static final String[] source = {
    "package processor;",
    "public class SillyGenerated {",
    "  boolean isSilly() {",
    "    return true;",
    "  }",
    "}",
  };

  @Override
  public void init(ProcessingEnvironment procEnv) {
    try {
      JavaFileObject jfo = procEnv.getFiler().createSourceFile("processor.SillyGenerated");
      try (Writer writer = jfo.openWriter()) {
        for (String line : source) {
          writer.append(line).append("\n");
        }
      }
    } catch (IOException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public boolean process(Set<? extends TypeElement> annotations, RoundEnvironment roundEnv) {
    return false;
  }

  @Override
  public Iterable<Completion> getCompletions(Element element, AnnotationMirror annotation,
      ExecutableElement member, String userText) {
    return Collections.emptyList();
  }

  @Override
  public Set<String> getSupportedAnnotationTypes() {
    return Collections.singleton("processor.Silly");
  }

  @Override
  public Set<String> getSupportedOptions() {
    return Collections.emptySet();
  }

  @Override
  public SourceVersion getSupportedSourceVersion() {
    return SourceVersion.latest();
  }
}
