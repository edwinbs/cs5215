package main;

import entity.Unavailability;
import entity.Course;
import entity.Curriculum;
import entity.Lecture;
import entity.Room;

import entity.Teacher;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;

class SkippingBufferedReader extends BufferedReader {

    public SkippingBufferedReader(Reader in) {
        super(in);
    }

    public void skipLine(int n) {
        try {
            while (n-- > 0) {
                readLine();
            }
        } catch (IOException ex) {
        };
    }

    @Override
    public String readLine() throws IOException {
        String s = "";
        while ((s = super.readLine()).isEmpty());
        return s;
    }
}

public class TimetableSolver {

    private Random rand = new Random();
    private String problemName = "";
    private int numOfCourses;
    private int numOfRooms;
    private int days;
    private int slotsPerDay;
    private int numOfCurricula;
    private int numOfConstraints;
    private HashMap<String, Course> courseMap = new HashMap<String, Course>();
    private ArrayList<Course> courseList = new ArrayList<Course>();
    private ArrayList<Teacher> teacherList = new ArrayList<Teacher>();
    private ArrayList<Room> roomList = new ArrayList<Room>();
    private ArrayList<Curriculum> curriculumList = new ArrayList<Curriculum>();
    private ArrayList<Lecture> lectureList = new ArrayList<Lecture>();
    private ArrayList<Integer> daysChoice = new ArrayList<Integer>();
    private ArrayList<Integer> slotsChoice = new ArrayList<Integer>();
    private Validator validator;
    private UndoManager undoManager = new UndoManager();
    private final int MAX_CONSTRUCTION_ITERS = 1000;
    private final int MAX_HILL_CLIMBING_ITERS = 1000000;
    private final int DEFAULT_TIMEOUT = 375;
    private int timeoutSecs = 0;
    private volatile boolean stopFlag = false;

    public boolean Initialize(String[] args) {
        try {
            input(args[0]);

            timeoutSecs = DEFAULT_TIMEOUT;

            for (int i = 1; i < args.length; ++i) {
                if (args[i].equals("-s") || args[i].equals("--seed")) {
                    rand.setSeed(Long.parseLong(args[i + 1]));
                    ++i;
                } else if (args[i].equals("-t") || args[i].equals("--timeout")) {
                    timeoutSecs = Integer.parseInt(args[i+1]);
                    ++i;
                }
            }
        } catch (IOException e) {
            System.out.printf("Failed to read file: %s\n", args[0]);
            return false;
        } catch (Exception e) {
            System.out.printf("Usage: TimetableSolver <input-file> [options]\n");
            return false;
        }
        return true;
    }

    public void Solve() {
        Timer t = new Timer();
        t.schedule(new TimeLimitter(), timeoutSecs * 1000);

        preprocessRooms();
        initialConstruction();
        hillClimbing();
        printSolution();

        t.cancel();
    }

    private void preprocessRooms() {
        for (Course c : courseMap.values()) {
            c.preprocessRooms(roomList, rand);
        }
    }

    private void clearAssignments() {
        for (Course c : courseMap.values()) {
            c.clearAssignments();
        }
    }

    private void initialConstruction() {
        for (int i = 0; (i < MAX_CONSTRUCTION_ITERS) && !stopFlag; ++i) {
            boolean isFailed = false;
            clearAssignments();
            Collections.sort(lectureList);
            lecture_loop:
            for (int l = 0; l < lectureList.size(); ++l) {
                Lecture lec = lectureList.get(l);
                ArrayList<Room> shuffledRooms = lec.getCourse().getShuffledRooms();

                for (int r = 0; r < shuffledRooms.size(); ++r) {
                    Collections.shuffle(daysChoice, rand);
                    Collections.shuffle(slotsChoice, rand);

                    for (int d = 0; d < daysChoice.size(); ++d) {
                        for (int s = 0; s < slotsChoice.size(); ++s) {
                            if (lec.isCompatibleWith(shuffledRooms.get(r), daysChoice.get(d), slotsChoice.get(s))) {
                                lec.assignSlot(shuffledRooms.get(r), daysChoice.get(d), slotsChoice.get(s));
                                continue lecture_loop;
                            }
                        }
                    }
                }
                isFailed = true;
                lec.addFailureCount();
            }

            if (!isFailed) {
                break;
            }
        }
    }
    
    private void hillClimbing() {
        int cost = validator.calcInitialCost();
        int waterLevel = (int) (cost * 1.02f);
        int lowestCost = cost;
        while (!stopFlag) {
            int rn = rand.nextInt(4);
            switch (rn) {
                case 0:
                    randomSwap();
                    break;
                case 1:
                    randomTimeMove();
                    break;
                case 2:
                    randomRoomMove();
                    break;
                case 3:
                    randomRoomStabilityMove();
                    break;
            }
            
            int newCost = validator.calcInitialCost();
            if (newCost > waterLevel) {
                undoManager.UndoAll();
            } else {
                if (newCost < lowestCost) {
                    waterLevel = (int) (newCost * 1.02f);
                    lowestCost = newCost;
                }
                cost = newCost;
                undoManager.ClearHistory();
            }
        }
    }
    
