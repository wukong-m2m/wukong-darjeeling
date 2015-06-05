/*
 * GCTest.java
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
 
import javax.fleck.Leds;
import javax.darjeeling.*;


public class GCTest
{

	private static class TreeNode
	{
	
		private TreeNode left,right;
		private byte value;
	
		public TreeNode(byte value)
		{
			right = left = null;
			this.value = value;
		}
	
		public byte getValue()
		{
			return value;
		}
	
		public void insert(TreeNode node)
		{
			if (node.getValue()<value)
				insertLeft(node);
			else
				insertRight(node);
		}
	
		private void insertLeft(TreeNode node)
		{
			if (left==null)
				left=node;
			else
				left.insert(node);
		}
	
		private void insertRight(TreeNode node)
		{
			if (right==null)
				right=node;
			else
				right.insert(node);
		}
	
		public TreeNode getLeft()
		{
			return left;
		}

		public TreeNode getRight()
		{
			return right;
		}

	}

	public static int recurseTree(TreeNode node, int i, byte[] targetArray)
	{
		int ret = 0;
		if (node==null) return 0;
		
		ret += recurseTree(node.getLeft(), i, targetArray);
		
		targetArray[i+ret] = node.getValue();
		ret++;
		
		ret += recurseTree(node.getRight(), i + ret, targetArray);
		
		return ret;
	}

	public static void treeSortTest(int testBase)
	{

		// int numbers[] = new int[] { 21,6,36,76,7,97,94,30,90,86,13,80,84,79,28,55,36,95,23,82,57,73,28,46,48,94,18,23,86,100,47,42,39,33,52,98,77,81,86,64,27,70,91,42,6,95,8,6,36,71,79,32,27,34,87,100,85,90,69,12,73,70,76,65,51,21,4,5,82,77,63,87,12,11,69,79,12,35,43,35,67,16,38,78,60,6,79,92,43,69,57,74,58,21,45,69,45,55,73,24 };
		byte numbers[] = new byte[] { 
				21,  6, 36, 76,  7, 97, 94, 30, 90, 86,
				13, 80, 84, 79, 28, 55, 36, 95, 23, 82
				};		

		TreeNode rootNode = new TreeNode(numbers[0]);
		for (int i=1; i<numbers.length; i++)
			rootNode.insert(new TreeNode(numbers[i]));
		
		recurseTree(rootNode, 0, numbers);
		
		boolean isAscending = true;
		for (int i=0; i<numbers.length-1; i++)
			isAscending &= numbers[i]<=numbers[i+1];
		
		Darjeeling.assertTrue(testBase + 0, isAscending);
		
	}

    public static void main(String args[])
    {
    	for (int i=0; i<100; i++)
    		treeSortTest(i);
    }
    
}
