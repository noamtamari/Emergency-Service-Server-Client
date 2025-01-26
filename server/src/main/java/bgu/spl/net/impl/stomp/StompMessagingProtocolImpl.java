package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.UserHandler;

import bgu.spl.net.api.StompMessagingProtocol;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<Frame> {
    private boolean shouldTerminate = false;
    private Connections<Frame> connections;
    private int connectionId;
    private UserHandler userHandler;

    @Override
    public void start(int connectionId, Connections<Frame> connections) {
        this.connections = connections;
        this.connectionId = connectionId;
        userHandler = UserHandler.getInstance();
    }

    @Override
    public void process(Frame message) {
        Frame process = processingMsg(message);
        if (process != null){
            // Send error frame
            connections.send(connectionId, process);
        } else {
            Frame frame = new Frame("RECEIPT");
            String receipt = message.getHeader("receipt");
            if (receipt == null) {
                frame.addHeader("receipt-id", "-100");
            } else {
                frame.addHeader("receipt-id", message.getHeader("receipt"));
            }
            connections.send(connectionId, frame);
        }
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    public Frame processingMsg(Frame msg) {
        if (msg.getType().equals("SUBSCRIBE")) {
            return handleSubscribe(msg);
        }
        if (msg.getType().equals("UNSUBSCRIBE")) {
            return handleUnsubscribe(msg);
        }
        if (msg.getType().equals("DISCONNECT")) {
            return handleDisconnect(msg);
        }
        if (msg.getType().equals("SEND")) {
            return handleSend(msg);
        }
        if (msg.getType().equals("CONNECT")) {
            return handleConnent(msg);
        }
        return null;
    }

    public Frame handleSubscribe(Frame msg) {
        String destination = msg.getHeader("destination");
        // Id is subscription id
        String subscriptionId = msg.getHeader("id");
        // subscriptionId is legal
        if (subscriptionId != null && !subscriptionId.equals("")) {
            // Desctination is legal
            if (destination != null && !destination.equals("")) {
                if (!connections.isSubscribed(connectionId,destination)){
                    connections.addToChannels(destination, connectionId, subscriptionId);
                }
            } else {
                return errorFrame("Illegal Destination", "The destination: '"+ destination + "' is illigal"  ,msg);
            }
        }
        // subscriptionId is illegal
        else {
            return errorFrame("Client's subscription id is illegal", "You are trying to subscribe with illigal subscription Id" ,msg); 
        }
        return null;
    }

    public Frame handleUnsubscribe(Frame msg) {
        String subscriptionId = msg.getHeader("id");
        // subscriptionId is legal
        boolean succseedUnsubscribe = false;
        if (subscriptionId != null && !subscriptionId.equals("")) {
            succseedUnsubscribe = connections.removeSubscription(subscriptionId, connectionId);
        }
        if (!succseedUnsubscribe) {
            return errorFrame("Illigal user subscription id: ","You tried to unsubscribe from a channel you are not subscribe to", msg); // add msg
        }
        return null;
    }

    public Frame handleDisconnect(Frame msg) {
        Frame frame = new Frame("RECEIPT");
        frame.addHeader("receipt-id", msg.getHeader("receipt"));
        connections.send(connectionId, frame);
        connections.disconnect(connectionId);
        synchronized(userHandler){
            userHandler.removeActiveUser(connectionId);
        }
        this.shouldTerminate = true;
        return null;
    }

    public Frame handleSend(Frame msg) {
        String destination = msg.getHeader("destination");
        if (destination != null) {
            if (connections.isSubscribed(connectionId, destination)) {
                Frame frame = new Frame(("MESSAGE"), msg.getMessageBody());
                connections.send(destination, frame); // Send to every user that is subscribe to destionation
            } else {
                return errorFrame("User not subscribed to this channel", "You are trying to report to channel "+ destination +" which you are not subscribed to. Please subscribe first", msg);
            }
        }
        return null;

    }

    public Frame handleConnent(Frame msg) {
        String userInfo = msg.getHeader("login");
        String userPasscode = msg.getHeader("passcode");
        // User already logged in
        synchronized(userHandler){
        if (userInfo != null && userHandler.IsUserLogedIn(userInfo)) {
            return errorFrame("User already logged in", "The user " + userInfo + " is already connected from a different client" ,msg);
        }
            // User not logged in and user exists
            if (userInfo != null && userPasscode != null && userHandler.userExists(userInfo)
                    && !userHandler.IsUserLogedIn(userInfo)) {
                boolean succseedLogin = userHandler.logInUser(userInfo, userPasscode, connectionId);
                // User not logged in and user exists and password is wrong
                if (!succseedLogin) {
                    return errorFrame("Wrong password", userInfo+ "'s password is incorrect, please try to login again with the password of registration" , msg);
                }
            }
            userHandler.addNewUser(userInfo, userPasscode, connectionId);
        }
        Frame frame = new Frame("CONNECTED");
        String version = msg.getHeader("accept-version");
        frame.addHeader("version", version);
        connections.send(connectionId, frame);
        return null;
    }

    public Frame errorFrame(String message, String detailedDescroption ,Frame msg) {
        Frame frame = new Frame("ERROR");
        String receipt = msg.getHeader("receipt");
        if (receipt == null) {
            frame.addHeader("receipt-id", "-100");
        } else {
            frame.addHeader("receipt-id",receipt);
        }
        frame.addHeader("message", message);
        String frameMsg = msg.stringMessage().replaceAll("\u0000", "");
        //frame.addHeader("The message", "\n" + "-----" + "\n" + frameMsg + "-----"+ "\n");
        frame.setMessageBody("The message: \n" + "-----" + "\n" + frameMsg + "-----"+ "\n" + detailedDescroption);
        synchronized(userHandler){
            UserHandler.getInstance().removeActiveUser(connectionId);
        }
        shouldTerminate = true;
        return frame;
    }
}
