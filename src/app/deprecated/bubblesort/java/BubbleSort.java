/*
 * BubbleSort.java
 * 
 * Copyright (c) 2008-2010 CSIRO, Delft University of Technology.
 * 
 * This file is part of Darjeeling.
 * 
 * Darjeeling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Darjeeling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Darjeeling.  If not, see <http://www.gnu.org/licenses/>.
 */
 
import javax.darjeeling.*;

public class BubbleSort
{
	public static void main(String args[])
	{
		int i,j,l;

		short NUMNUMBERS = 256;
		short numbers[] = new short[NUMNUMBERS];

		Darjeeling.print("START");

		for (l=0; l<100; l++)
		{

			for (i=0; i<NUMNUMBERS; i++)
				numbers[i] = (short)(NUMNUMBERS - 1 - i);

			for (i=0; i<NUMNUMBERS; i++)
			{
				for (j=0; j<NUMNUMBERS-i-1; j++)
					if (numbers[j]>numbers[j+1])
					{
						short temp = numbers[j];
						numbers[j] = numbers[j+1];
						numbers[j+1] = temp;
					}
			}

		}
		
		Darjeeling.print("END");
	}
}
