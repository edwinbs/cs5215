package entity;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.StringTokenizer;

public class Curriculum {
    
    String              name;
	int                 numOfCourses;
	ArrayList<Course>   member_list;

    public static Curriculum create(String line, HashMap<String, Course> courses)
    {
        StringTokenizer strTok = new StringTokenizer(line);

        String sName = strTok.nextToken();
        int tmpNumOfCourse = Integer.parseInt(strTok.nextToken());
        ArrayList<Course> tmpMemberList = new ArrayList<Course>();

        for (int j=0; j<tmpNumOfCourse; ++j)
            tmpMemberList.add(courses.get(strTok.nextToken()));
        
        return new Curriculum(sName, tmpNumOfCourse, tmpMemberList);
    }
    
	public Curriculum(String name, int numOfCourses, ArrayList<Course> member_list) {
		super();
        this.name = name;
        this.numOfCourses = numOfCourses;
		this.member_list = member_list;
	}

}
