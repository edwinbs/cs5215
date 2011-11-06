package entity;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Random;
import java.util.StringTokenizer;

class CourseSnapshot {

    int constraintLevel;
    int[] workingDays;
    int numWorkingDays;
    HashMap<Room, Integer> roomUsageCountMap;
    int roomUsageCount;

    public CourseSnapshot(int constraintLevel, int[] workingDays,
            int numWorkingDays, HashMap<Room, Integer> roomUsageCountMap,
            int roomUsageCount) {

        this.constraintLevel = constraintLevel;
        this.numWorkingDays = numWorkingDays;
        this.roomUsageCount = roomUsageCount;

        this.workingDays = new int[workingDays.length];
        System.arraycopy(workingDays, 0, this.workingDays, 0, workingDays.length);

        this.roomUsageCountMap = new HashMap<Room, Integer>();
        this.roomUsageCountMap.putAll(roomUsageCountMap);
    }
}

public class Course implements Restorable {

    private String name;
    private Teacher teacher;
    private int numOfLectures;
    private int minWorkingDay;
    private int minOfStudents;
    private int constraintLevel; //TODO: Needs snapshot
    private ArrayList<Curriculum> curricula = new ArrayList<Curriculum>();
    private ArrayList<Lecture> lectures = new ArrayList<Lecture>();
    private ArrayList<Room> shuffledRooms = new ArrayList<Room>();
    private boolean[][] unavailable;
    private int[] workingDays; //TODO: Needs snapshot
    private int numWorkingDays = 0; //TODO: Needs snapshot
    private HashMap<Room, Integer> roomUsageCountMap = new HashMap<Room, Integer>(); //TODO: Needs snapshot
    private int roomUsageCount = 0; //TODO: Needs snapshot
    
    private HashMap<Integer, CourseSnapshot> snapshots = new HashMap<Integer, CourseSnapshot>();

    public static Course create(String line,
            ArrayList<Teacher> teacherList, int days, int slotsPerDay) {

        StringTokenizer strTok = new StringTokenizer(line);
        Course c = new Course(strTok.nextToken(),
                strTok.nextToken(),
                Integer.parseInt(strTok.nextToken()),
                Integer.parseInt(strTok.nextToken()),
                Integer.parseInt(strTok.nextToken()),
                teacherList,
                days, slotsPerDay);

        for (Lecture l : c.getLectures()) {
            l.setCourse(c);
        }

        return c;
    }

