package cz.bublik.fingerprint.reader

/**
 * Created by tomas.bublik on 22.3.2016.
 */

import TwoWaySerialComm
import org.slf4j.Logger
import org.slf4j.LoggerFactory

fun main(args: Array<String>) {
    var logger: Logger = LoggerFactory.getLogger("cz.bublik.fingerprint.reader.MainKt");
    logger.debug("Hello world.");

    var twoWaySerialComm: TwoWaySerialComm = TwoWaySerialComm();
    //twoWaySerialComm.connect("/dev/ttyUSB0")
    twoWaySerialComm.connect("COM2")
}