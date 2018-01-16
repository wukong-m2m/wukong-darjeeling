package javax.rtcbench;

import javax.rtc.RTC;
import javax.rtc.Lightweight;

public class Point {
    public short x, y, z;

    public Point() {
        init(this);
    }
    @Lightweight
    public static void init(Point pointPtr) {
        pointPtr.x = 0;
        pointPtr.y = 0;
        pointPtr.z = 0;
    }

    static void centroidLoc(Point retLocPtr, Point points[])
    {
        short i = 0;
        short nbrPoints = (short)points.length;

        int x=0, y=0, z=0;  // to prevent overflow from adding multiple 16-bit points

        for (i = 0; i < nbrPoints; ++i) {
            x += points[i].x;
            y += points[i].y;
            z += points[i].z;
        }

        retLocPtr.x = (short) (x / i);
        retLocPtr.y = (short) (y / i);
        retLocPtr.z = (short) (z / i);
    }
}
