package com.github.aarcangeli.serioussamandroid;

import static com.github.aarcangeli.serioussamandroid.MainActivity.TAG;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Build;
import android.os.Environment;
import android.util.Log;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.Toast;

import androidx.annotation.NonNull;

import com.liulishuo.filedownloader.FileDownloader;
import com.liulishuo.filedownloader.FileDownloadListener;
import com.liulishuo.filedownloader.BaseDownloadTask;
import java.io.File;

public class CacheDownloader {
    public interface DownloadCallback {
        void onDownloadComplete(Boolean downloaded);
    }

    private AlertDialog.Builder builder;
    private AlertDialog progressDialog;
    private ProgressBar progressBar;
    private static String gro = "SE1_00.gro";
    private static String zipName = "hgfjgfjfjtse.zip";
    private final Context context;
    private String folderPath = "/sdcard/";

    public CacheDownloader(Context context) {
        this.context = context;
    }

    public AlertDialog.Builder getDialogProgressBar(String title, String message, Boolean cancelable) {

        if (builder == null) {
            builder = new AlertDialog.Builder(context);
            builder.setTitle(title);
            builder.setMessage(message);
            builder.setCancelable(cancelable);
            progressBar = new ProgressBar(context, null, android.R.attr.progressBarStyleHorizontal);
            LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT);
            progressBar.setLayoutParams(lp);
            builder.setView(progressBar);
        }
        return builder;
    }

    @NonNull
    private static File getCheckFile() {
        if (BuildConfig.home.endsWith("TFE")) {
            zipName = "sadfasfsafastfe.zip";
        }
        if (BuildConfig.home.endsWith("TFE")) {
            gro = "1_00c.gro";
        }
        return new File(Environment.getExternalStorageDirectory(), BuildConfig.home + "/" + gro).getAbsoluteFile();
    }
    // Проверка папки, если папки нет вызов функции showDownloadDialog
    public void checkFolderAndDownloadFile(final DownloadCallback callback) {
        File fil = getCheckFile();
        if (!fil.exists()) {
            Log.wtf(TAG, "Gro not found, starting download");
            showDownloadDialog(callback);
        } else {
            Log.wtf(TAG, "Gro found, starting game!");
            callback.onDownloadComplete(false);
        }
    }
    // Диалоговое окно спрашивающее качаем или нет, если да - вызов функции downloadFile
    private void showDownloadDialog(final DownloadCallback callback) {
        new AlertDialog.Builder(context)
                .setTitle("Файл " + gro + " не найден")
                .setMessage("Скачать и установить кэш " + BuildConfig.home + "?")
                .setPositiveButton("Да", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        downloadFile(callback);
                        dialog.dismiss();
                    }
                })
                .show();
    }

    private void updateProgress(int progress) {
        if (progressBar != null) {
            progressBar.setProgress(progress);
        }
    }

    private void downloadFile(final DownloadCallback callback) {
        progressDialog = getDialogProgressBar( "Загрузка игровых данных", "Начинается скачивание", false).create();
        progressDialog.show();
        new Thread(new Runnable() {
            public void run() {
                String url = "https://github.com/slowpoke1337one/zadacha44/releases/download/v1.0.0/" + zipName;
                FileDownloader.setup(context);
                FileDownloader.getImpl().create(url)
                        .setPath(Environment.getExternalStorageDirectory().getAbsolutePath()+"/" + BuildConfig.home + ".zip")
                        .setListener(new FileDownloadListener() {
                            @Override
                            protected void pending(BaseDownloadTask task, int soFarBytes, int totalBytes) {
                                Log.wtf(TAG, "Preparing to download " + zipName);
                            }

                            @Override
                            protected void progress(BaseDownloadTask task, int soFarBytes, int totalBytes) {
                                final int progress = (int) ((soFarBytes * 100L) / totalBytes);
                                ((MainActivity) context).runOnUiThread(new Runnable() {
                                    public void run() {
                                        progressDialog.setMessage("Скачивание кэша: " + progress + "%");
                                        updateProgress(progress);
                                    }
                                });
                            }

                            @Override
                            protected void completed(BaseDownloadTask task) {
                                Log.wtf(TAG, "Download completed!");
                                ((MainActivity) context).runOnUiThread(new Runnable() {
                                    public void run() {
                                        Toast.makeText(context, "Загрузка успешно завершена", Toast.LENGTH_LONG).show();
                                        progressDialog.setMessage("Загрузка успешно завершена");
                                        progressDialog.dismiss();
                                        callback.onDownloadComplete(true);
                                    }
                                });
                            }


                            @Override
                            protected void paused(BaseDownloadTask task, int soFarBytes, int totalBytes) {
                                //mozhet byt kogda nibut budet used
                            }

                            @Override
                            protected void error(BaseDownloadTask task, final Throwable e) {
                                e.printStackTrace();
                                Log.wtf(TAG, "Error: " + e.getMessage());
                                ((MainActivity) context).runOnUiThread(new Runnable() {
                                    public void run() {
                                        progressDialog.setMessage(e.getMessage());
                                    }
                                });
                            }

                            @Override
                            protected void warn(BaseDownloadTask task) {
                                //mozhet byt kogda nibut budet used
                            }
                        }).start();
            }
        }).start();
    }
}
