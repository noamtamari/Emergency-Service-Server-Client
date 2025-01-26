package bgu.spl.net.srv;

/**
 * The Connections interface represents a connection manager that handles sending messages,
 * managing subscriptions, and disconnecting clients.
 *
 * @param <T> the type of messages handled by the connections
 */
public interface Connections<T> {

    /**
     * Sends a message to a specific client identified by the connection ID.
     *
     * @param connectionId the ID of the connection to send the message to
     * @param msg the message to send
     * @return true if the message was sent successfully, false otherwise
     */
    boolean send(int connectionId, T msg);

    /**
     * Sends a message to all clients subscribed to a specific channel.
     *
     * @param channel the channel to send the message to
     * @param msg the message to send
     */
    void send(String channel, T msg);

    /**
     * Disconnects a client identified by the connection ID.
     *
     * @param connectionId the ID of the connection to disconnect
     */
    void disconnect(int connectionId);

    /**
     * Adds a client to a specific channel with a subscription ID.
     *
     * @param destination the channel to add the client to
     * @param connectionId the ID of the connection to add
     * @param subscriptionId the subscription ID for the client
     */
    void addToChannels(String destination, int connectionId, String subscriptionId);

    /**
     * Removes a subscription for a client identified by the connection ID and subscription ID.
     *
     * @param subscriptionId the subscription ID to remove
     * @param connectionId the ID of the connection to remove the subscription from
     * @return true if the subscription was removed successfully, false otherwise
     */
    boolean removeSubscription(String subscriptionId, int connectionId);

    /**
     * Checks if a client identified by the connection ID is subscribed to a specific channel.
     *
     * @param connectionId the ID of the connection to check
     * @param destination the channel to check the subscription for
     * @return true if the client is subscribed to the channel, false otherwise
     */
    boolean isSubscribed(int connectionId, String destination);
}