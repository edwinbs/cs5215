package entity;

import java.util.ArrayList;
import java.util.HashMap;

class LectureSnapshot {
    Room room;
    int day;
    int timeSlot;
    int failureCount;
    
    public LectureSnapshot(Room room, int day, int timeSlot, int failureCount) {
        this.room = room;
        this.day = day;
        this.timeSlot = timeSlot;
        this.failureCount = failureCount;
    }
}

public class Lecture implements Comparable, Restorable {

    private String courseName = "";
    private Course course;
    private Room room; //TODO: Needs snapshot
    private int day; //TODO: Needs snapshot
    private int timeSlot; //TODO: Needs snapshot
    private int failureCount; //TODO: Needs snapshot
    
    private HashMap<Integer, LectureSnapshot> snapshots = new HashMap<Integer, LectureSnapshot>();

    public static Lecture create(String courseName) {
        return new Lecture(courseName);
    }

    public Lecture(String courseName) {
        this.courseName = courseName;
    }

    public void assignSlot(Room r, int d, int s) {
        this.room = r;
        this.day = d;
        this.timeSlot = s;

        setUsed(true);
    }

    public void setCourse(Course course) {
        this.course = course;
    }

    public Course getCourse() {
        return this.course;
    }

    public void addFailureCount() {
        ++this.failureCount;
    }

    @Override
    public String toString() {
        String roomName = "";
        if (room != null) {
            roomName = room.getName();
        }

        return String.format("%s %s %d %d", courseName, roomName, day, timeSlot);
    }

    @Override
    public int compareTo(Object t) {
        if (!(t instanceof Lecture)) {
            throw new UnsupportedOperationException("Incompatible types");
        }

        return (this.getWeight() < ((Lecture) t).getWeight()) ? 1 : -1;
    }

    private int getWeight() {
        return this.getCourse().getConstraintLevel()
                + 10 * this.failureCount
                + (this.getCourse().getMinOfStudents() / 10);
    }

    public boolean isCompatibleWith(Room r, Integer d, Integer s) {
        if (r.isUsed(d, s)) {
            return false;
        }

        if (getCourse().getTeacher().IsUsed(d, s)) {
            return false;
        }

        if (getCourse().isUnavailable(d, s)) {
            return false;
        }

        ArrayList<Curriculum> curricula = getCourse().getCurricula();
        for (int i = 0; i < curricula.size(); ++i) {
            if (curricula.get(i).isUsed(d, s)) {
                return false;
            }
        }

        return true;
    }
    
    public boolean canSwap(Lecture lec2) {
        if (getCourse().getTeacher().IsUsed(lec2.getDay(), lec2.getTimeSlot())) {
            return false;
        }
        
        if (getCourse().isUnavailable(lec2.getDay(), lec2.getTimeSlot())) {
            return false;
        }
        
        ArrayList<Curriculum> curricula = getCourse().getCurricula();
        for (int i = 0; i < curricula.size(); ++i) {
            if (curricula.get(i).isUsed(lec2.getDay(), lec2.getTimeSlot())) {
                return false;
            }
        }

        return true;
    }

    public void clearAssignments() {
        setUsed(false);
        room = null;
        day = 0;
        timeSlot = 0;
    }

    private void setUsed(boolean b) {
        for (Curriculum c : course.getCurricula()) {
            c.setUsed(b, day, timeSlot);
        }

        if (room != null) {
            room.setUsed(b ? this : null, day, timeSlot);
        }

        this.getCourse().getTeacher().setUsed(b, day, timeSlot);

        //Dynamic programming
        this.getCourse().setWorkingDay(b, day);
        this.getCourse().setRoomUsed(b, room);
    }

    public Room getRoom() {
        return this.room;
    }

    public int getDay() {
        return this.day;
    }

    public int getTimeSlot() {
        return this.timeSlot;
    }

    public boolean isRoomLargeEnough() {
        return this.getRoom().getCapacity() >= this.getCourse().getMinOfStudents();
    }

    public boolean isRoomNotWorse(Room room) {
        if (room.getCapacity() >= this.getCourse().getMinOfStudents())
            return true;
        
        return room.getCapacity() >= this.getRoom().getCapacity();
    }
    
    public int getRoomCapacityCost() {
        int diff = this.getCourse().getMinOfStudents() - this.getRoom().getCapacity();
        return (diff > 0) ? diff : 0;
    }

    public int getSoftConstraintCost() {
        int cost = 0;
        cost += this.getRoomCapacityCost();
        cost += this.getCourse().getMinWorkingDaysCost();
        cost += this.getCourse().getRoomStabilityPenalty();
        for (Curriculum cur : this.getCourse().getCurricula()) {
            cost += cur.getIsolatedLecturesCost();
        }
        return cost;
    }
    
    public boolean canbePutAdjacentWithNewRoom(Lecture oneWithNeighbor,Room newRoom,int slotPerday)
    {
    	if (oneWithNeighbor.getTimeSlot()-1>=0 )
    	{
    		//System.out.println("CHECK: room, new day and new slot = "+newRoom.getName()+" & "+oneWithNeighbor.getDay()+" & "+(oneWithNeighbor.getTimeSlot()-1));
    		if (isCompatibleWith(newRoom, oneWithNeighbor.getDay(), oneWithNeighbor.getTimeSlot()-1))
    			return true;
    	}
    	
    	if ((oneWithNeighbor.getTimeSlot()+1)<=(slotPerday-1) )
    	{
    		//System.out.println("CHECK: room, new day and new slot = "+newRoom.getName()+" & "+oneWithNeighbor.getDay()+" & "+(oneWithNeighbor.getTimeSlot()+1));
    		if (isCompatibleWith(newRoom, oneWithNeighbor.getDay(), oneWithNeighbor.getTimeSlot()+1))
    			return true;
    	}
    	
    	return false;
    }
    public boolean canbePutAdjacentWithSameRoom(Lecture oneWithNeighbor,int slotPerday){
    	
    	if (oneWithNeighbor.getTimeSlot()-1>=0 && getRoom() != oneWithNeighbor.getRoom())
    	{
    		if (isCompatibleWith(getRoom(), oneWithNeighbor.getDay(), oneWithNeighbor.getTimeSlot()-1))
    			return true;
    	}
    	
    	if ((oneWithNeighbor.getTimeSlot()+1)<=(slotPerday-1) && getRoom() != oneWithNeighbor.getRoom())
    	{
    		if (isCompatibleWith(getRoom(), oneWithNeighbor.getDay(), oneWithNeighbor.getTimeSlot()+1))
    			return true;
    	}
    	
    	return false;
    }

    @Override
    public void takeSnapshot(int snapshotType) {
        snapshots.put(snapshotType, new LectureSnapshot(this.room, this.day, this.timeSlot, this.failureCount));
    }

    @Override
    public void restoreSnapshot(int snapshotType) {
        LectureSnapshot snapshot = snapshots.get(snapshotType);
        if (snapshot != null) {
            this.room = snapshot.room;
            this.day = snapshot.day;
            this.timeSlot = snapshot.timeSlot;
            this.failureCount = snapshot.failureCount;
        }
    }
}
