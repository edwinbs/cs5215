package entity;

import java.util.HashMap;
import java.util.StringTokenizer;

class RoomSnapshot {
    Lecture[][] user;
    
    public RoomSnapshot(Lecture[][] user) {
        this.user = new Lecture[user.length][user[0].length];
        for (int i=0; i<user.length; ++i) {
            System.arraycopy(user[i], 0, this.user[i], 0, user[i].length);
        }
    }
}

public class Room implements Restorable {

    private String name;
    private int capacity;
    private Lecture[][] user; //TODO: Needs snapshot
    private HashMap<Integer, RoomSnapshot> snapshots = new HashMap<Integer, RoomSnapshot>();

    public static Room create(String line, int days, int slotsPerDay) {
        StringTokenizer strTok = new StringTokenizer(line);
        return new Room(strTok.nextToken(), Integer.parseInt(strTok.nextToken()), days, slotsPerDay);
    }

    public Room(String name, int capacity, int days, int slotsPerDay) {
        super();
        this.name = name;
        this.capacity = capacity;
        this.user = new Lecture[days][slotsPerDay];
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

    public void setUsed(Lecture lec, int d, int s) {
        this.user[d][s] = lec;
    }

    public boolean isUsed(Integer d, Integer s) {
        return (this.user[d][s] != null);
    }

    public Lecture getUser(int d, int s) {
        return this.user[d][s];
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

    @Override
    public void takeSnapshot(int snapshotType) {
        snapshots.put(snapshotType, new RoomSnapshot(this.user));
    }

    @Override
    public void restoreSnapshot(int snapshotType) {
        RoomSnapshot snapshot = snapshots.get(snapshotType);
        if (snapshot != null) {
            this.user = new Lecture[snapshot.user.length][snapshot.user[0].length];
            for (int i=0; i<snapshot.user.length; ++i) {
                System.arraycopy(snapshot.user[i], 0, this.user[i], 0, snapshot.user[i].length);
            }
        }
    }
}
