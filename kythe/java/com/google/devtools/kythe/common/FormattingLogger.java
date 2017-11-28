/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.devtools.kythe.common;

import com.google.common.collect.ImmutableSet;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.LogRecord;
import java.util.logging.Logger;

public class FormattingLogger {
  private final Logger logger;

  public FormattingLogger() {
    this(Logger.getAnonymousLogger());
  }

  public FormattingLogger(Class<?> cls) {
    String suffix = "";
    while (cls.isAnonymousClass()) {
      suffix = ".<anonymous_class>" + suffix;
      if (cls.getEnclosingMethod() != null) {
        suffix = "." + cls.getEnclosingMethod().getName() + suffix;
      }
      cls = cls.getEnclosingClass();
    }
    this.logger = Logger.getLogger(cls.getCanonicalName() + suffix);
    this.logger.addHandler(SetSourceHandler.INSTANCE);
  }

  public FormattingLogger(Logger logger) {
    this.logger = logger;
  }

  public static FormattingLogger getLogger(Class<?> cls) {
    return new FormattingLogger(cls);
  }

  public void finestfmt(String message, Object... args) {
    logfmt(Level.FINEST, message, args);
  }

  public void finerfmt(String message, Object... args) {
    logfmt(Level.FINER, message, args);
  }

  public void finefmt(String message, Object... args) {
    logfmt(Level.FINE, message, args);
  }

  public void infofmt(String message, Object... args) {
    logfmt(Level.INFO, message, args);
  }

  public void configfmt(String message, Object... args) {
    logfmt(Level.CONFIG, message, args);
  }

  public void warningfmt(String message, Object... args) {
    logfmt(Level.WARNING, message, args);
  }

  public void severefmt(String message, Object... args) {
    logfmt(Level.SEVERE, message, args);
  }

  public void finestfmt(Throwable thrown, String message, Object... args) {
    logfmt(Level.FINEST, thrown, message, args);
  }

  public void finerfmt(Throwable thrown, String message, Object... args) {
    logfmt(Level.FINER, thrown, message, args);
  }

  public void finefmt(Throwable thrown, String message, Object... args) {
    logfmt(Level.FINE, thrown, message, args);
  }

  public void configfmt(Throwable thrown, String message, Object... args) {
    logfmt(Level.CONFIG, thrown, message, args);
  }

  public void warningfmt(Throwable thrown, String message, Object... args) {
    logfmt(Level.WARNING, thrown, message, args);
  }

  public void infofmt(Throwable thrown, String message, Object... args) {
    logfmt(Level.INFO, thrown, message, args);
  }

  public void severefmt(Throwable thrown, String message, Object... args) {
    logfmt(Level.SEVERE, thrown, message, args);
  }

  public void finest(Throwable thrown, String message) {
    logger.log(Level.FINEST, message, thrown);
  }

  public void finer(Throwable thrown, String message) {
    logger.log(Level.FINER, message, thrown);
  }

  public void fine(Throwable thrown, String message) {
    logger.log(Level.FINE, message, thrown);
  }

  public void config(Throwable thrown, String message) {
    logger.log(Level.CONFIG, message, thrown);
  }

  public void warning(Throwable thrown, String message) {
    logger.log(Level.WARNING, message, thrown);
  }

  public void info(Throwable thrown, String message) {
    logger.log(Level.INFO, message, thrown);
  }

  public void severe(Throwable thrown, String message) {
    logger.log(Level.SEVERE, message, thrown);
  }

  public void finest(String message) {
    logger.finest(message);
  }

  public void finer(String message) {
    logger.finer(message);
  }

  public void fine(String message) {
    logger.fine(message);
  }

  public void config(String message) {
    logger.config(message);
  }

  public void warning(String message) {
    logger.warning(message);
  }

  public void info(String message) {
    logger.info(message);
  }

  public void severe(String message) {
    logger.severe(message);
  }

  private void logfmt(Level level, String message, Object... args) {
    // Check the log level before logging, to avoid spurious formatted string generation.
    if (logger.isLoggable(level)) {
      logger.log(level, String.format(message, args));
    }
  }

  private void logfmt(Level level, Throwable thrown, String message, Object... args) {
    // Check the log level before logging, to avoid spurious formatted string generation.
    if (logger.isLoggable(level)) {
      logger.log(level, String.format(message, args), thrown);
    }
  }

  /** Simple handler that sets the class and method name of each {@link LogRecord}. */
  private static class SetSourceHandler extends Handler {
    private static final SetSourceHandler INSTANCE = new SetSourceHandler();

    // Classes that appear at the tail of a stack trace coming from #publish(LogRecord).
    private static final ImmutableSet<String> LOGGER_CLASSES =
        ImmutableSet.of(
            "com.google.devtools.kythe.common.FormattingLogger$SetSourceHandler",
            "java.util.logging.Logger",
            "com.google.devtools.kythe.common.FormattingLogger");

    @Override
    public void publish(LogRecord record) {
      if (record.getSourceClassName() == null
          || record.getSourceMethodName() == null
          || LOGGER_CLASSES.contains(record.getSourceClassName())) {
        Throwable t = record.getThrown();
        if (t == null || t.getStackTrace().length == 0) {
          t = new Throwable();
        }
        for (StackTraceElement e : t.getStackTrace()) {
          String className = e.getClassName();
          if (!LOGGER_CLASSES.contains(className)) {
            record.setSourceClassName(className);
            record.setSourceMethodName(e.getMethodName());
            break;
          }
        }
      }
    }

    @Override
    public void flush() {}

    @Override
    public void close() {}
  }
}
