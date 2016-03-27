package cz.bublik.fingerprint.reader.serial

/**
 * Created by tomas.bublik on 25.3.2016.
 */

interface SerialCommunication {
    @Throws(Exception::class)
    fun connect(portName: String)
}