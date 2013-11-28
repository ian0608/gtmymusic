package com.example.gtmymusic;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.DigestInputStream;
import java.security.MessageDigest;
import java.util.AbstractMap.SimpleEntry;
import java.util.ArrayList;

import android.os.AsyncTask;
import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.view.Menu;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends Activity {
	
	private ListItemArray mostRecentList = null;
	private ListItemArray mostRecentDiff = null;
	private Socket s = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
	
	public void list(View view)
	{
		/*
		ListItemArray currentDir = itemsCurrentDir();
		if (currentDir != null)
			displayMessage(currentDir.toString());
		else
			displayMessage("error getting current directory items");
		*/
		
		displayMessage("Files on server:\n");
		new ListTask().execute();
	}
	
	public void diff(View view)
	{
		if (mostRecentList == null)
		{
			displayMessage("Run LIST first");
		}
		else
		{
			ListItemArray current = itemsCurrentDir();
			if (current == null)
			{
				displayMessage("Error getting current directory items");
				return;
			}
			mostRecentDiff = diffArrays(mostRecentList, current);
			displayMessage("Client is missing:\n\n");
			appendMessage(mostRecentDiff.toString());
			
		}
	}
	
	public void cap(View view)
	{
		EditText edit = (EditText)findViewById(R.id.edit_cap);
		String capString = edit.getText().toString();
		Integer cap = null;
		try
		{
			cap = Integer.parseInt(capString);
		}
		catch (NumberFormatException e)
		{
			e.printStackTrace();
			displayMessage("Improper format for cap");
			return;
		}
		new CapTask().execute(cap);
	}
	
	public void pull(View view)
	{
		if (mostRecentDiff == null)
		{
			displayMessage("Run DIFF first");
		}
		else
		{
		displayMessage("Downloading:\n");
			new PullTask().execute();
		}
	}
	
	public void displayMessage(String message)
    {
		TextView text = (TextView)findViewById(R.id.message_text);
        text.setText(message);
    }
	
	public void appendMessage(String message)
    {
		TextView text = (TextView)findViewById(R.id.message_text);
        text.setText(text.getText() + message);
    }
	
	public void displayCapValue(Integer value)
	{
		TextView text = (TextView)findViewById(R.id.current_cap_value);
		text.setText(value.toString());
	}
	
	private class ListItem
	{
		public String hash;
		public String filename;
		public byte[] hashBytes;
		
		public ListItem(String h, String f, byte[] hb)
		{
			hash = h;
			filename = f;
			hashBytes = hb;
		}
	}
	
	private ListItemArray diffArrays(ListItemArray authoritative, ListItemArray other)
	{
		ListItemArray diff = new ListItemArray();
		for(ListItem authoritativeItem : authoritative.array)
		{
			boolean found = false;
			for(ListItem item : other.array)
			{
				if (authoritativeItem.hash.equals(item.hash) && authoritativeItem.filename.equals(item.filename))
				{
					found = true;
					break;
				}
			}
			if (found == false)
			{
				diff.addItem(new ListItem(authoritativeItem.hash, authoritativeItem.filename, authoritativeItem.hashBytes));
			}
		}
		return diff;
	}
	
	private ListItemArray itemsCurrentDir()
	{
		ListItemArray items = new ListItemArray();
		File dir = getFilesDir();
		for (File file : dir.listFiles()) {
		    if (file.isFile())
		    {
		        String filename = file.getName();
		        
		        try {
		        	FileInputStream fileIn = new FileInputStream(file);
					MessageDigest digest = MessageDigest.getInstance("MD5");
					DigestInputStream digestIn = new DigestInputStream(fileIn, digest);
					byte[] buffer = new byte[8192];
					while (digestIn.read(buffer) != -1)
					{
					}
					digestIn.close();
					byte[] hashBytes = digest.digest();
					items.addItem(new ListItem(getHexString(hashBytes), filename, hashBytes));
				} catch (Exception e) {
					e.printStackTrace();
					return null;
				}
		    }
		}
		
		return items;
	}
	
	private class ListItemArray
	{
		public ArrayList<ListItem> array;
		
		public ListItemArray()
		{
			array = new ArrayList<ListItem>();
		}
		
		public String toString()
		{
			StringBuilder builder = new StringBuilder();
			for(ListItem item : array)
			{
				builder.append(item.hash);
				builder.append("\n");
				builder.append(item.filename);
				builder.append("\n\n");
			}
			return builder.toString();
		}
		
		public void addItem(ListItem item)
		{
			array.add(item);
		}
	}

	private String getHexString(byte[] bytesArray)
	{
		StringBuilder hexStringBuilder = new StringBuilder();
    	for (int j=0; j<16; j++)
        {
        	//if (j==0 && itemBytes[j] < 16) builder.append("0");
        	//builder.append(Integer.toHexString((int) (itemBytes[j] & 0xff)));
    		String hex = Integer.toHexString((bytesArray[j] & 0xff));
    		if (hex.length() == 1)
    			hexStringBuilder.append('0');
    		hexStringBuilder.append(hex);
    		
        }
    	return hexStringBuilder.toString();
	}
	
	private class ListTask extends AsyncTask<Void, Void, String>
	{
		protected String doInBackground(Void... params)
		{
			//StringBuilder builder = new StringBuilder();
			ListItemArray items = new ListItemArray();
			//NETWORKING GOES HERE
        	try {
        		if (s==null)
        			s = new Socket("130.207.114.22", 6079);
        		OutputStream out = s.getOutputStream();
        		
        		String command = "LIST";
        		byte[] toSend = new byte[command.length()];
        		java.util.Arrays.fill(toSend, (byte)0);
        		
        		byte[] commandBytes = command.getBytes();
        		System.arraycopy(commandBytes, 0, toSend, 0, commandBytes.length);
        		
        		out.write(toSend);
        		
        		//BufferedReader reader = new BufferedReader(new InputStreamReader(s.getInputStream()));
                BufferedInputStream stream = new BufferedInputStream(s.getInputStream());	//size?
        		//read
                //byte[] bytes = new byte[32];
                byte[] countBytes = new byte[4];
                int bytesRead = stream.read(countBytes, 0, 4);
                if (bytesRead < 4)
                {
                	//s.close();
                	return "Error reading item count";
                }
                
                ByteBuffer bb = ByteBuffer.wrap(countBytes);
                //bb.order(ByteOrder.BIG_ENDIAN); default
                int count = bb.getInt();
                
                //TextView text = (TextView)findViewById(R.id.message_text);
                //text.setText(count + " items");
                //toReturn = bytesRead + " " + count;
                
                
                for (int i=0; i < count; i++)
                {
                	byte[] itemBytes = new byte[273];
                	bytesRead = stream.read(itemBytes, 0, 273);
                	if (bytesRead < 273)
                	{
                		//s.close();
                		return "Read unexpected number of item bytes";
                	}
                	
                	
                	//builder.append(", ");
                	byte[] hashBytes = new byte[16];
                	System.arraycopy(itemBytes, 0, hashBytes, 0, 16);
                	String hexString = getHexString(hashBytes);
                	
                	int nullTerm = -1;
                	
                	for (int k=16; k<273; k++)
                	{
                		if (itemBytes[k] == 0)
                		{
                			nullTerm = k;
                			break;
                		}
                	}
                	
                	if (nullTerm == -1)
                	{
                		//s.close();
                		return "Error reading a filename";
                	}

                	//bb = ByteBuffer.wrap(itemBytes, 16, nullTerm-16);
                	//builder.append(new String(bb.array()));	//no... uses full backing array
                	byte[] filenameBytes = new byte[nullTerm-16];
                	System.arraycopy(itemBytes, 16, filenameBytes, 0, nullTerm-16);
                	//builder.append(new String(filenameBytes));
                	
                	//builder.append("\n");
                	
                	items.addItem(new ListItem(hexString, new String(filenameBytes), hashBytes));
                }
                
                //Close connection
                //s.close();
        	}
        	catch (UnknownHostException e) {
        		// TODO Auto-generated catch block
        		e.printStackTrace();
        	} catch (IOException e) {
        		// TODO Auto-generated catch block
        		e.printStackTrace();
        	}
			
        	mostRecentList = items;
			return mostRecentList.toString();
		}
		
		protected void onPostExecute(String result) {
            appendMessage(result);
        }
	}
	
	private class CapTask extends AsyncTask<Integer, Void, SimpleEntry<String, Integer>>
	{
		protected SimpleEntry<String, Integer> doInBackground(Integer... params)
		{
			if (params.length != 1)
				return new SimpleEntry<String, Integer>("Error with integer parameter", -1);
			
			Integer cap = params[0];
			//StringBuilder builder = new StringBuilder();
			//NETWORKING GOES HERE
        	try {
        		if (s==null)
        			s = new Socket("130.207.114.22", 6079);
        		OutputStream out = s.getOutputStream();
        		
        		String command = "CAP ";
        		byte[] toSend = new byte[command.length() + 4];
        		java.util.Arrays.fill(toSend, (byte)0);
        		
        		byte[] commandBytes = command.getBytes();
        		System.arraycopy(commandBytes, 0, toSend, 0, commandBytes.length);
        		//toSend[command.length()] = 0;	//last byte
        		byte[] capBytes = ByteBuffer.allocate(4).order(ByteOrder.BIG_ENDIAN).putInt(cap).array();
        		if (capBytes.length != 4)
        		{
        			//s.close();
        			return new SimpleEntry<String, Integer>("Error converting cap to byte array", -1);
        		}
        		System.arraycopy(capBytes, 0, toSend, command.length(), capBytes.length);
        		
        		//if (true)
        		//	return new SimpleEntry<String, Integer> (new String(toSend), (int)toSend[7]);
        					
        		out.write(toSend);
        		
        		//BufferedReader reader = new BufferedReader(new InputStreamReader(s.getInputStream()));
                BufferedInputStream stream = new BufferedInputStream(s.getInputStream());	//size?
        		//read
                //byte[] bytes = new byte[32];
                byte[] ackBytes = new byte[5];
                int bytesRead = stream.read(ackBytes, 0, 5);
                if (bytesRead < 5)
                {
                	//s.close();
                	return new SimpleEntry<String, Integer>("Cap not accepted by server", -1);
                }
                
                String ackString = new String(ackBytes);
                if (ackString.equals("CAPOK"))
                {
                	return new SimpleEntry<String, Integer>("Cap accepted", cap);
                }
                else
                {
                	return new SimpleEntry<String, Integer>("Cap not accepted by server", -1);
                }
                
                
                //Close connection
                //s.close();
        	}
        	catch (UnknownHostException e) {
        		// TODO Auto-generated catch block
        		e.printStackTrace();
        	} catch (IOException e) {
        		// TODO Auto-generated catch block
        		e.printStackTrace();
        	}
        	
        	return new SimpleEntry<String, Integer>("Cap not accepted by server", -1);
		}
		
		protected void onPostExecute(SimpleEntry<String, Integer> result) {
			displayCapValue(result.getValue());
            displayMessage(result.getKey());
        }
	}
	
	private class PullTask extends AsyncTask<Void, Void, String>
	{
		protected String doInBackground(Void... params)
		{
			StringBuilder builder = new StringBuilder();
			if (mostRecentDiff.array.size() == 0)
				return "Client is up to date!";
			
			//NETWORKING GOES HERE
        	try {
        		if (s==null)
        			s = new Socket("130.207.114.22", 6079);
        		OutputStream out = s.getOutputStream();
        		
        		String command = "PULL";
        		byte[] toSend = new byte[command.length() + 4 + mostRecentDiff.array.size()*16];
        		java.util.Arrays.fill(toSend, (byte)0);
        		
        		byte[] commandBytes = command.getBytes();
        		System.arraycopy(commandBytes, 0, toSend, 0, commandBytes.length);
        		//toSend[command.length()] = 0;	//last byte
        		
        		byte[] countBytes = ByteBuffer.allocate(4).putInt(mostRecentDiff.array.size()).array();
        		if (countBytes.length != 4)
        		{
        			return "Error converting count to byte array";
        		}
        		System.arraycopy(countBytes, 0, toSend, command.length(), countBytes.length);
        		
        		for(int i=0; i<mostRecentDiff.array.size(); i++)
        		{
        			System.arraycopy(mostRecentDiff.array.get(i).hashBytes, 0, toSend, command.length() + 4 + i*16, 16);
        		}
        		
        		//if (true)
        		//	return new String(toSend);
        		
        		out.write(toSend);
        		
        		//BufferedReader reader = new BufferedReader(new InputStreamReader(s.getInputStream()));
                BufferedInputStream stream = new BufferedInputStream(s.getInputStream());
        		//read
                //byte[] bytes = new byte[32];
                countBytes = new byte[4];
                int bytesRead = stream.read(countBytes, 0, 4);
                if (bytesRead < 4)
                {
                	//s.close();
                	return "Error reading file count";
                }
                
                ByteBuffer bb = ByteBuffer.wrap(countBytes);
                //bb.order(ByteOrder.BIG_ENDIAN); default
                int count = bb.getInt();
                
                //if (true)
                //	return "File count " + count;
                
                for (int i=0; i < count; i++)
                {
                	byte[] filenameBytes = new byte[257];
                	bytesRead = stream.read(filenameBytes, 0, 257);
                	if (bytesRead < 257)
                	{
                		//s.close();
                		return "Read unexpected number of metadata bytes";
                	}
                	
                	int nullTerm = -1;
                	for (int k=0; k<257; k++)
                	{
                		if (filenameBytes[k] == 0)
                		{
                			nullTerm = k;
                			break;
                		}
                	}                	
                	if (nullTerm == -1)
                	{
                		//s.close();
                		return "Error reading a filename";
                	}

                	byte[] filenameBytesCutoff = new byte[nullTerm];
                	System.arraycopy(filenameBytes, 0, filenameBytesCutoff, 0, nullTerm);
                	String filename = new String(filenameBytesCutoff);
                	
                	byte[] filesizeBytes = new byte[4];
                	bytesRead = stream.read(filesizeBytes, 0, 4);
                    if (bytesRead < 4)
                    {
                    	//s.close();
                    	return "Error reading filesize";
                    }
                    
                    bb = ByteBuffer.wrap(filesizeBytes);
                    //bb.order(ByteOrder.BIG_ENDIAN); default
                    int filesize = bb.getInt();
                    
                    //if (true)
                    //	return "first filesize: " + filesize;
                    
                    //File file = new File(getFilesDir(), filename);
                    //file.createNewFile();
                    try {
                    	//FileOutputStream outputStream = openFileOutput(filename, 0);
                    	FileOutputStream outputStream = openFileOutput(filename, Context.MODE_PRIVATE);
                    	byte[] buffer = new byte[8192];
                    	int f=0;
                    	while (f<filesize)
                    	{
                    		bytesRead = stream.read(buffer, 0, filesize-f > 8192 ? 8192 : filesize - f);
                    		f += bytesRead;
                    		outputStream.write(buffer, 0, bytesRead);
                    	}
                    	
                    	builder.append(filename + "\n");
                    }
                    catch (Exception e)
                    {
                    	e.printStackTrace();
                    	return "Error reading/writing file " + filename;
                    }
                }
                
                
        	}
        	catch (UnknownHostException e) {
        		// TODO Auto-generated catch block
        		e.printStackTrace();
        	} catch (IOException e) {
        		// TODO Auto-generated catch block
        		e.printStackTrace();
        	}
			return builder.toString();
		}
		
		protected void onPostExecute(String result) {
            appendMessage(result);
        }
	}

}
