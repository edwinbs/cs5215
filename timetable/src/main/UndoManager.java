package main;

import entity.Lecture;
import entity.Room;
import java.util.ArrayList;


public class UndoManager {
    private ArrayList<IUndoableAction> history = new ArrayList<IUndoableAction>();
    
    public void MakeAssignment(Lecture l, Room r, int day, int timeSlot) {
        AssignmentAction action = AssignmentAction.create(l, r, day, timeSlot);
        history.add(action);
        action.execute();
    }
    
    public void Swap(Lecture lec1, Lecture lec2) {
        SwapAction action = SwapAction.create(lec1, lec2);
        history.add(action);
        action.execute();
    }
    
    public void UndoAll() {
        for (IUndoableAction action : history) {
            action.rollBack();
        }
        ClearHistory();
    }

    public void ClearHistory() {
        history.clear();
    }
}

interface IUndoableAction {
    public void execute();
    public void rollBack();
}

class AssignmentAction implements IUndoableAction {

    private Lecture lecture;
    private Room prevRoom;
    private Room newRoom;
    private int prevDay;
    private int newDay;
    private int prevTimeSlot;
    private int newTimeSlot;

    public static AssignmentAction create(Lecture l, Room newRoom, int newDay,
            int newTimeSlot) {

        return new AssignmentAction(l, l.getRoom(), newRoom, l.getDay(), newDay,
                l.getTimeSlot(), newTimeSlot);
    }

    private AssignmentAction(Lecture l, Room prevRoom, Room newRoom, int prevDay,
            int newDay, int prevTimeSlot, int newTimeSlot) {

        this.lecture = l;
        this.prevRoom = prevRoom;
        this.newRoom = newRoom;
        this.prevDay = prevDay;
        this.newDay = newDay;
        this.prevTimeSlot = prevTimeSlot;
        this.newTimeSlot = newTimeSlot;
    }
    
    @Override
    public void execute() {
        changeAssignment(newRoom, newDay, newTimeSlot);
    }
    
    @Override
    public void rollBack() {
        changeAssignment(prevRoom, prevDay, prevTimeSlot);
    }
    
    private void changeAssignment(Room r, int day, int timeSlot) {
        lecture.clearAssignments();
        lecture.assignSlot(r, day, timeSlot);
    }
}

class SwapAction implements IUndoableAction {

    private Lecture lec1;
    private Lecture lec2;
    
    public static SwapAction create(Lecture lec1, Lecture lec2) {
        return new SwapAction(lec1, lec2);
    }
    
    public SwapAction(Lecture lec1, Lecture lec2) {
        this.lec1 = lec1;
        this.lec2 = lec2;
    }
    
    @Override
    public void execute() {
        swap();
    }

    @Override
    public void rollBack() {
        swap();
    }
    
    private void swap() {
        Room room1 = lec1.getRoom();
        Room room2 = lec2.getRoom();
        
        int day1 = lec1.getDay();
        int day2 = lec2.getDay();
        
        int timeSlot1 = lec1.getTimeSlot();
        int timeSlot2 = lec2.getTimeSlot();
        
        lec1.clearAssignments();
        lec2.clearAssignments();
        
        lec1.assignSlot(room2, day2, timeSlot2);
        lec2.assignSlot(room1, day1, timeSlot1);
    }
    
}