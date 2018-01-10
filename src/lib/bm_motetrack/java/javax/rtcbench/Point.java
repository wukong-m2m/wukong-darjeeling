package javax.rtcbench;

import javax.rtc.RTC;

public class Point {
    public short x, y, z;

    public Point() {
        this.x = 0;
        this.y = 0;
        this.z = 0;
    }

    static void Point_centroidLoc(Point retLocPtr, Point points[])
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
