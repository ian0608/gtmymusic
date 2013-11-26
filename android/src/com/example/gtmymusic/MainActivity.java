package com.example.gtmymusic;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

import android.os.AsyncTask;
import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends Activity {

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
		new ListTask().execute();
	}
	
	public void displayMessage(String message)
    {
		TextView text = (TextView)findViewById(R.id.message_text);
        text.setText(message);
    }
	
	private class ListTask extends AsyncTask<Void, Void, String>
	{
		protected String doInBackground(Void... params)
		{
			StringBuilder builder = new StringBuilder();
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
                	
                	for (int j=0; j<16; j++)
                    {
                    	//if (j==0 && itemBytes[j] < 16) builder.append("0");
                    	//builder.append(Integer.toHexString((int) (itemBytes[j] & 0xff)));
                		String hex = Integer.toHexString((itemBytes[j] & 0xff));
                		if (hex.length() == 1)
                			builder.append('0');
                		builder.append(hex);
                		
                    }
                	builder.append(", ");
                	
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
                	//builder.append(new String(bb.array()));
                	byte[] filenameBytes = new byte[nullTerm-16];
                	System.arraycopy(itemBytes, 16, filenameBytes, 0, nullTerm-16);
                	builder.append(new String(filenameBytes));
                	
                	builder.append("\n");
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
			
			return builder.toString();
		}
		
		protected void onPostExecute(String result) {
            displayMessage(result);
        }
	}

}
