package entity;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.StringTokenizer;

public class Curriculum {
    
    String              name;
	int                 numOfCourses;
	ArrayList<Course>   member_list;
    boolean[][]         availability;

    public static Curriculum create(String line, HashMap<String, Course> courses, int days, int slotsPerDay)
    {
        StringTokenizer strTok = new StringTokenizer(line);

        String sName = strTok.nextToken();
        int tmpNumOfCourse = Integer.parseInt(strTok.nextToken());
        ArrayList<Course> tmpMemberList = new ArrayList<Course>();
        
        Curriculum inst = new Curriculum(sName, tmpNumOfCourse, tmpMemberList, days, slotsPerDay);

        for (int j=0; j<tmpNumOfCourse; ++j)
        {
            Course course = courses.get(strTok.nextToken());
            course.setCurriculum(inst);
            tmpMemberList.add(course);
        }
        
        return inst;
    }
    
	public Curriculum(String name, int numOfCourses, ArrayList<Course> member_list, int days, int slotsPerDay) {
		super();
        this.name = name;
        this.numOfCourses = numOfCourses;
		this.member_list = member_list;
        this.availability = new boolean[days][slotsPerDay];
	}

}
