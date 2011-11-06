package entity;

import java.util.HashMap;

class TeacherSnapshot {
    boolean used[][];
    
    public TeacherSnapshot(boolean used[][]) {
        this.used = new boolean[used.length][used[0].length];
        for (int i=0; i<used.length; ++i) {
            System.arraycopy(used[i], 0, this.used[i], 0, used[i].length);
        }
    }
}

public class Teacher implements Restorable {

    private String name = "";
    private boolean used[][]; //TODO: Needs snapshot
    private HashMap<Integer, TeacherSnapshot> snapshots = new HashMap<Integer, TeacherSnapshot>();

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

    @Override
    public void takeSnapshot(int snapshotType) {
        snapshots.put(snapshotType, new TeacherSnapshot(this.used));
    }

    @Override
    public void restoreSnapshot(int snapshotType) {
        TeacherSnapshot snapshot = snapshots.get(snapshotType);
        if (snapshot != null) {
            this.used = new boolean[snapshot.used.length][snapshot.used[0].length];
            for (int i=0; i<snapshot.used.length; ++i) {
                System.arraycopy(snapshot.used[i], 0, this.used[i], 0, snapshot.used[i].length);
            }
        }
    }
}
