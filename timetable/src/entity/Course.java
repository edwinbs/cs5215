package entity;

import java.util.HashMap;
import java.util.StringTokenizer;

public class Course {

    String name;
    Teacher teacher;
    int numOfLectures;
    int minWorkingDay;
    int minOfStudents;
    Curriculum curriculum;
    boolean[][] availability;

    public static Course create(String line, 
            HashMap<String, Teacher> teacherMap, int days, int slotsPerDay) {
        
        StringTokenizer strTok = new StringTokenizer(line);
        return new Course(strTok.nextToken(),
                strTok.nextToken(),
                Integer.parseInt(strTok.nextToken()),
                Integer.parseInt(strTok.nextToken()),
                Integer.parseInt(strTok.nextToken()),
                teacherMap,
                days, slotsPerDay);
    }

    public Course(String name, String teacherName,
            int numOfLectures, int minWorkingDay, int minOfStudents,
            HashMap<String, Teacher> teacherMap,
            int days, int slotsPerDay) {
        
        super();

        this.name = name;
        this.numOfLectures = numOfLectures;
        this.minWorkingDay = minWorkingDay;
        this.minOfStudents = minOfStudents;
        this.availability = new boolean[days][slotsPerDay];

        if (!teacherMap.containsKey(teacherName)) {
            teacherMap.put(teacherName, Teacher.create(days, slotsPerDay));
        }

        this.teacher = teacherMap.get(teacherName);
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

    public void setCurriculum(Curriculum curriculum) {
        this.curriculum = curriculum;
    }

    public void setAvailable(boolean bAvailable, int day, int slot) {
        this.availability[day][slot] = bAvailable;
    }
}
