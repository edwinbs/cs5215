package entity;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.StringTokenizer;

public class Curriculum {

    private String name;
    private int numOfCourses;
    private ArrayList<Course> member_list;
    private boolean[][] used;

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
    }

    public void setUsed(boolean b, int d, int s) {
        this.used[d][s] = b;
    }

    public boolean isUsed(int d, int s) {
        return this.used[d][s];
    }
}
