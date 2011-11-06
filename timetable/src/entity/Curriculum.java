package entity;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.StringTokenizer;

class CurriculumSnapshot {
    boolean[][] used;
    boolean[][] hasNeighbour;
    
    public CurriculumSnapshot(boolean[][] used, boolean[][] hasNeighbour) {
        this.used = new boolean[used.length][used[0].length];
        for (int i=0; i<used.length; ++i) {
            System.arraycopy(used[i], 0, this.used[i], 0, used[i].length);
        }
        
        this.hasNeighbour = new boolean[hasNeighbour.length][hasNeighbour[0].length];
        for (int i=0; i<hasNeighbour.length; ++i) {
            System.arraycopy(hasNeighbour[i], 0, this.hasNeighbour[i], 0, hasNeighbour[i].length);
        }
    }
}

public class Curriculum implements Restorable {

    private String name;
    private int numOfCourses;
    private ArrayList<Course> member_list;
    private boolean[][] used; //TODO: Needs snapshot
    private boolean[][] hasNeighbour; //TODO: Needs snapshot
    private int days = 0;
    private int slotsPerDay = 0;
    private HashMap<Integer, CurriculumSnapshot> snapshots = new HashMap<Integer, CurriculumSnapshot>();

    public static Curriculum create(String line, HashMap<String, Course> courses, int days, int slotsPerDay) {
        StringTokenizer strTok = new StringTokenizer(line);

        String sName = strTok.nextToken();
        int tmpNumOfCourse = Integer.parseInt(strTok.nextToken());
        ArrayList<Course> tmpMemberList = new ArrayList<Course>();

        Curriculum inst = new Curriculum(sName, tmpNumOfCourse, tmpMemberList, days, slotsPerDay);

        for (int j = 0; j < tmpNumOfCourse; ++j) {
            Course course = courses.get(strTok.nextToken());
            course.addCurriculum(inst);
            tmpMemberList.add(course);
        }

        return inst;
    }

    public Curriculum(String name, int numOfCourses, ArrayList<Course> member_list, int days, int slotsPerDay) {
        super();
        this.name = name;
        this.numOfCourses = numOfCourses;
        this.member_list = member_list;
        this.used = new boolean[days][slotsPerDay];
        this.slotsPerDay = slotsPerDay;
        this.days = days;

        hasNeighbour = new boolean[days][slotsPerDay];
    }

    public void setUsed(boolean b, int d, int s) {
        hasNeighbour[d][s] = false;

        if (b && !used[d][s]) {
            if (s - 1 >= 0 && used[d][s - 1]) {
                hasNeighbour[d][s] = true;
                hasNeighbour[d][s - 1] = true;
            }
            if (s + 1 < slotsPerDay && used[d][s + 1]) {
                hasNeighbour[d][s] = true;
                hasNeighbour[d][s + 1] = true;
            }
        } else if (!b && used[d][s]) {
            if (s - 1 >= 0) {
                if (!(s - 2 >= 0) || !used[d][s - 2]) {
                    hasNeighbour[d][s - 1] = false;
                }
            }
            if (s + 1 < slotsPerDay) {
                if (!(s + 2 < slotsPerDay) || !used[d][s + 2]) {
                    hasNeighbour[d][s + 1] = false;
                }
            }
        }

        used[d][s] = b;
    }

    public boolean isUsed(int d, int s) {
        return this.used[d][s];
    }

    public int getIsolatedLecturesCost() {
        int isolatedLecturesCount = 0;
        for (int d = 0; d < days; ++d) {
            for (int s = 0; s < slotsPerDay; ++s) {
                if (used[d][s] && !hasNeighbour[d][s]) {
                    ++isolatedLecturesCount;
                }
            }
        }

        return isolatedLecturesCount * 2;
    }
    
    public ArrayList<Lecture> getIsolatedLectures(){
    	
    	ArrayList<Lecture> IsolatedLectures = new ArrayList<Lecture>();
    	
    	for (int d = 0; d < days; ++d) {
            for (int s = 0; s < slotsPerDay; ++s) {
                if (used[d][s] && !hasNeighbour[d][s])
                {
                	for (int j=0;j<member_list.size();++j)
                	{
                		Course c = member_list.get(j);
                		Lecture l = c.getLectureForSlot(d, s);
                		if (l != null)
                			IsolatedLectures.add(l);
                	}
                }
            }
    	}
    	
    	return IsolatedLectures;
    }
    
    public ArrayList<Lecture> getHasNeighborLectures(){
    	
    	ArrayList<Lecture> HasNeighborLectures = new ArrayList<Lecture>();
    	
    	for (int d = 0; d < days; ++d) {
            for (int s = 0; s < slotsPerDay; ++s) {
                if (used[d][s] && hasNeighbour[d][s])
                {
                	for (int j=0;j<member_list.size();++j)
                	{
                		Course c = member_list.get(j);
                		Lecture l = c.getLectureForSlot(d, s);
                		if (l != null)
                			HasNeighborLectures.add(l);
                	}
                }
            }
    	}
    	
    	return HasNeighborLectures;
    }

    @Override
    public void takeSnapshot(int snapshotType) {
        snapshots.put(snapshotType, new CurriculumSnapshot(this.used, this.hasNeighbour));
    }

    @Override
    public void restoreSnapshot(int snapshotType) {
        CurriculumSnapshot snapshot = snapshots.get(snapshotType);
        if (snapshot != null) {
            this.used = new boolean[snapshot.used.length][snapshot.used[0].length];
            for (int i=0; i<snapshot.used.length; ++i) {
                System.arraycopy(snapshot.used[i], 0, this.used[i], 0, snapshot.used[i].length);
            }

            this.hasNeighbour = new boolean[snapshot.hasNeighbour.length][snapshot.hasNeighbour[0].length];
            for (int i=0; i<snapshot.hasNeighbour.length; ++i) {
                System.arraycopy(snapshot.hasNeighbour[i], 0, this.hasNeighbour[i], 0, snapshot.hasNeighbour[i].length);
            }
        }
    }
}
