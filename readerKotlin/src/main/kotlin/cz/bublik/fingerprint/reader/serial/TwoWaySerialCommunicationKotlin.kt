package cz.bublik.fingerprint.reader.serial

import gnu.io.CommPortIdentifier
import gnu.io.SerialPort
import java.io.ByteArrayInputStream
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream

/**
 * Created by tomas.bublik on 25.3.2016.
 */
class TwoWaySerialCommunicationKotlin {
    @Throws(Exception::class)
    fun connect(portName: String) {
        val portIdentifier = CommPortIdentifier.getPortIdentifier(portName)
        if (portIdentifier.isCurrentlyOwned) {
            println("Error: Port is currently in use")
        } else {
            val timeout = 2000
            val commPort = portIdentifier.open(this.javaClass.name, timeout)

            if (commPort is SerialPort) {
                commPort.setSerialPortParams(115200,
                        SerialPort.DATABITS_8,
                        SerialPort.STOPBITS_1,
                        SerialPort.PARITY_NONE)

                val inputStream = commPort.inputStream
                val out = commPort.outputStream

                Thread(SerialReader(inputStream)).start()
                Thread(SerialWriter(out)).start()

            } else {
                println("Error: Only serial ports are handled by this example.")
            }
        }
    }

    class SerialReader(internal var inputStream: InputStream) : Runnable {

        override fun run() {
            val buffer = ByteArray(1024)
            var len = -1
            try {
                while (len > -1) {
                    len = this.inputStream.read(buffer)
                    print(String(buffer, 0, len))
                }
            } catch (e: IOException) {
                e.printStackTrace()
            }

        }
    }

    class SerialWriter(internal var out:

                       OutputStream) : Runnable {
        internal var source = byteArrayOf(0xEF.toByte(), 0x01, 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0x01, 0x00, 0x03, 0x01, 0x00, 0x05)
        internal var bis = ByteArrayInputStream(source)

        override fun run() {
            try {
                var c = 0
                /*
                while ( (c = System.in.read()) >-1){
                    this.out.write(c);
                }
*/
                while (c > -1) {
                    c = bis.read()
                    this.out.write(c)
                }

            } catch (e: IOException) {
                e.printStackTrace()
            }

        }
    }

    companion object {

        @JvmStatic fun main(args: Array<String>) {
            try {
                TwoWaySerialCommunicationKotlin().connect("/dev/ttyUSB0")
            } catch (e: Exception) {
                e.printStackTrace()
            }

        }
    }

}
