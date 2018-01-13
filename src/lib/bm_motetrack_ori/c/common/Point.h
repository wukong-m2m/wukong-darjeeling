/*
 * Copyright (c) 2005
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/** 
 * A simple point data structure.
 *
 * <pre>URL: http://www.eecs.harvard.edu/~konrad/projects/motetrack</pre>
 * @author Konrad Lorincz
 * @version 2.0, January 5, 2005
 */
#ifndef POINT_H
#define POINT_H             


struct Point {
    uint16_t x;
    uint16_t y;
    uint16_t z;
};
typedef struct Point Point;


// ======================== Methods ===============================
/**
 * Initializes that point by setting all fielsds to zero.
 */
static inline void Point_init(Point *pointPtr)
{
    pointPtr->x = 0;
    pointPtr->y = 0;
    pointPtr->z = 0;
}

/**
 * Computes the centroid of a number of points.
 * @param retLocPtr  returns the centroid point through this pointer
 * @param points[]  an array of the points whos centroid to compute
 * @param nbrPoints  the number of points;  must nut be larger that the size of the points array!
 */
static inline void Point_centroidLoc(Point *retLocPtr, Point points[], uint16_t nbrPoints)
{
    uint16_t i = 0;
    uint32_t x=0, y=0, z=0;  // to prevent overflow from adding multiple 16-bit points


    for (i = 0; i < nbrPoints; ++i) {
        x += points[i].x;
        y += points[i].y;
        z += points[i].z;
    }

    retLocPtr->x = (uint16_t) (x / i);
    retLocPtr->y = (uint16_t) (y / i);
    retLocPtr->z = (uint16_t) (z / i);
}

// /**
//  * Prints the content of the point.
//  */
// inline void Point_print(Point *pPtr)
// {
//     printfUART("<x=%i, y=%i, z=%i [Point]>", pPtr->x, pPtr->y, pPtr->z);
// }

#endif
