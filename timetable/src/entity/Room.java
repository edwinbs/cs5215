package entity;

import java.util.StringTokenizer;

public class Room {

    private String name;
    private int capacity;
    private boolean[][] used;

    public static Room create(String line, int days, int slotsPerDay) {
        StringTokenizer strTok = new StringTokenizer(line);
        return new Room(strTok.nextToken(), Integer.parseInt(strTok.nextToken()), days, slotsPerDay);
    }

    public Room(String name, int capacity, int days, int slotsPerDay) {
        super();
        this.name = name;
        this.capacity = capacity;
        this.used = new boolean[days][slotsPerDay];
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public int getCapacity() {
        return capacity;
    }

    public void setCapacity(int capacity) {
        this.capacity = capacity;
    }

    public void setUsed(boolean b, int d, int s) {
        this.used[d][s] = b;
    }

    public boolean isUsed(Integer d, Integer s) {
        return this.used[d][s];
    }

    @Override
    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }
        if (getClass() != obj.getClass()) {
            return false;
        }
        final Room other = (Room) obj;
        if ((this.name == null) ? (other.name != null) : !this.name.equals(other.name)) {
            return false;
        }
        return true;
    }

    @Override
    public int hashCode() {
        return this.name.hashCode();
    }
}
