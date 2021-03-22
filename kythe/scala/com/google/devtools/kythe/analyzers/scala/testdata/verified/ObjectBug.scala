package com.nest.insights.common.util

import scala.collection.JavaConverters._
import scala.util.Try

/**
 * Object containing common avro to Scala parsing routines
 */
object InsightsAvroParser {

  def javaCharSequenceToScalaString(x : java.lang.CharSequence) = {
    if (x == null) "" else x.toString
  }

  def javaDoubleListToScalaDoubleList(x : java.util.List[java.lang.Double]) = {
    if (x == null) List[Double]() else x.asScala.map(_.asInstanceOf[Double]).toList
  }

  /**
   *
   * Parses OccupancySummary data field magic string. Wrapped in a Try becomes sometimes there's garbage in the string
   * (a known issue with 4.2.5).
   * @param line: OccupancySummary data magic string
   * @return list of doubles
   */
  def parseOccupancyData(line: String): Try[List[Double]] = {

    Try(
      line.stripPrefix("[").stripSuffix("]").split(",").map(occupancyTimeSeries =>
        occupancyTimeSeries.toDouble).toList
    )
  }
}