    private void randomSwap() {
        Collections.shuffle(lectureList, rand);
        for (int i=0; i<lectureList.size(); ++i) {
            Lecture lec1 = lectureList.get(i);
            for (int j=0; j<lectureList.size(); ++j) {
                Lecture lec2 = lectureList.get(j);
                if (lec1.getCourse() != lec2.getCourse()) {
                    if (swap(lec1, lec2))
                        return;
                }
            }
        }
    }
    
    private boolean swap(Lecture lec1, Lecture lec2) {
        if (lec1.canSwap(lec2) && lec2.canSwap(lec1)) {
            undoManager.Swap(lec1, lec2);
            return true;
        }
        return false;
    }
    
    private void randomTimeMove() {
        Collections.shuffle(lectureList, rand);
        Collections.shuffle(daysChoice, rand);
        Collections.shuffle(slotsChoice, rand);
        
        for (int i=0; i<lectureList.size(); ++i) {
            Lecture lec = lectureList.get(i);

            for (int d = 0; d < daysChoice.size(); ++d) {
                for (int s = 0; s < slotsChoice.size(); ++s) {
                    if (lec.getDay() != d || lec.getTimeSlot() != s) {
                        if (timeMove(lec, d, s))
                            return;
                    }
                }
            }
        }
    }
    
    private boolean timeMove(Lecture lec, int day, int timeSlot) {
        if (lec.isCompatibleWith(lec.getRoom(), day, timeSlot)) {
            undoManager.Move(lec, lec.getRoom(), day, timeSlot);
            return true;
        }
        return false;
    }
    
    private void randomRoomMove() {
        Collections.shuffle(lectureList, rand);
        Collections.shuffle(roomList, rand);
        
        for (int i=0; i<lectureList.size(); ++i) {
            Lecture lec = lectureList.get(i);
            
            for (int j=0; j<roomList.size(); ++j) {
                Room r = roomList.get(j);
                if (r != lec.getRoom()) {
                    if (roomMove(lec, r))
                        return;
                }
            }
        }
    }
    
    private boolean roomMove(Lecture lec, Room r) {
        if (lec.isCompatibleWith(r, lec.getDay(), lec.getTimeSlot())) {
            undoManager.Move(lec, r, lec.getDay(), lec.getTimeSlot());
            return true;
        }
        return false;
    }

    private void randomRoomStabilityMove() {
        Collections.shuffle(courseList, rand);

        for (int i=0; i<courseList.size(); ++i) {
            Course c = courseList.get(i);
            if (c.getRoomStabilityPenalty() > 0) {
                if (roomStabilityMove(c))
                    return;
            }
        }
    }

    private boolean roomStabilityMove(Course c) {
        Room r = c.getMajorityRoom();
        if (r == null)
            return false;

        boolean isSuccess = false;

        lecture_loop:
        for (Lecture lec : c.getLectures()) {
            if (lec.getRoom() == r)
                continue;

            Collections.shuffle(daysChoice, rand);
            Collections.shuffle(slotsChoice, rand);

            for (int d = 0; d < daysChoice.size(); ++d) {
                for (int s = 0; s < slotsChoice.size(); ++s) {
                    if (lec.isCompatibleWith(r, daysChoice.get(d), slotsChoice.get(s))) {
                        move(lec, r, daysChoice.get(d), slotsChoice.get(s));
                        isSuccess = true;
                        continue lecture_loop;
                    }
                }
            }
        }
        return isSuccess;
    }

    private boolean move(Lecture lec, Room r, int day, int timeSlot) {
        if (lec.isCompatibleWith(r, day, timeSlot)) {
            undoManager.Move(lec, r, day, timeSlot);
            return true;
        }
        return false;
    }

    private void printSolution() {
        for (Course c : courseMap.values()) {
            for (Lecture l : c.getLectures()) {
                System.out.println(l);
            }
        }
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

        for (int i = 0; i < days; ++i) {
            daysChoice.add(i);
        }

        for (int i = 0; i < slotsPerDay; ++i) {
            slotsChoice.add(i);
        }

        //read COURSES information
        in.skipLine(1);
        for (int i = 0; i < numOfCourses; ++i) {
            Course c = Course.create(in.readLine(), teacherList, days, slotsPerDay);
            courseMap.put(c.getName(), c);
        }
        courseList.addAll(courseMap.values());

        //read ROOMS information
        in.skipLine(1);
        for (int i = 0; i < numOfRooms; ++i) {
            roomList.add(Room.create(in.readLine(), days, slotsPerDay));
        }

        for (Course c : courseMap.values()) {
            lectureList.addAll(c.getLectures());
        }

        //read CURRICULA information
        in.skipLine(1);
        for (int i = 0; i < numOfCurricula; ++i) {
            curriculumList.add(Curriculum.create(in.readLine(), courseMap, days, slotsPerDay));
        }

        //read UNAVAILABILITY information
        in.skipLine(1);
        for (int i = 0; i < numOfConstraints; ++i) {
            Unavailability.updateCourse(in.readLine(), courseMap);
        }

        validator = new Validator(courseMap.values(), curriculumList, roomList, teacherList, days, slotsPerDay);

        in.close();
    }

    class TimeLimitter extends TimerTask {

        @Override
        public void run() {
            stopFlag = true;
        }

    }
}