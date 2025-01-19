package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Connections;

import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.api.StompMessagingProtocol;

public class StompMessagingProtocolImpl<T> implements StompMessagingProtocol<T> {
    private boolean shouldTerminate = false;
    private Connections<T> connections;
    int connectionId;

    @Override
    public void start(int connectionId, Connections<T> connections) {
        this.connections = connections;
        this.connectionId = connectionId;
    }

    @Override
    public void process(T message) {
        connections.send(connectionId, message);
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }
    
}
