package main;

import entity.Unavailability;
import entity.Course;
import entity.Curriculum;
import entity.Room;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.util.ArrayList;
import java.util.HashMap;

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
	private int     numOfDays;
	private int     numOfPeriodPerday;
	private int     numOfCurricula;
	private int     numOfConstraints;
	
	private HashMap<String, Course> courses = new HashMap<String, Course>();
	private ArrayList<Room> rooms = new ArrayList<Room>();
	private ArrayList<Curriculum> curricula = new ArrayList<Curriculum>();
	private ArrayList<Unavailability> unavailabilities = new ArrayList<Unavailability>();
    
    public TimetableSolver(String[] args) {        
		try {
			input(args[0]);
		} catch (IOException e) {
            System.out.printf("Failed to read file: %s\n", args[0]);
		}
    }
    
    private void input(String filename) throws IOException {
		SkippingBufferedReader in = new SkippingBufferedReader(new FileReader(filename));
        
        problemName = in.readLine().substring("Name:".length()).trim();
		numOfCourses = Integer.parseInt(in.readLine().substring("Courses:".length()).trim());
		numOfRooms = Integer.parseInt(in.readLine().substring("Rooms:".length()).trim());
		numOfDays = Integer.parseInt(in.readLine().substring("Days:".length()).trim());
		numOfPeriodPerday = Integer.parseInt(in.readLine().substring("Periods_per_day:".length()).trim());
		numOfCurricula = Integer.parseInt(in.readLine().substring("Curricula:".length()).trim());
		numOfConstraints = Integer.parseInt(in.readLine().substring("Constraints:".length()).trim());
		
		//read Courses information
		in.readLine();//skip
		for (int i=0; i<numOfCourses; ++i)
		{
            Course c = Course.create(in.readLine());
            courses.put(c.getName(), c);
		}
		
		//read Rooms information
		in.skipLine(1);
		for (int i=0; i<numOfRooms; ++i)
            rooms.add(Room.create(in.readLine()));
		
		//read CURRICULA info
		in.skipLine(1);
		for (int i=0; i<numOfCurricula; ++i)
            curricula.add(Curriculum.create(in.readLine(), courses));
		
		//read constraints info
		in.skipLine(1);
		for (int i=0; i<numOfConstraints; ++i)
            unavailabilities.add(Unavailability.create(in.readLine(), courses));
		
		in.close();
	}
}