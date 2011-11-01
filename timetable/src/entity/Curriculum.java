package entity;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.StringTokenizer;

public class Curriculum {

    private String name;
    private int numOfCourses;
    private ArrayList<Course> member_list;
    private boolean[][] used;
    private boolean[][] hasNeighbour;
    private int days = 0;
    private int slotsPerDay = 0;

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
}
