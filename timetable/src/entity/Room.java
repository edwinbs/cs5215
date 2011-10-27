package entity;

import java.util.StringTokenizer;

public class Room {
    
	String name;
	int capacity;
    boolean[][] availability;
    
    public static Room create(String line, int days, int slotsPerDay)
    {
        StringTokenizer strTok = new StringTokenizer(line);
        return new Room(strTok.nextToken(), Integer.parseInt(strTok.nextToken()), days, slotsPerDay);
    }
	
	public Room(String name, int capacity, int days, int slotsPerDay) {
		super();
		this.name = name;
		this.capacity = capacity;
        this.availability = new boolean[days][slotsPerDay];
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

}
