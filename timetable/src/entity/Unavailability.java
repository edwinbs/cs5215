package entity;

import java.util.HashMap;
import java.util.StringTokenizer;

public class Unavailability {
	
	Course  course;
	int     day;
	int     period;
    
    public static Unavailability create(String line, HashMap<String, Course> courses)
    {
        StringTokenizer strTok = new StringTokenizer(line);
        return new Unavailability(courses.get(strTok.nextToken()),
                               Integer.parseInt(strTok.nextToken()),
                               Integer.parseInt(strTok.nextToken()));
    }

	public Unavailability(Course course, int day, int period) {
		super();
		this.course = course;
		this.day = day;
		this.period = period;
	}

	public Course getCourse() {
		return course;
	}

	public void setCourse(Course course) {
		this.course = course;
	}

	public int getDay() {
		return day;
	}

	public void setDay(int day) {
		this.day = day;
	}

	public int getPeriod() {
		return period;
	}

	public void setPeriod(int period) {
		this.period = period;
	}

}
