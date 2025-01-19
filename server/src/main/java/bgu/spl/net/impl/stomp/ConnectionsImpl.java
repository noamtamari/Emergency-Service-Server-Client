package bgu.spl.net.impl.stomp;

import java.sql.Connection;
import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

public class ConnectionsImpl<T> implements Connections<T> {
    private ConcurrentHashMap<Integer, ConnectionHandler<T>> connections;
    private ConcurrentHashMap<String,  ConcurrentHashMap<Integer, ConnectionHandler<T>>> channels;//map of channels to connections

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

    // Sends a message T to clients subscribed to the channel
    @Override
    public void send(String channel, T msg) {
        ConcurrentHashMap<Integer, ConnectionHandler<T>> channelConnections = channels.get(channel);
        if (channelConnections != null && !channelConnections.isEmpty()) { //if there are cleints subscribed to the channel or the channel exists
            for (ConnectionHandler<T> connection : channelConnections.values()) {
                connection.send(msg);
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        connections.remove(connectionId);
    }

  
}
