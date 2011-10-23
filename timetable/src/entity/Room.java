package entity;

import java.util.StringTokenizer;

public class Room {
    
	String name;
	int capacity;
    
    public static Room create(String line)
    {
        StringTokenizer strTok = new StringTokenizer(line);
        return new Room(strTok.nextToken(), Integer.parseInt(strTok.nextToken()));
    }
	
	public Room(String name, int capacity) {
		super();
		this.name = name;
		this.capacity = capacity;
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
