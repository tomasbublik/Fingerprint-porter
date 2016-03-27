package cz.bublik.fingerprint.reader

/**
 * Created by tomas.bublik on 22.3.2016.
 */

import TwoWaySerialCommunication
import org.slf4j.Logger
import org.slf4j.LoggerFactory

fun main(args: Array<String>) {
    var logger: Logger = LoggerFactory.getLogger("cz.bublik.fingerprint.reader.MainKt");
    logger.debug("Hello world.");

    var twoWaySerialCommunication: TwoWaySerialCommunication = TwoWaySerialCommunication();
    //twoWaySerialComm.connect("/dev/ttyUSB0")
    twoWaySerialCommunication.connect("COM2")
}