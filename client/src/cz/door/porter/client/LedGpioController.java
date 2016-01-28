package cz.door.porter.client;


import com.pi4j.io.gpio.*;

/**
 * Created by tomas.bublik on 10.9.2015.
 */
public class LedGpioController {

    public static void main( String args[] )
    {
        // create gpio controller instance
        final GpioController gpio = GpioFactory.getInstance();

        // provision gpio pin #02 as an input pin with its internal pull down resistor enabled
        // (configure pin edge to both rising and falling to get notified for HIGH and LOW state
        // changes)
        GpioPinDigitalInput myButton = gpio.provisionDigitalInputPin(RaspiPin.GPIO_02,             // PIN NUMBER
                "MyButton",                   // PIN FRIENDLY NAME (optional)
                PinPullResistance.PULL_DOWN); // PIN RESISTANCE (optional)

        // provision gpio pins #04 as an output pin and make sure is is set to LOW at startup
        GpioPinDigitalOutput myLed = gpio.provisionDigitalOutputPin(RaspiPin.GPIO_04,   // PIN NUMBER
                "My LED",           // PIN FRIENDLY NAME (optional)
                PinState.LOW);      // PIN STARTUP STATE (optional)

        // explicitly set a state on the pin object
        myLed.setState(PinState.HIGH);

        // use convenience wrapper method to set state on the pin object
        myLed.low();
        myLed.high();

        // use toggle method to apply inverse state on the pin object
        myLed.toggle();

        // use pulse method to set the pin to the HIGH state for
        // an explicit length of time in milliseconds
        System.out.println("And now blink");
        while (true) {
            //myLed.pulse(1000);
            myLed.high();
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            myLed.low();
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }


        //System.out.println("Table created successfully");
        //System.exit(0);
    }
}
