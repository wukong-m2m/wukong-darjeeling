/*
 * MessageList.java
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
 
package org.csiro.darjeeling.infuser.checkphase;

import java.io.PrintStream;
import java.util.ArrayList;

public class MessageList 
{
	
	private ArrayList<Message> messages;
	
	public MessageList()
	{
		messages = new ArrayList<Message>();
	}

	/**
	 * @param e
	 * @return
	 * @see java.util.ArrayList#add(java.lang.Object)
	 */
	public boolean add(Message e)
	{
		return messages.add(e);
	}

	public boolean add(Message.Type type, String fileName, int lineNumber, String message)
	{
		return add(new Message(type, fileName, lineNumber, message));
	}

	public boolean add(Message.Type type, String fileName, String message)
	{
		return add(new Message(type, fileName, -1, message));
	}

	/**
	 * @param index
	 * @return
	 * @see java.util.ArrayList#get(int)
	 */
	public Message get(int index)
	{
		return messages.get(index);
	}

	/**
	 * @return
	 * @see java.util.ArrayList#size()
	 */
	public int size()
	{
		return messages.size();
	}
	
	public int getErrorCount()
	{
		int ret = 0;
		for (Message message : messages)
			if (message.getType()==Message.Type.ERROR) ret++;
		return ret;
	}
	
	public int getWarningCount()
	{
		int ret = 0;
		for (Message message : messages)
			if (message.getType()==Message.Type.WARNING) ret++;
		return ret;
	}
	
	public Message[] getMessages()
	{
		Message[] ret = new Message[messages.size()];
		messages.toArray(ret);
		return ret;
	}
	
	public void print(PrintStream out)
	{
		for (Message message : messages)
			out.println(message);
        if(getErrorCount()>0 || getWarningCount()>0)
            out.printf("%d errors, %d warnings\n", getErrorCount(), getWarningCount());
	}

}
