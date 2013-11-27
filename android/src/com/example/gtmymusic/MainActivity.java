package com.example.gtmymusic;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.security.DigestInputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;

import android.os.AsyncTask;
import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends Activity {
	
	private ArrayList<ListItem> mostRecentList = null;
	private ArrayList<ListItem> mostRecentDiff = null;

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
	
	private class ListItem
	{
		public String hash;
		public String filename;
		
		public ListItem(String h, String f)
		{
			hash = h;
			filename = f;
		}
	}
	
	private ListItemArray diffArrays(ListItemArray authoritative, ListItemArray other)
	{
		ListItemArray diff = new ListItemArray();
		
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
					items.addItem(new ListItem(getHexString(hashBytes), filename));
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
        		Socket s = new Socket("130.207.114.22", 6079);
        		OutputStream out = s.getOutputStream();
        		
        		String command = "LIST";
        		byte[] toSend = new byte[command.length() + 1];
        		java.util.Arrays.fill(toSend, (byte)0);
        		
        		byte[] commandBytes = command.getBytes();
        		System.arraycopy(commandBytes, 0, toSend, 0, commandBytes.length);
        		toSend[command.length()] = 0;	//last byte
        		
        		out.write(toSend);
        		
        		//BufferedReader reader = new BufferedReader(new InputStreamReader(s.getInputStream()));
                BufferedInputStream stream = new BufferedInputStream(s.getInputStream());	//size?
        		//read
                //byte[] bytes = new byte[32];
                byte[] countBytes = new byte[4];
                int bytesRead = stream.read(countBytes, 0, 4);
                if (bytesRead < 4)
                {
                	s.close();
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
                		s.close();
                		return "Read unexpected number of item bytes";
                	}
                	
                	
                	//builder.append(", ");
                	String hexString = getHexString(itemBytes);
                	
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
                		s.close();
                		return "Error reading a filename";
                	}

                	//bb = ByteBuffer.wrap(itemBytes, 16, nullTerm-16);
                	//builder.append(new String(bb.array()));	//no... uses full backing array
                	byte[] filenameBytes = new byte[nullTerm-16];
                	System.arraycopy(itemBytes, 16, filenameBytes, 0, nullTerm-16);
                	//builder.append(new String(filenameBytes));
                	
                	//builder.append("\n");
                	
                	items.addItem(new ListItem(hexString, new String(filenameBytes)));
                }
                
                //Close connection
                s.close();
        	}
        	catch (UnknownHostException e) {
        		// TODO Auto-generated catch block
        		e.printStackTrace();
        	} catch (IOException e) {
        		// TODO Auto-generated catch block
        		e.printStackTrace();
        	}
			
			return items.toString();
		}
		
		protected void onPostExecute(String result) {
            appendMessage(result);
        }
	}

}
