package bgu.spl.net.impl.stomp;

import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

public class ConnectionsImpl<T> implements Connections<T> {
    private ConcurrentHashMap<Integer, ConnectionHandler<T>> connections;
    private ConcurrentHashMap<String, ConcurrentHashMap<Integer, String>> channels;
    private int messageId = 0;

    public ConnectionsImpl() {
        connections = new ConcurrentHashMap<>();
        channels = new ConcurrentHashMap<>();
    }

    @Override
    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> connection = connections.get(connectionId);
        if (connection != null) {
            connection.send(msg);
            return true;
        }
        return false;
    }

    @SuppressWarnings("unchecked")
    @Override
    public void send(String channel, T msg) {
        ConcurrentHashMap<Integer, String> channelConnections = channels.get(channel);
        if (channelConnections != null && !channelConnections.isEmpty()) { // If there are clients subscribed to the channel or the channel exists
            for (Integer connectionId : channelConnections.keySet()) {
                ConnectionHandler<T> ch = connections.get(connectionId);
                if (ch != null){
                    Frame frame = new Frame((Frame)msg, channelConnections.get(connectionId), messageId++);
                    ch.send((T) frame);
                }
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        connections.remove(connectionId);
        for (ConcurrentHashMap<Integer, String> channel : channels.values()) {
            channel.remove(connectionId);
        }
    }

    public void connect(int connectionId, ConnectionHandler<T> handler) {
        connections.put(connectionId, handler);
    }

    // Getters and Setters

    public ConcurrentHashMap<Integer, ConnectionHandler<T>> getConnections() {
        return connections;
    }

    public void setConnections(ConcurrentHashMap<Integer, ConnectionHandler<T>> connections) {
        this.connections = connections;
    }

    public ConcurrentHashMap<String, ConcurrentHashMap<Integer, String>> getChannels() {
        return channels;
    }

    @Override
    public void addToChannels(String channel, int connectionId, String subscriptionId) {
        // Add the channel if it doesn't exist
        channels.putIfAbsent(channel, new ConcurrentHashMap<>());

        // Add the connection ID and subscription ID to the channel
        channels.get(channel).put(connectionId, subscriptionId); 
    }

    public int getMessageId() {
        return messageId;
    }

    public void setMessageId(int messageId) {
        this.messageId = messageId;
    }

    @Override
    public boolean isSubscribed(int connectionId,String channel) {
        if (channels.get(channel).get(connectionId) != null){
            return true;
        }
        return false;
    }

    @Override
    public boolean removeSubscription(String subscriptionId, int connectionId){
        boolean removed = false;
        for (ConcurrentHashMap<Integer, String> channel : channels.values()) {
            if (channel.get(connectionId) != null && channel.get(connectionId).equals(subscriptionId)){
                channel.remove(connectionId);
                removed = true;
            }
        }
        return removed;
    }

    // Increment and retrieve the next unique message ID
    public synchronized int getNextMessageId() {
        return messageId++;
    }
}
