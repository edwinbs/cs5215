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
    
    public void UndoAll() {
        for (IUndoableAction action : history) {
            action.rollBack();
        }
        ClearHistory();
    }

    public void ClearHistory() {
        history.clear();
    }
    
    public void Swap(Lecture lec1, Lecture lec2) {
        SwapAction action = SwapAction.create(lec1, lec2,SwapAction.SWAP);
        history.add(action);
        action.execute();
    }

    public void SwapRoom(Lecture lec1, Lecture lec2)
    {
    	SwapAction action = SwapAction.create(lec1, lec2,SwapAction.SWAP_ROOM);
        history.add(action);
        action.execute();
    }
    
    public void Move(Lecture lec, Room room, int day, int timeSlot) {
        AssignmentAction action = AssignmentAction.create(lec, room, day, timeSlot);
        history.add(action);
        action.execute();
    }
    
    public void putAdjacent(Lecture isolatedLecture,Lecture hasNeighborLecture,int slotPerDay){
    	CurriculumAction action = CurriculumAction.create(isolatedLecture,hasNeighborLecture,slotPerDay);
    	history.add(action);
    	action.execute();
    }
    
    public void putAdjacent(Lecture isolatedLecture,Lecture hasNeighborLecture,int slotPerDay,Room newRoom){
    	CurriculumAction action = CurriculumAction.create(isolatedLecture, hasNeighborLecture, slotPerDay,newRoom);
    	history.add(action);
    	action.execute();
    }
    
    public void MoveLectureToAnotherSlot(Lecture lecture,int newDay,int newSlot){
    	MoveLectureToAnotherSlotAction action = MoveLectureToAnotherSlotAction.create(lecture, newDay, newSlot);
    	history.add(action);
    	action.execute();
    }
    
    public void MoveLectureToNewRoom(Lecture lecture,Room room){
    	MoveAction action = MoveAction.create(lecture, room);
    	history.add(action);
    	action.execute();
    }
}
interface IUndoableAction {
    public void execute();
    public void rollBack();
}

class MoveAction implements IUndoableAction{
	
	Lecture l;
	Room newRoom;
	Room oldRoom;
	int day;
	int slot;
	
	public static MoveAction create(Lecture l,Room r){
		return new MoveAction(l,r);
	}
	
	private MoveAction(Lecture l,Room r){
		this.l = l;
		this.newRoom = r;
		this.oldRoom = l.getRoom();
		this.day = l.getDay();
		this.slot = l.getTimeSlot();
	}

	@Override
	public void execute() {
		l.clearAssignments();
		l.assignSlot(newRoom, day, slot);	
	}

	@Override
	public void rollBack() {
		l.clearAssignments();
		l.assignSlot(oldRoom, day, slot);	
	}
	
	
}
class MoveLectureToAnotherSlotAction implements IUndoableAction{

	int preDay,preSlot;
	Room preRoom;
	Lecture lecture;
	int newDay,newSlot;
	public static MoveLectureToAnotherSlotAction create(Lecture l,int day,int slot){
		return new MoveLectureToAnotherSlotAction(l, day, slot);
	}
	
	private MoveLectureToAnotherSlotAction(Lecture l,int day,int slot){
		newDay = day;
		newSlot = slot;
		preDay = l.getDay();
		preSlot = l.getTimeSlot();
		preRoom = l.getRoom();
		lecture = l;
	}

	@Override
	public void execute() {
		lecture.clearAssignments();
		lecture.assignSlot(preRoom, newDay, newSlot);
	}

	@Override
	public void rollBack() {
		lecture.clearAssignments();
		lecture.assignSlot(preRoom, preDay, preSlot);	
	}
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
    
    public static int SWAP = 1;
    public static int SWAP_ROOM = 2;
    
    private int swap_choice;
    
    public static SwapAction create(Lecture lec1, Lecture lec2,int choice) {
        return new SwapAction(lec1, lec2,choice);
    }
    
    public SwapAction(Lecture lec1, Lecture lec2,int choice) {
        this.lec1 = lec1;
        this.lec2 = lec2;
        this.swap_choice = choice;
    }
    
    @Override
    public void execute() {
    	if (this.swap_choice==SWAP)
    		swap();
    	else if (this.swap_choice==SWAP_ROOM)
    		swapRoom();
    }

