package main;

public class main {

    public static void main(String[] args) {
        if (args.length < 1) {
            System.out.printf("Usage: java -jar Timetable.jar <input-file>\n");
            System.exit(1);
        }

        TimetableSolver instance = new TimetableSolver(args);
        instance.Solve();
    }
}
