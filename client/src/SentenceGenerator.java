import org.jetbrains.annotations.NotNull;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.*;
import java.util.concurrent.*;

/**
 * Created by tomas.bublik on 3.11.2015.
 */
public class SentenceGenerator {
    final static DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

    final int[] solutionArray = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    
    private static final String lineSeparator = System.getProperty("line.separator");

    final Random generator = new Random();
    final int MIN = 100;
    final int MAX = 200;

    public static void main(String[] args) {
        /**
         * -1 means forever
         */
        int numberOfSeconds = -1;
        if (args != null && args.length > 0) {
            try {
                numberOfSeconds = Integer.valueOf(args[0]);
            } catch (NumberFormatException e) {
                e.printStackTrace();
            }
        }

        SentenceGenerator sentenceGenerator = new SentenceGenerator();
        //sentenceGenerator.startGenerating(numberOfSeconds);
        sentenceGenerator.startGenerating(numberOfSeconds);
    }

    private void startGenerating(int numberOfSeconds) {
        //ExecutorService executor = Executors.newWorkStealingPool();
        ExecutorService executor = Executors.newFixedThreadPool(3);

        List<Callable<String>> callables = null;
        try {
            callables = Arrays.asList(
                    () -> {
                        innerOfGenerator(Thread.currentThread().getName() + ": First task", Action.HUMAN_IN);
                        return "";
                    },
                    () -> {
                        innerOfGenerator(Thread.currentThread().getName() + ": Second task", Action.HUMAN_OUT);
                        return "";
                    });
                    /*,
                    () -> {
                        innerOfGenerator(Thread.currentThread().getName() + ": Third task", Action.DOOR_ACTIVATION);
                        return "";
                    });*/
        } catch (Exception e) {
            e.printStackTrace();
        }

        try {
            if (numberOfSeconds != -1) {
                System.out.println("Running with timeout: " + numberOfSeconds);
                executor.invokeAll(callables, numberOfSeconds, TimeUnit.SECONDS);
            } else {
                System.out.println("Running in infinite loop");
                executor.invokeAll(callables);
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        System.out.println("Finished");
        System.exit(0);
    }

    private String innerOfGenerator(String x, Action action) {
        for (; ; ) {
            try {
                int randomNumber = generator.nextInt(MAX - MIN) + MIN;
                Thread.sleep(randomNumber);
            } catch (InterruptedException e) {
                throw new RuntimeException();
            }
            //System.out.println(x);
            String print = null;
            switch (action) {
                case HUMAN_IN:
                    print = generateEntranceIn();
                    break;
                case HUMAN_OUT:
                    print = generateEntranceOut();
                    break;
                default:
                    break;
            }
            if (print != null) {
                System.out.println(print);
                if (print.contains("VENKOVNI")) {
                    listPersonsIn();
                }
            }
        }
    }

    //AtomicInteger idOfPersonIn = new AtomicInteger(0);

    ConcurrentHashMap<Integer, Date> personsIn = new ConcurrentHashMap<>();

    enum Action {
        HUMAN_IN, HUMAN_OUT;
    }

    enum DoorAction {
        OPEN("OPEN"), CLOSE("CLOSE");

        private final String name;

        DoorAction(String name) {
            this.name = name;
        }

        public boolean equalsName(String otherName) {
            return (otherName == null) ? false : name.equals(otherName);
        }

        public String toString() {
            return this.name;
        }
    }           

    private String generateEntranceIn() {
        //ENTRANCE|VENKOVNI|2015-09-15T13:33:01|098765
        StringBuilder stringBuilder;
        String dateFormatted = getDateString();
        int id = getRandomNumberInRange(1, 10);
        if (!contained(id)) {
            stringBuilder = new StringBuilder();
            stringBuilder.append("ENTRANCE|VENKOVNI|" + dateFormatted + "|" + id);
            stringBuilder.append(lineSeparator);
            stringBuilder.append(generateDoorActivation(DoorAction.OPEN));
            stringBuilder.append(lineSeparator);
            stringBuilder.append(generateDoorActivation(DoorAction.CLOSE));
            personsIn.put(id, new Date());
            return stringBuilder.toString();
        }
        System.out.println("Toto již obsahuje: "+id);
        return null;
    }

    @NotNull
    private String getDateString() {
        LocalDateTime dateTime = LocalDateTime.now();
        return dateTime.format(formatter);
    }

    private String generateEntranceOut() {
        //ENTRANCE|VNITRNI|2015-09-15T13:33:01|098765
        StringBuilder stringBuilder;
        String dateFormatted = getDateString();
        int id = 0;
        if (!personsIn.isEmpty()) {
            id = getRandomIdFromPersonsIn(id);
            stringBuilder = new StringBuilder();
            stringBuilder.append("ENTRANCE|VNITRNI|" + dateFormatted + "|" + id);
            stringBuilder.append(lineSeparator);
            stringBuilder.append(generateDoorActivation(DoorAction.OPEN));
            stringBuilder.append(lineSeparator);
            stringBuilder.append(generateDoorActivation(DoorAction.CLOSE));
            personsIn.remove(id);
            return stringBuilder.toString();
        }
        return null;
    }

    private int getRandomIdFromPersonsIn(int id) {
        Set<Integer> enumInIndexes = personsIn.keySet();
        int size = enumInIndexes.size();
        int item = new Random().nextInt(size); // In real life, the Random object should be rather more shared than this
        int i = 0;
        for (Integer obj : enumInIndexes) {
            if (i == item) {
                id = obj;
            }
            i = i + 1;
        }
        return id;
    }

    @NotNull
    private String generateDoorActivation(DoorAction doorAction) {
        //DOOR_ACTIVATION|OPEN|2015-09-15T13:33:04|DOOR
        String dateFormatted = getDateString();
        return "DOOR_ACTIVATION|" + doorAction.toString() + "|" + dateFormatted + "|DOOR ";
    }

    private static int getRandomNumberInRange(int min, int max) {
        if (min >= max) {
            throw new IllegalArgumentException("max must be greater than min");
        }
        Random r = new Random();
        return r.nextInt((max - min) + 1) + min;
    }

    private void listPersonsIn() {
        if (!personsIn.isEmpty()) {
            StringBuilder stringBuilder = new StringBuilder();
            for (Integer integer : personsIn.keySet()) {
                stringBuilder.append(integer.intValue()).append(",");
            }

            System.out.println("Persons in:" +stringBuilder.substring(0, stringBuilder.length()-1));
        }
    }

    private boolean contained(int id) {
        if (!personsIn.isEmpty()) {
            for (Integer integer : personsIn.keySet()) {
                if (id == integer.intValue()) {
                    return true;
                }
            }
        }
        return false;
    }
}
