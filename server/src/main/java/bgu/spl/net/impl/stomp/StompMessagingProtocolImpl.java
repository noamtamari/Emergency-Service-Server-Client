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
        T response = processingMsg(message);
        /// change
        if (response != null){
            connections.send(connectionId, response);
        }
        Frame frame = new Frame("RECEIPT");
        frame.addHeader("receipt", ((Frame)message).getHeader("receipt"));
        connections.send(connectionId, frame);
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    public T processingMsg(T msg){
        if(((Frame)msg).getType().equals("SUBSCRIBE") | ((Frame)msg).getType().equals("UNSUBSCRIBE") | ((Frame)msg).getType().equals("DISCONNECT")){
            return null;
        }
        if(((Frame)msg).getType().equals("SEND")){
            Frame frame = new Frame(("MESSAGE"), ((Frame)msg).getBody());
            return handleSend(msg);
        }
    }
    public T handleSend(T msg){
        Frame frame = new Frame("SEND", ((Frame)msg).getBody());

    }

    public T handleSubscribe(T msg){

    }
}
