package com.github.aarcangeli.serioussamandroid;

import android.os.AsyncTask;
import android.content.Intent;
import android.content.Context;
import android.content.DialogInterface;
import androidx.core.content.FileProvider;
import android.net.Uri;
import android.app.AlertDialog;

import org.json.JSONObject;
import org.json.JSONException;

import java.io.File;
import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.BufferedInputStream;
import java.net.URL;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URLConnection;

public class Updater {
	public int latestVersionCode;
	private Context context;
	private int curVersionCode;
	
	public Updater(Context context, int curVer) {
		this.context = context;
		this.curVersionCode = curVer;
	}
	
	public class DownloadUpdate extends AsyncTask<String,Integer,Void> {
		String path = "/sdcard/YourApp.apk";
		protected Void doInBackground(String... sUrl) {
			try {
				URL url = new URL(sUrl[0]);
				HttpURLConnection connection=(HttpURLConnection)url.openConnection();

				int fileLength = connection.getContentLength();

				// download the file
				InputStream input = new BufferedInputStream(url.openStream());
				OutputStream output = new FileOutputStream(path);

				byte data[] = new byte[1024];
				long total = 0;
				int count;
				while ((count = input.read(data)) != -1) {
					total += count;
					publishProgress((int) (total * 100 / fileLength));
					output.write(data, 0, count);
				}

				output.flush();
				output.close();
				input.close();
			} catch (Exception e) {
			}
			return null;
		}

		// begin the installation by opening the resulting file
		@Override
		protected void onPostExecute(Void result) {
			super.onPostExecute(result);
			Uri uri = FileProvider.getUriForFile(context,
									context.getApplicationContext().getPackageName() + ".provider", new File(path));
			Intent intent = new Intent();
			intent.setAction(Intent.ACTION_VIEW);
			intent.setDataAndType(uri, "application/vnd.android.package-archive" );
			intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
			intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			context.startActivity(intent);
		}	
	}

	public class ReadFileTask extends AsyncTask<Void,Void,JSONObject> {
		String updateURL = null; 
		@Override
		protected JSONObject doInBackground(Void... params)
		{

			String str = "https://raw.githubusercontent.com/Skyrimus/Serious-Sam-Android/master/update.json";
			URLConnection urlConn = null;
			BufferedReader bufferedReader = null;
			try
			{
				URL url = new URL(str);
				urlConn = url.openConnection();
				bufferedReader = new BufferedReader(new InputStreamReader(urlConn.getInputStream()));

				StringBuffer stringBuffer = new StringBuffer();
				String line;
				while ((line = bufferedReader.readLine()) != null)
				{
					stringBuffer.append(line);
				}

				return new JSONObject(stringBuffer.toString());
			}
			catch(Exception ex)
			{
				return null;
			}
			finally
			{
				if(bufferedReader != null)
				{
					try {
						bufferedReader.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
		}
		
		@Override
		protected void onPostExecute(JSONObject response) {
		  super.onPostExecute(response);
		  if (response != null) {
			try {
				if (BuildConfig.home.endsWith("TSE")) {
					updateURL = response.getString("url_tse");
				} else {
					updateURL = response.getString("url_tfe");
				}
				latestVersionCode = Integer.parseInt(response.getString("versionCode"));
				if (latestVersionCode > curVersionCode) {
					AlertDialog.Builder dlgAlert = new AlertDialog.Builder(context);
					dlgAlert.setMessage("Game update available! Download update now?");
					dlgAlert.setTitle("ATTENTION: UPDATE");
					dlgAlert.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
						@Override
						public void onClick(DialogInterface dialog, int which) {
							DownloadUpdate tsk=new DownloadUpdate();
							tsk.execute(updateURL);
						}
					});
					 dlgAlert.setNegativeButton("No", new DialogInterface.OnClickListener() {
						 @Override
					   public void onClick(DialogInterface dialog, int id) {
						   dialog.dismiss();
					   }
				   });
					dlgAlert.show();
				}
			} catch (JSONException ex) {
			}
		  }
		}
		
	}
}