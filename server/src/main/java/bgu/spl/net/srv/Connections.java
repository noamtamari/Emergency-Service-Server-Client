package bgu.spl.net.srv;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    void addToChannels(String destination, int connectionId, String subscriptionId);

    boolean removeSubscription(String subscriptionId, int connectionId);

    boolean isSubscribed(int connectionId, String destination);
}
