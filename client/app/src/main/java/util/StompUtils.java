package util;

import android.util.Log;

import ua.naiksoftware.stomp.StompClient;

public class StompUtils {
    @SuppressWarnings({"ResultOfMethodCallIgnored", "CheckResult"})
    public static void lifecycle(StompClient stompClient) {
        stompClient.lifecycle().subscribe(lifecycleEvent -> {
            switch (lifecycleEvent.getType()) {
                case OPENED:
                    Log.d("IOT", "Stomp connection opened");
                    break;

                case ERROR:
                    Log.e("IOT", "Error", lifecycleEvent.getException());
                    break;

                case CLOSED:
                    Log.d("IOT", "Stomp connection closed");
                    break;
            }
        });
    }
}
