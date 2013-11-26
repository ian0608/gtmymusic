package com.example.gtmymusic;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

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
			String toReturn = "";
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
                ByteBuffer bb = ByteBuffer.wrap(countBytes);
                bb.order(ByteOrder.BIG_ENDIAN);
                int count = bb.getInt();
                
                //TextView text = (TextView)findViewById(R.id.message_text);
                //text.setText(count + " items");
                toReturn = bytesRead + " " + count;
                
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
			
			return toReturn;
		}
		
		protected void onPostExecute(String result) {
            displayMessage(result);
        }
	}

}
