package main;

import entity.Unavailability;
import entity.Course;
import entity.Curriculum;
import entity.Room;

import entity.Teacher;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;

class SkippingBufferedReader extends BufferedReader
{
    public SkippingBufferedReader(Reader in) {
        super(in);
    }
    
    public void skipLine(int n) {
        try { while (n-- > 0) readLine(); } catch (IOException ex) {};
    }
    
    @Override
    public String readLine() throws IOException {
        String s = "";
        while ((s = super.readLine()).isEmpty());
        return s;
    }
}

public class TimetableSolver
{
    private String  problemName = "";
	private int     numOfCourses;
	private int     numOfRooms;
	private int     days;
	private int     slotsPerDay;
	private int     numOfCurricula;
	private int     numOfConstraints;
	
	private HashMap<String, Course> courseMap = new HashMap<String, Course>();
    private HashMap<String, Teacher> teacherMap = new HashMap<String, Teacher>();
    
	private ArrayList<Room> roomList = new ArrayList<Room>();
	private ArrayList<Curriculum> curriculumList = new ArrayList<Curriculum>();
    
    public TimetableSolver(String[] args) {        
		try {
			input(args[0]);
		} catch (IOException e) {
            System.out.printf("Failed to read file: %s\n", args[0]);
		}
    }
    
    public void Solve() {
        
    }
    
    private void input(String filename) throws IOException {
		SkippingBufferedReader in = new SkippingBufferedReader(new FileReader(filename));
        
        problemName = in.readLine().substring("Name:".length()).trim();
		numOfCourses = Integer.parseInt(in.readLine().substring("Courses:".length()).trim());
		numOfRooms = Integer.parseInt(in.readLine().substring("Rooms:".length()).trim());
		days = Integer.parseInt(in.readLine().substring("Days:".length()).trim());
		slotsPerDay = Integer.parseInt(in.readLine().substring("Periods_per_day:".length()).trim());
		numOfCurricula = Integer.parseInt(in.readLine().substring("Curricula:".length()).trim());
		numOfConstraints = Integer.parseInt(in.readLine().substring("Constraints:".length()).trim());
		
		//read COURSES information
		in.skipLine(1);
		for (int i=0; i<numOfCourses; ++i)
		{
            Course c = Course.create(in.readLine(), teacherMap, days, slotsPerDay);
            courseMap.put(c.getName(), c);
		}
		
		//read ROOMS information
		in.skipLine(1);
		for (int i=0; i<numOfRooms; ++i)
            roomList.add(Room.create(in.readLine(), days, slotsPerDay));
		
		//read CURRICULA information
		in.skipLine(1);
		for (int i=0; i<numOfCurricula; ++i)
            curriculumList.add(Curriculum.create(in.readLine(), courseMap, days, slotsPerDay));
		
		//read UNAVAILABILITY information
		in.skipLine(1);
		for (int i=0; i<numOfConstraints; ++i)
            Unavailability.updateCourse(in.readLine(), courseMap);
		
		in.close();
	}
}