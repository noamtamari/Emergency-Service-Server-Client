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
            connections.send(connectionId, process);
        } else {
            Frame frame = new Frame("RECEIPT");
            String receipt = message.getHeader("receipt");
            if (receipt == null) {
                frame.addHeader("receipt-id", "345");
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
            // System.out.println("Server got DISCONNECT and need to return receipt with id:
            // " + msg.getHeader("receipt"));
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
                connections.addToChannels(destination, connectionId, subscriptionId);
            } else {
                return errorFrame("Destination illegal", msg);
            }
        }
        // subscriptionId is illegal
        else {
            return errorFrame("Client's id is illegal", msg);// add msg
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
            return errorFrame("Cannot unsubscribe to unsubscribe user", msg);// add msg
        }
        return null;
    }

    public Frame handleDisconnect(Frame msg) {
        Frame frame = new Frame("RECEIPT");
        frame.addHeader("receipt-id", msg.getHeader("receipt"));
        connections.send(connectionId, frame);
        connections.disconnect(connectionId);
        UserHandler.getInstance().removeActiveUser(connectionId);
        shouldTerminate = true;
        return null;
    }

    public Frame handleSend(Frame msg) {
        String destination = msg.getHeader("destination");
        if (destination != null) {
            if (connections.isSubscribed(connectionId, destination)) {
                Frame frame = new Frame(("MESSAGE"), msg.getMessageBody());
                connections.send(destination, frame);
            } else {
                return errorFrame("User not subscribed to this channel", msg);
            }
        } else {
            return errorFrame("Destination illegal", msg);
        }
        return null;

    }

    public Frame handleConnent(Frame msg) {
        String userInfo = msg.getHeader("login");
        String userPasscode = msg.getHeader("passcode");
        // User already logged in
        if (userInfo != null && userHandler.IsUserLogedIn(userInfo)) {
            return errorFrame("User already logged in", msg);
        }
        // User not logged in and user exists
        if (userInfo != null && userPasscode != null && userHandler.userExists(userInfo)
                && !userHandler.IsUserLogedIn(userInfo)) {
            boolean succseedLogin = userHandler.logInUser(userInfo, userPasscode, connectionId);
            // User not logged in and user exists and password is wrong
            if (!succseedLogin) {
                return errorFrame("Wrong password", msg);
            }
        }
        Frame frame = new Frame("CONNECTED");
        String version = msg.getHeader("accept-version");
        if (version != null) {
            UserHandler.getInstance().addNewUser(userInfo, userPasscode, connectionId);
            frame.addHeader("version", version);
            connections.send(connectionId, frame);
        } else {
            return errorFrame("Version not supported", msg);
        }
        return null;
    }

    public Frame errorFrame(String message, Frame msg) {
        Frame frame = new Frame("ERROR");
        String receipt = msg.getHeader("receipt");
        if (receipt == null) {
            frame.addHeader("receipt-id", "-1");
        } else {
            frame.addHeader("receipt-id", msg.getHeader("receipt"));
        }
        frame.addHeader("message", "malformed frame received");
        String frameMsg = msg.stringMessage().replaceAll("\u0000", "");
        frame.addHeader("The message", "\n" + "-----" + "\n" + frameMsg + "-----"+ "\n");
        frame.setMessageBody(message);
        UserHandler.getInstance().removeActiveUser(connectionId);

        //System.out.println(frame.stringMessage().contains("\u0000"));
        shouldTerminate = true;
        return frame;
    }
}
