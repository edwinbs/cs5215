package entity;

public class Teacher {
    private boolean availability[][];
    
    public static Teacher create(int days, int slotsPerDay) {
        return new Teacher(days, slotsPerDay);
    }
    
    public Teacher(int days, int slotsPerDay) {
        this.availability = new boolean[days][slotsPerDay];
    }
}
