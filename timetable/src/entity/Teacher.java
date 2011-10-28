package entity;

public class Teacher {

    private String name = "";
    private boolean used[][];

    public static Teacher create(String name, int days, int slotsPerDay) {
        return new Teacher(name, days, slotsPerDay);
    }

    public Teacher(String name, int days, int slotsPerDay) {
        this.name = name;
        this.used = new boolean[days][slotsPerDay];
    }

    public String getName() {
        return this.name;
    }

    public boolean IsUsed(int d, int s) {
        return used[d][s];
    }

    public void setUsed(boolean b, Integer d, Integer s) {
        this.used[d][s] = b;
    }
}
