package entity;

import java.util.StringTokenizer;

public class Course {

	String  name;
    String  teacherName;
	int     numOfLectures;
	int     minWorkingDay;
	int     minOfStudents;
	
	public static Course create(String line)
    {
        StringTokenizer strTok = new StringTokenizer(line);
        return new Course(strTok.nextToken(),
                           strTok.nextToken(),
                           Integer.parseInt(strTok.nextToken()),
                           Integer.parseInt(strTok.nextToken()),
                           Integer.parseInt(strTok.nextToken()));
    }

	public Course(String name, String teacherName,
			int numOfLectures, int minWorkingDay, int minOfStudents) {
		super();
		this.name = name;
		this.teacherName = teacherName;
		this.numOfLectures = numOfLectures;
		this.minWorkingDay = minWorkingDay;
		this.minOfStudents = minOfStudents;
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

	public String getTeacherName() {
		return teacherName;
	}

	public void setTeacherName(String teacherName) {
		this.teacherName = teacherName;
	}
    
}