    @Override
    public void rollBack() {
    	if (this.swap_choice==SWAP)
    		swap();
    	else if (this.swap_choice==SWAP_ROOM)
    		swapRoom();
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
    
    private void swapRoom(){
    	
        Room room1 = lec1.getRoom();
        Room room2 = lec2.getRoom();
        
        int day1 = lec1.getDay();
        int day2 = lec2.getDay();
        
        int timeSlot1 = lec1.getTimeSlot();
        int timeSlot2 = lec2.getTimeSlot();
        
        lec1.clearAssignments();
        lec2.clearAssignments();
        
        lec1.assignSlot(room2, day1, timeSlot1);
        lec2.assignSlot(room1, day2, timeSlot2);
        
    }
    
}

class CurriculumAction implements IUndoableAction{
	
	private Lecture isolatedLecture;
	private Lecture hasNeighborLecture;
	
	private int preDay;
	private int preSlot;
	private int slotPerday;
	private Room preRoom;
	private Room newRoom;
	private int hasNeighborDay;
	private int hasNeighborSlot;
	private boolean useNewRoom;
	
	public static CurriculumAction create(Lecture isolatedLecture,Lecture hasNeighborLecture,int slotPerday){
		return new CurriculumAction(isolatedLecture, hasNeighborLecture,slotPerday);
	}
	public static CurriculumAction create(Lecture isolatedLecture,Lecture hasNeighborLecture,int slotPerday,Room newRoom){
		return new CurriculumAction(isolatedLecture, hasNeighborLecture,slotPerday,newRoom);
	}
	
	private CurriculumAction(Lecture isolatedLecture,Lecture hasNeighborLecture,int slotPerday,Room newRoom){
		this.isolatedLecture = isolatedLecture;
		this.hasNeighborLecture = hasNeighborLecture;
		this.preDay = this.isolatedLecture.getDay();
		this.preSlot = this.isolatedLecture.getTimeSlot();
		this.slotPerday = slotPerday;
		this.preRoom = this.isolatedLecture.getRoom();
		this.newRoom = newRoom;
		this.hasNeighborDay  = hasNeighborLecture.getDay();
		this.hasNeighborSlot = hasNeighborLecture.getTimeSlot();
		this.useNewRoom = true;
	}
	private CurriculumAction(Lecture isolatedLecture,Lecture hasNeighborLecture,int slotPerday){
		this.isolatedLecture = isolatedLecture;
		this.hasNeighborLecture = hasNeighborLecture;
		this.preDay = this.isolatedLecture.getDay();
		this.preSlot = this.isolatedLecture.getTimeSlot();
		this.slotPerday = slotPerday;
		this.preRoom = this.isolatedLecture.getRoom();
		this.hasNeighborDay  = hasNeighborLecture.getDay();
		this.hasNeighborSlot = hasNeighborLecture.getTimeSlot();
		this.useNewRoom = false;
	}
	
	private void putAdjacent(){
		if (putBefore())
			return;
		
		putAfter();
	}
	
	private boolean putBefore(){
		if (useNewRoom){
			if (hasNeighborSlot-1>=0)
			{
				if (isolatedLecture.isCompatibleWith(newRoom, hasNeighborDay, hasNeighborSlot-1))
				{
					isolatedLecture.clearAssignments();
					isolatedLecture.assignSlot(newRoom, hasNeighborDay, hasNeighborSlot-1);
					return true;
				}
			}
			
			return false;
		}
		else{
			if (hasNeighborSlot-1>=0)
			{
				if (isolatedLecture.isCompatibleWith(preRoom, hasNeighborDay, hasNeighborSlot-1))
				{
					isolatedLecture.clearAssignments();
					isolatedLecture.assignSlot(preRoom, hasNeighborDay, hasNeighborSlot-1);
					return true;
				}
			}
			
			return false;
		}
	}
	
	private boolean putAfter(){
		if (useNewRoom){
			if ((hasNeighborSlot+1)<=(slotPerday-1))
			{
				if (isolatedLecture.isCompatibleWith(newRoom, hasNeighborDay, hasNeighborSlot+1))
				{
					isolatedLecture.clearAssignments();
					isolatedLecture.assignSlot(newRoom, hasNeighborDay, hasNeighborSlot+1);
					return true;
				}
			}
			
			return false;
		}
		else{	
			if ((hasNeighborSlot+1)<=(slotPerday-1))
			{
				if (isolatedLecture.isCompatibleWith(preRoom, hasNeighborDay, hasNeighborSlot+1))
				{
					isolatedLecture.clearAssignments();
					isolatedLecture.assignSlot(preRoom, hasNeighborDay, hasNeighborSlot+1);
					return true;
				}
			}
			
			return false;
		}
	}

	@Override
	public void execute() {
		putAdjacent();
		
	}

	@Override
	public void rollBack() {
		isolatedLecture.clearAssignments();
		isolatedLecture.assignSlot(preRoom, preDay, preSlot);
		
	}
}