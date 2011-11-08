package main;

public class main {

    public static void main(String[] args) {
        if (args.length < 1) {
            System.out.printf("Usage: java main.main <input-file> [options]\n");
            System.out.printf("Options:\n");
            System.out.printf(" -s --seed <seed-value>\n");
            System.out.printf("     Use <seed-value> as the random number generator seed.\n");
            System.out.printf(" -t --timeout <timeout-secs>\n");
            System.out.printf("     Terminate the solver after <timepit-secs> seconds. Default=375s.\n");
            System.exit(1);
        }

        TimetableSolver instance = new TimetableSolver();
        if (instance.Initialize(args)) {
            instance.Solve();
        }
    }
}
