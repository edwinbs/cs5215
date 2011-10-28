package entity;

import java.util.ArrayList;

public class Lecture implements Comparable {

    private String courseName = "";
    private Course course;
    private Room room;
    private int day;
    private int timeSlot;
    private int failureCount;

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

        int mine = this.getCourse().getConstraintLevel() + 10 * this.failureCount;
        int yours = ((Lecture) t).getCourse().getConstraintLevel() + 10 * ((Lecture) t).failureCount;

        if (mine < yours) {
            return 1;
        } else {
            return -1;
        }
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
            room.setUsed(b, day, timeSlot);
        }

        this.getCourse().getTeacher().setUsed(b, day, timeSlot);
    }
}
