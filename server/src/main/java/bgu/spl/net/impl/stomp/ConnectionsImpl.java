package bgu.spl.net.impl.stomp;

import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

/**
 * The ConnectionsImpl class implements the Connections interface and manages
 * connections, channels, and message sending for a server.
 *
 * @param <T> the type of messages handled by the connections
 */
public class ConnectionsImpl<T> implements Connections<T> {
    private ConcurrentHashMap<Integer, ConnectionHandler<T>> connections;
    private ConcurrentHashMap<String, ConcurrentHashMap<Integer, String>> channels;
    private int messageId = 0;

    public ConnectionsImpl() {
        connections = new ConcurrentHashMap<>();
        channels = new ConcurrentHashMap<>();
    }

    /**
     * Sends a message to a specific client identified by the connection ID.
     *
     * @param connectionId the ID of the connection to send the message to
     * @param msg          the message to send
     * @return true if the message was sent successfully, false otherwise
     */
    @Override
    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> connection = connections.get(connectionId);
        if (connection != null) {
            connection.send(msg);
            return true;
        }
        return false;
    }

    /**
     * Sends a message to all clients subscribed to a specific channel.
     *
     * @param channel the channel to send the message to
     * @param msg     the message to send
     */
    @SuppressWarnings("unchecked")
    @Override
    public void send(String channel, T msg) {
        ConcurrentHashMap<Integer, String> channelConnections = channels.get(channel);
        if (channelConnections != null && !channelConnections.isEmpty()) { // If there are clients subscribed to the
                                                                           // channel or the channel exists
            for (Integer connectionId : channelConnections.keySet()) {
                ConnectionHandler<T> ch = connections.get(connectionId);
                if (ch != null) {
                    Frame frame = new Frame((Frame) msg, channelConnections.get(connectionId), messageId++, channel);
                    ch.send((T) frame);
                }
            }
        }
    }

    /**
     * Disconnects a client identified by the connection ID.
     *
     * @param connectionId the ID of the connection to disconnect
     */
    @Override
    public void disconnect(int connectionId) {
        connections.remove(connectionId);
        for (ConcurrentHashMap<Integer, String> channel : channels.values()) {
            channel.remove(connectionId);
        }
    }

    /**
     * Connects a client with the given connection ID and handler.
     *
     * @param connectionId the ID of the connection to add
     * @param handler      the connection handler for the client
     */
    public void connect(int connectionId, ConnectionHandler<T> handler) {
        connections.put(connectionId, handler);
    }

    /**
     * Gets the connections map.
     *
     * @return the connections map
     */
    public ConcurrentHashMap<Integer, ConnectionHandler<T>> getConnections() {
        return connections;
    }

    /**
     * Sets the connections map.
     *
     * @param connections the connections map to set
     */
    public void setConnections(ConcurrentHashMap<Integer, ConnectionHandler<T>> connections) {
        this.connections = connections;
    }

    /**
     * Gets the channels map.
     *
     * @return the channels map
     */
    public ConcurrentHashMap<String, ConcurrentHashMap<Integer, String>> getChannels() {
        return channels;
    }

    /**
     * Adds a client to a specific channel with a subscription ID.
     *
     * @param channel        the channel to add the client to
     * @param connectionId   the ID of the connection to add
     * @param subscriptionId the subscription ID for the client
     */
    @Override
    public void addToChannels(String channel, int connectionId, String subscriptionId) {
        // Add the channel if it doesn't exist
        channels.putIfAbsent(channel, new ConcurrentHashMap<>());

        // Add the connection ID and subscription ID to the channel
        channels.get(channel).put(connectionId, subscriptionId);
    }

    /**
     * Gets the current message ID.
     *
     * @return the current message ID
     */
    public int getMessageId() {
        return messageId;
    }

    /**
     * Sets the message ID.
     *
     * @param messageId the message ID to set
     */
    public void setMessageId(int messageId) {
        this.messageId = messageId;
    }

    /**
     * Checks if a client identified by the connection ID is subscribed to a
     * specific channel.
     *
     * @param connectionId the ID of the connection to check
     * @param channel      the channel to check the subscription for
     * @return true if the client is subscribed to the channel, false otherwise
     */
    @Override
    public boolean isSubscribed(int connectionId, String channel) {
        if (channels.get(channel) == null) {
            return false;
        }
        if (channels.get(channel).get(connectionId) != null) {
            return true;
        }
        return false;
    }

    /**
     * Removes a subscription for a client identified by the connection ID and
     * subscription ID.
     *
     * @param subscriptionId the subscription ID to remove
     * @param connectionId   the ID of the connection to remove the subscription
     *                       from
     * @return true if the subscription was removed successfully, false otherwise
     */
    @Override
    public boolean removeSubscription(String subscriptionId, int connectionId) {
        boolean removed = false;
        for (ConcurrentHashMap<Integer, String> channel : channels.values()) {
            if (channel.get(connectionId) != null && channel.get(connectionId).equals(subscriptionId)) {
                channel.remove(connectionId);
                removed = true;
            }
        }
        return removed;
    }

    /**
     * Increment and retrieve the next unique message ID.
     *
     * @return the next unique message ID
     */
    public synchronized int getNextMessageId() {
        return messageId++;
    }
}
