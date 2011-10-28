package entity;

import java.util.HashMap;
import java.util.StringTokenizer;

public class Unavailability {

    public static void updateCourse(String line, HashMap<String, Course> courses) {
        StringTokenizer strTok = new StringTokenizer(line);

        Course course = courses.get(strTok.nextToken());
        course.setUnavailable(Integer.parseInt(strTok.nextToken()), Integer.parseInt(strTok.nextToken()));
    }
}