    public Course(String name, String teacherName,
            int numOfLectures, int minWorkingDay, int minOfStudents,
            ArrayList<Teacher> teacherList,
            int days, int slotsPerDay) {

        super();

        this.name = name;

        this.numOfLectures = numOfLectures;
        for (int i = 0; i < numOfLectures; ++i) {
            this.lectures.add(Lecture.create(name));
        }

        this.minWorkingDay = minWorkingDay;
        this.minOfStudents = minOfStudents;
        this.unavailable = new boolean[days][slotsPerDay];
        this.workingDays = new int[days];

        //For some reason using HashMap is more expensive than linear search!
        boolean bFound = false;
        for (int i = 0; i < teacherList.size(); ++i) {
            if (teacherList.get(i).getName().equals(teacherName)) {
                this.teacher = teacherList.get(i);
                bFound = true;
                break;
            }
        }

        if (!bFound) {
            Teacher t = Teacher.create(teacherName, days, slotsPerDay);
            this.teacher = t;
            teacherList.add(t);
        }
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public int getNumOfLectures() {
        return numOfLectures;
    }

    public void setNumOfLectures(int numOfLectures) {
        this.numOfLectures = numOfLectures;
    }

    public int getMinWorkingDay() {
        return minWorkingDay;
    }

    public void setMinWorkingDay(int minWorkingDay) {
        this.minWorkingDay = minWorkingDay;
    }

    public int getMinOfStudents() {
        return minOfStudents;
    }

    public void setMinOfStudents(int minOfStudents) {
        this.minOfStudents = minOfStudents;
    }

    public void addCurriculum(Curriculum curriculum) {
        this.curricula.add(curriculum);
    }

    public ArrayList<Curriculum> getCurricula() {
        return this.curricula;
    }

    public void setUnavailable(int day, int slot) {
        if (this.unavailable[day][slot] == false) {
            this.unavailable[day][slot] = true;
            this.setConstraintLevel(this.getConstraintLevel() + 1);
        }
    }

    public Teacher getTeacher() {
        return this.teacher;
    }

    public boolean isUnavailable(Integer d, Integer s) {
        return this.unavailable[d][s];
    }

    public ArrayList<Room> getShuffledRooms() {
        return this.shuffledRooms;
    }

    public ArrayList<Lecture> getLectures() {
        return this.lectures;
    }

    public int getConstraintLevel() {
        return constraintLevel;
    }

    public void setConstraintLevel(int constraintLevel) {
        this.constraintLevel = constraintLevel;
    }

    public void preprocessRooms(ArrayList<Room> roomList, Random rand) {
        ArrayList<Room> bestRooms = new ArrayList<Room>();
        ArrayList<Room> goodRooms = new ArrayList<Room>();
        ArrayList<Room> badRooms = new ArrayList<Room>();

        int bestDiff = Integer.MAX_VALUE;

        for (Room r : roomList) {
            int diff = r.getCapacity() - minOfStudents;

            if (diff >= 0) {
                if (diff < bestDiff) {
                    goodRooms.addAll(bestRooms);
                    bestRooms.clear();
                    bestRooms.add(r);
                } else if (diff == bestDiff) {
                    bestRooms.add(r);
                } else {
                    goodRooms.add(r);
                }
            } else {
                badRooms.add(r);
            }
        }

        Collections.shuffle(bestRooms, rand);
        shuffledRooms.addAll(bestRooms);

        Collections.shuffle(goodRooms, rand);
        shuffledRooms.addAll(goodRooms);

        Collections.shuffle(badRooms, rand);
        shuffledRooms.addAll(badRooms);
    }

    public void clearAssignments() {
        for (Lecture l : lectures) {
            l.clearAssignments();
        }
    }

    void setWorkingDay(boolean b, int day) {
        if (b) {
            if (workingDays[day] == 0) {
                ++numWorkingDays;
            }
            ++workingDays[day];
        } else if (workingDays[day] > 0) {
            --workingDays[day];
            if (workingDays[day] == 0) {
                --numWorkingDays;
            }
        }
    }

    public int getMinWorkingDaysCost() {
        int diff = minWorkingDay - numWorkingDays;
        return (diff > 0) ? diff * 5 : 0;
    }

    public void setRoomUsed(boolean b, Room r) {
        Integer n = roomUsageCountMap.get(r);
        if (b) {
            if (n == null || n == 0) {
                roomUsageCountMap.put(r, 1);
                ++roomUsageCount;
            } else {
                roomUsageCountMap.put(r, n + 1);
            }
        } else if (n != null && n > 0) {
            roomUsageCountMap.put(r, --n);
            if (n == 0) {
                --roomUsageCount;
            }
        }
    }

    public int getRoomStabilityPenalty() {
        return (roomUsageCount > 1) ? roomUsageCount - 1 : 0;
    }

    public Room getMajorityRoom() {
        int max = 0;
        Room majorityRoom = null;
        for (Room r : roomUsageCountMap.keySet()) {
            int count = roomUsageCountMap.get(r);
            if (count > max) {
                max = count;
                majorityRoom = r;
            }
        }
        return majorityRoom;
    }

    public int getNumOfLecturesForDay(int day) {
        return workingDays[day];
    }

    
    public Lecture getLectureForSlot(int day,int slot){
    	
    	for (int j=0;j<lectures.size();++j)
    	{
    		Lecture l = lectures.get(j);
    		
    		if (l.getDay()==day && l.getTimeSlot()==slot)
    			return l;
    	}
    	
    	return null;
    }
    
    public ArrayList<Integer> getWorkingDays(){
    	ArrayList<Integer> workingDaysList = new ArrayList<Integer>();   	
    	for (int j=0;j<workingDays.length;++j)
    		if (workingDays[j]>0)
    			workingDaysList.add(j);
    	Collections.shuffle(workingDaysList);
    	return workingDaysList;
    }
    
    public ArrayList<Integer> getNonWorkingDays(){
    	ArrayList<Integer> NonWorkingDaysList = new ArrayList<Integer>();  	    	
    	for (int j=0;j<workingDays.length;++j)
    		if (workingDays[j]==0)
    			NonWorkingDaysList.add(j);
    	Collections.shuffle(NonWorkingDaysList);
    	return NonWorkingDaysList;
    }
    
    public Lecture getRandomLecture(){
    	Random rand = new Random();
    	int rn = rand.nextInt(lectures.size());
    	
    	return lectures.get(rn);	
    }    
    
    @Override
    public void takeSnapshot(int snapshotType) {
        snapshots.put(snapshotType, new CourseSnapshot(this.constraintLevel,
                this.workingDays, this.numWorkingDays, this.roomUsageCountMap,
                this.roomUsageCount));
    }

    @Override
    public void restoreSnapshot(int snapshotType) {
        CourseSnapshot snapshot = snapshots.get(snapshotType);
        if (snapshot != null) {
            this.constraintLevel = snapshot.constraintLevel;
            this.numWorkingDays = snapshot.numWorkingDays;
            this.roomUsageCount = snapshot.roomUsageCount;

            this.workingDays = new int[snapshot.workingDays.length];
            System.arraycopy(snapshot.workingDays, 0, this.workingDays, 0, snapshot.workingDays.length);

            this.roomUsageCountMap = new HashMap<Room, Integer>();
            this.roomUsageCountMap.putAll(snapshot.roomUsageCountMap);
        }
    }
}
