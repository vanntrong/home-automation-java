package com.javaiot.server;

import com.google.auth.oauth2.GoogleCredentials;
import com.google.firebase.FirebaseApp;
import com.google.firebase.FirebaseOptions;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

import java.io.*;
import java.util.Objects;

@SpringBootApplication
public class ServerApplication {

	public static void main(String[] args) {
		SpringApplication.run(ServerApplication.class, args);
		try {
			ClassLoader classLoader = ServerApplication.class.getClassLoader();

			File file = new File(Objects.requireNonNull(classLoader.getResource("serviceAccountKey.json")).getFile());

			InputStream serviceAccount = new FileInputStream(file.getAbsolutePath());
			GoogleCredentials credentials = GoogleCredentials.fromStream(serviceAccount);
			FirebaseOptions options = new FirebaseOptions.Builder()
					.setCredentials(credentials)
					.setDatabaseUrl("https://home-automation-56421-default-rtdb.asia-southeast1.firebasedatabase.app")
					.build();
			FirebaseApp.initializeApp(options);
		}
		catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		catch (IOException e) {
			e.printStackTrace();
		}
	}
}
