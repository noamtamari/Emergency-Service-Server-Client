package bgu.spl.net.impl.stomp;

import java.sql.Connection;
import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

public class ConnectionsImpl implements Connections<T> {
    private ConcurrentHashMap<Integer, ConnectionHandler<T>> connections;

    public ConnectionsImpl() {
        connections = new ConcurrentHashMap<>();
    }

    @Override
    public boolean send(int connectionId, T msg) {

    }

    @Override
    public void send(String channel, T msg) {
        // IMPLEMENT IF NEEDED
    }

    @Override
    public void disconnect(int connectionId) {
        // IMPLEMENT IF NEEDED
    }

    public void connect(int connectionId) {
        // IMPLEMENT IF NEEDED
    }
}
