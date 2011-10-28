package main;

import entity.*;
import java.util.ArrayList;
import java.util.Collection;

public class Validator {

    private ArrayList<Course> courseList = new ArrayList<Course>();
    private ArrayList<Curriculum> curriculumList = new ArrayList<Curriculum>();
    private ArrayList<Room> roomList = new ArrayList<Room>();
    private ArrayList<Teacher> teacherList = new ArrayList<Teacher>();
    private int numDays = 0;
    private int numSlotsPerDay = 0;
    private int roomCapacityCost = 0;
    private int minWorkingDaysCost = 0;
    private int curriculumCompactnessCost = 0;
    private int roomStabilityCost = 0;
    private int totalCost = 0;

    public Validator(Collection<Course> courses,
            Collection<Curriculum> curricula,
            Collection<Room> rooms,
            Collection<Teacher> teachers,
            int numDays,
            int numSlotsPerDay) {

        this.courseList.addAll(courses);
        this.curriculumList.addAll(curricula);
        this.roomList.addAll(rooms);
        this.teacherList.addAll(teachers);

        this.numDays = numDays;
        this.numSlotsPerDay = numSlotsPerDay;

    }

    public int calcInitialCost() {
        calcInitialRoomCapacityCost();
        calcInitialMinWorkingDaysCost();
        calcInitialCurriculumCompactnessCost();
        calcInitialRoomStabilityCost();

        totalCost = getRoomCapacityCost() + getMinWorkingDaysCost() + getCurriculumCompactnessCost() + getRoomStabilityCost();
        return getTotalCost();
    }

    private int calcInitialRoomCapacityCost() {
        roomCapacityCost = 0;
        for (Course c : courseList) {
            for (Lecture l : c.getLectures()) {
                int diff = l.getCourse().getMinOfStudents() - l.getRoom().getCapacity();
                if (diff > 0) {
                    roomCapacityCost += diff;
                }
            }
        }
        return getRoomCapacityCost();
    }

    private int calcInitialMinWorkingDaysCost() {
        minWorkingDaysCost = 0;
        for (Course c : courseList) {
            minWorkingDaysCost += c.getMinWorkingDaysCount() * 5;
        }

        return getMinWorkingDaysCost();
    }

    private int calcInitialCurriculumCompactnessCost() {
        curriculumCompactnessCost = 0;
        for (Curriculum c : curriculumList) {
            curriculumCompactnessCost += c.getIsolatedLecturesCount() * 2;
        }
        return getCurriculumCompactnessCost();
    }

    private int calcInitialRoomStabilityCost() {
        roomStabilityCost = 0;
        for (Course c : courseList) {
            roomStabilityCost += c.getExcessRoomUsageCount();
        }
        return getRoomStabilityCost();
    }

    public int getRoomCapacityCost() {
        return roomCapacityCost;
    }

    public int getMinWorkingDaysCost() {
        return minWorkingDaysCost;
    }

    public int getCurriculumCompactnessCost() {
        return curriculumCompactnessCost;
    }

    public int getRoomStabilityCost() {
        return roomStabilityCost;
    }

    public int getTotalCost() {
        return totalCost;
    }
}
